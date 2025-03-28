#include "guess_handler.hpp"

#include <userver/components/component_context.hpp>
#include <userver/formats/json.hpp>
#include <userver/logging/log.hpp>
#include <userver/server/http/http_status.hpp>

namespace contesto {

GuessHandler::GuessHandler(const userver::components::ComponentConfig& config,
                           const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      similarity_service_(std::make_shared<WordSimilarityService>(config, context)),
      session_manager_(context.FindComponent<SessionManager>()) {
  LOG_INFO() << "GuessHandler initialized";
}

std::string GuessHandler::HandleRequestThrow(const userver::server::http::HttpRequest& request,
                                             userver::server::request::RequestContext&) const {
  if (request.GetMethod() == userver::server::http::HttpMethod::kOptions) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
    return "";
  }

  try {
    const auto& body = request.RequestBody();

    if (body.empty()) {
      request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
      LOG_ERROR() << "Empty request body";
      return userver::formats::json::ToString(userver::formats::json::MakeObject("error", "Empty request body"));
    }

    userver::formats::json::Value json;
    try {
      json = userver::formats::json::FromString(body);
    } catch (const userver::formats::json::Exception& e) {
      request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
      LOG_ERROR() << "Invalid JSON: " << e.what();
      return userver::formats::json::ToString(userver::formats::json::MakeObject("error", "Invalid JSON format"));
    }

    const auto guessed_word = json["word"].As<std::string>();

    if (guessed_word.empty()) {
      request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
      LOG_ERROR() << "No word provided";
      return userver::formats::json::ToString(userver::formats::json::MakeObject("error", "Word cannot be empty"));
    }

    // Get session ID from cookie
    const auto& session_id = request.GetCookie("session_id");
    if (session_id.empty()) {
      request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
      LOG_ERROR() << "No session_id found (neither in cookie nor in request body)";
      return userver::formats::json::ToString(userver::formats::json::MakeObject("error", "No active game session"));
    }

    // Get target word for this session
    if (!session_manager_.HasSession(session_id)) {
      request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
      LOG_ERROR() << "Session " << session_id << " not found in session manager";
      return userver::formats::json::ToString(userver::formats::json::MakeObject("error", "Invalid game session"));
    }

    const std::string target_word = session_manager_.GetTargetWord(session_id);

    // Validate the word
    if (!similarity_service_->ValidateWord(guessed_word)) {
      request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
      LOG_ERROR() << "Invalid word: " << guessed_word;
      return userver::formats::json::ToString(userver::formats::json::MakeObject("error", "Invalid word"));
    }

    // Get similarity ranking
    const auto similar_words = similarity_service_->GetSimilarWords(guessed_word, target_word);

    if (similar_words.empty()) {
      request.SetResponseStatus(userver::server::http::HttpStatus::kInternalServerError);
      LOG_ERROR() << "Failed to get similar words for target word: " << target_word;
      return userver::formats::json::ToString(userver::formats::json::MakeObject("error", "Failed to process word"));
    }

    const auto& result = similar_words[0];
    const bool is_correct = (guessed_word == target_word);

    const auto response =
        userver::formats::json::MakeObject("word", guessed_word, "rank", result.rank, "correct", is_correct);

    LOG_INFO() << "Guess: " << guessed_word << ", Rank: " << result.rank
               << ", Correct: " << (is_correct ? "yes" : "no");

    return userver::formats::json::ToString(response);

  } catch (const std::exception& e) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kInternalServerError);
    LOG_ERROR() << "Error processing guess: " << e.what();
    return userver::formats::json::ToString(userver::formats::json::MakeObject("error", e.what()));
  }
}

}  // namespace contesto