#pragma once

#include "session_manager.hpp"
#include "word_similarity_service.hpp"

#include <userver/server/handlers/http_handler_base.hpp>

namespace contesto {

class NewGameHandler final : public userver::server::handlers::HttpHandlerBase {
public:
  static constexpr std::string_view kName = "contesto-new-game-handler";

  NewGameHandler(const userver::components::ComponentConfig&, const userver::components::ComponentContext&);

  std::string HandleRequestThrow(const userver::server::http::HttpRequest&,
                                 userver::server::request::RequestContext&) const override;

private:
  std::shared_ptr<WordSimilarityService> similarity_service_ = nullptr;
  SessionManager& session_manager_;
};

}  // namespace contesto