#pragma once

#include "session_manager.hpp"

#include <userver/server/handlers/http_handler_base.hpp>

namespace contexto {

class GiveUpHandler final : public userver::server::handlers::HttpHandlerBase {
public:
  static constexpr std::string_view kName = "contexto-give-up-handler";

  GiveUpHandler(const userver::components::ComponentConfig&, const userver::components::ComponentContext&);

  std::string HandleRequestThrow(const userver::server::http::HttpRequest&,
                                 userver::server::request::RequestContext&) const override;

private:
  SessionManager& session_manager_;
};

}  // namespace contexto
