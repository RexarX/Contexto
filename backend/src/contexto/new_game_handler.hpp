#pragma once

#include <userver/server/handlers/http_handler_base.hpp>

namespace contexto {

class SessionManager;
class WordDictionaryComponent;

class NewGameHandler final : public userver::server::handlers::HttpHandlerBase {
public:
  static constexpr std::string_view kName = "contexto-new-game-handler";

  NewGameHandler(const userver::components::ComponentConfig&, const userver::components::ComponentContext&);

  std::string HandleRequestThrow(const userver::server::http::HttpRequest&,
                                 userver::server::request::RequestContext&) const override;

private:
  SessionManager& session_manager_;
  const WordDictionaryComponent& dictionary_;
};

}  // namespace contexto
