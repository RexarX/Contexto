#include "new_game_handler.hpp"

#include <userver/components/component_context.hpp>
#include <userver/formats/json.hpp>
#include <userver/logging/log.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/utils/uuid4.hpp>

namespace contexto {

NewGameHandler::NewGameHandler(const userver::components::ComponentConfig& config,
                               const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      session_manager_(context.FindComponent<SessionManager>()),
      dictionary_(context.FindComponent<WordDictionaryComponent>()) {
  LOG_INFO() << "NewGameHandler initialized";
}

std::string NewGameHandler::HandleRequestThrow(const userver::server::http::HttpRequest& request,
                                               userver::server::request::RequestContext&) const {
  if (request.GetMethod() == userver::server::http::HttpMethod::kOptions) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
    return "";
  }

  try {
    std::string session_id;
    {
      const auto& cookie = request.GetCookie("session_id");
      if (cookie.empty()) {
        session_id = userver::utils::generators::GenerateUuid();
        // Set the cookie explicitly with path and SameSite attributes
        auto& response = request.GetHttpResponse();
        response.SetCookie(userver::server::http::Cookie("session_id", session_id));
        LOG_INFO() << "Created new session ID: " << session_id;
      } else {
        session_id = cookie;
        LOG_INFO() << "Using existing session ID: " << session_id;
      }
    }

    const std::string_view target_word = dictionary_.GenerateNewTargetWord();
    LOG_INFO() << "New game created with session " << session_id << " and target word: " << target_word;

    session_manager_.SetTargetWord(session_id, target_word);

    const auto response = userver::formats::json::MakeObject("success", true, "session_id", session_id);

    return userver::formats::json::ToString(response);

  } catch (const std::exception& e) {
    request.SetResponseStatus(userver::server::http::HttpStatus::kInternalServerError);
    LOG_ERROR() << "Error creating new game: " << e.what();
    return userver::formats::json::ToString(userver::formats::json::MakeObject("error", e.what()));
  }
}

}  // namespace contexto
