#include "hello_handler.hpp"

namespace app {

std::string HelloHandler::HandleRequest(server::http::HttpRequest& request, server::request::RequestContext&) const {
  const std::string& name = request.GetArg("name");
  formats::json::ValueBuilder response;
  
  response["message"] = SayHelloTo(name);
  
  request.SetResponseStatus(server::http::HttpStatus::kOk);
  return formats::json::ToString(response.ExtractValue());
}

}