#include "give_up_handler.hpp"

#include <userver/components/component_context.hpp>
#include <userver/formats/json.hpp>
#include <userver/logging/log.hpp>
#include <userver/server/http/http_status.hpp>

namespace contexto {

GiveUpHandler::GiveUpHandler(const userver::components::ComponentConfig& config,
                             const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context), session_manager_(context.FindComponent<SessionManager>()) {
  LOG_INFO() << "GiveUpHandler initialized";
}

std::string GiveUpHandler::HandleRequestThrow(const userver::server::http::HttpRequest& request,
                                              userver::server::request::RequestContext&) const {
  auto& http_response = request.GetHttpResponse();

  // Set proper CORS headers
  const auto& origin = request.GetHeader("Origin");
  if (!origin.empty()) {
    http_response.SetHeader(std::string_view("Access-Control-Allow-Origin"), origin);
  } else {
    http_response.SetHeader(std::string_view("Access-Control-Allow-Origin"), "*");
  }

  http_response.SetHeader(std::string_view("Access-Control-Allow-Methods"), "GET, POST, OPTIONS");
  http_response.SetHeader(std::string_view("Access-Control-Allow-Headers"), "Content-Type, X-Requested-With");
  http_response.SetHeader(std::string_view("Access-Control-Allow-Credentials"), "true");

  if (request.GetMethod() == userver::server::http::HttpMethod::kOptions) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
    return "";
  }

  try {
    // Get session ID from cookie or request body
    std::string session_id;

    const auto& cookie = request.GetCookie("session_id");
    if (!cookie.empty()) {
      session_id = cookie;
    } else {
      const auto& body = request.RequestBody();
      if (!body.empty()) {
        try {
          auto json = userver::formats::json::FromString(body);
          if (json.HasMember("session_id")) {
            session_id = json["session_id"].As<std::string>();
          }
        } catch (const std::exception& e) {
          LOG_ERROR() << "Failed to parse request body: " << e.what();
        }
      }
    }

    if (session_id.empty()) {
      request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
      return userver::formats::json::ToString(
          userver::formats::json::MakeObject("error", "No active game session found"));
    }

    // Check if the session exists
    if (!session_manager_.HasSession(session_id)) {
      request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
      return userver::formats::json::ToString(userver::formats::json::MakeObject("error", "Invalid game session"));
    }

    if (session_manager_.IsGameOver(session_id)) {
      request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
      return userver::formats::json::ToString(
          userver::formats::json::MakeObject("error", "Game is already over. Start a new game to continue."));
    }

    // Get the target word for the session
    const std::string_view target_word_with_pos = session_manager_.GetTargetWord(session_id);
    if (target_word_with_pos.empty()) {
      request.SetResponseStatus(userver::server::http::HttpStatus::kInternalServerError);
      return userver::formats::json::ToString(
          userver::formats::json::MakeObject("error", "Failed to retrieve target word"));
    }

    // Extract just the word part without the POS tag
    const std::string_view word = models::GetWordFromWordWithPOS(target_word_with_pos);

    // Mark the game as over
    session_manager_.MarkGameOver(session_id);

    // Return the target word
    const auto response = userver::formats::json::MakeObject("success", true, "target_word", word);

    LOG_INFO() << "Player gave up. Session: " << session_id << ", Target word: " << target_word_with_pos;

    return userver::formats::json::ToString(response);

  } catch (const std::exception& e) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kInternalServerError);
    LOG_ERROR() << "Error processing give up request: " << e.what();
    return userver::formats::json::ToString(userver::formats::json::MakeObject("error", e.what()));
  }
}

}  // namespace contexto
