#pragma once

#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/utest/using_namespace_userver.hpp>
#include <fmt/format.h>

#include "pch.hpp"

namespace app {

static inline std::string SayHelloTo(std::string_view name) {
  if (name.empty()) { return "Welcome to App!"; }
  return fmt::format("Welcome to App, {}!", name);
}

class HelloHandler final : public server::handlers::HttpHandlerBase {
public:
  static constexpr std::string_view kName = "handler-hello";
  using HttpHandlerBase::HttpHandlerBase;
  
  std::string HandleRequest(server::http::HttpRequest& request, server::request::RequestContext&) const override;
};



}