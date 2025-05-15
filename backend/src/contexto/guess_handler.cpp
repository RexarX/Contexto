#include "guess_handler.hpp"

#include <userver/components/component_context.hpp>
#include <userver/formats/json.hpp>
#include <userver/logging/log.hpp>
#include <userver/server/http/http_status.hpp>

namespace contexto {

GuessHandler::GuessHandler(const userver::components::ComponentConfig& config,
                           const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      session_manager_(context.FindComponent<SessionManager>()),
      dictionary_(context.FindComponent<WordDictionaryComponent>()) {
  LOG_INFO() << "GuessHandler initialized";
}

std::string GuessHandler::HandleRequestThrow(const userver::server::http::HttpRequest& request,
                                             userver::server::request::RequestContext&) const {
  auto& http_response = request.GetHttpResponse();
  http_response.SetHeader(std::string_view("Access-Control-Allow-Origin"), "*");
  http_response.SetHeader(std::string_view("Access-Control-Allow-Methods"), "GET, POST, OPTIONS");
  http_response.SetHeader(std::string_view("Access-Control-Allow-Headers"), "Content-Type");
  http_response.SetHeader(std::string_view("Access-Control-Allow-Credentials"), "true");

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

    std::string guessed_word = json["word"].As<std::string>();
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
      LOG_ERROR() << "Session " << "'" << session_id << "'" << " not found in session manager";
      return userver::formats::json::ToString(userver::formats::json::MakeObject("error", "Invalid game session"));
    }

    const std::string_view target_word_with_pos = session_manager_.GetTargetWord(session_id);
    if (!dictionary_.ValidateWord(guessed_word)) {
      request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
      LOG_ERROR() << "Unknown word submitted: '" << guessed_word << "'";
      return userver::formats::json::ToString(userver::formats::json::MakeObject("error", "Invalid word"));
    }

    // Get similarity ranking
    // const auto similar_words = dictionary_.GetSimilarWords(guessed_word, target_word);
    // if (similar_words.empty()) {
    //   request.SetResponseStatus(userver::server::http::HttpStatus::kInternalServerError);
    //   LOG_ERROR() << "Failed to get similar words for target word: " << target_word;
    //   return userver::formats::json::ToString(userver::formats::json::MakeObject("error", "Failed to process word"));
    // }

    // const auto& result = similar_words.front();
    const auto temp = dictionary_.GetDictionary().GetMostSimilarWords(target_word_with_pos, 100);
    std::ostringstream ss;
    ss << "Most similar words to target '" << target_word_with_pos << "': ";
    int count = 0;
    for (const auto& value : temp) {
      if (++count > 10) break;
      ss << "'" << value.first->word_with_pos << "'" << " (sim: " << value.second << "); ";
    }

    LOG_INFO() << ss.str();

    const auto rank_result = dictionary_.CalculateRank(guessed_word, target_word_with_pos);
    if (!rank_result) {
      request.SetResponseStatus(userver::server::http::HttpStatus::kInternalServerError);
      constexpr std::string_view error = "Failed to calculate rank";
      LOG_ERROR() << "Error processing guess: " << error;
      return userver::formats::json::ToString(userver::formats::json::MakeObject("error", error));
    }

    const int rank = *rank_result;

    const auto response =
        userver::formats::json::MakeObject("word", guessed_word, "rank", rank, "correct", (rank == 1 ? "yes" : "no"));

    LOG_INFO() << "Guess: " << guessed_word << ", Rank: " << rank << ", Correct: " << (rank == 1 ? "yes" : "no");

    session_manager_.AddGuess(session_id, std::move(guessed_word), rank);

    return userver::formats::json::ToString(response);

  } catch (const std::exception& e) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kInternalServerError);
    LOG_ERROR() << "Error processing guess: " << e.what();
    return userver::formats::json::ToString(userver::formats::json::MakeObject("error", e.what()));
  }
}

}  // namespace contexto
