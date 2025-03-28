#include "hello_handler.hpp"

#include <userver/server/handlers/http_handler_base.hpp>

namespace app {

namespace {

class HelloHandler final : public userver::server::handlers::HttpHandlerBase {
public:
  static constexpr std::string_view kName = "handler-hello";

  HelloHandler(const userver::components::ComponentConfig& config, const userver::components::ComponentContext& context)
      : HttpHandlerBase(config, context) {}

  std::string HandleRequestThrow(const userver::server::http::HttpRequest& request,
                                 userver::server::request::RequestContext&) const override {
    const auto& name = request.GetArg("name");
    const std::string msg = GetWelcomeMessage(name);

    request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
    return fmt::format(R"({{"message": "{}"}})", msg);
  }
};

}  // namespace

void AppendHello(userver::components::ComponentList& component_list) {
  component_list.Append<HelloHandler>();
}

}  // namespace app