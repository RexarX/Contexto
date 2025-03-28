#include "contesto/guess_handler.hpp"
#include "contesto/new_game_handler.hpp"
#include "contesto/session_manager.hpp"

#include <userver/clients/dns/component.hpp>
#include <userver/clients/http/component.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/server/handlers/ping.hpp>
#include <userver/server/handlers/tests_control.hpp>
#include <userver/testsuite/testsuite_support.hpp>
#include <userver/utils/daemon_run.hpp>

int main(int argc, char* argv[]) {
  const auto component_list = userver::components::MinimalServerComponentList()
                                  .Append<userver::server::handlers::TestsControl>("tests-control")
                                  .Append<userver::components::TestsuiteSupport>("testsuite-support")
                                  .Append<userver::components::HttpClient>()
                                  .Append<userver::server::handlers::Ping>("ping")
                                  .Append<userver::clients::dns::Component>("dns-client")
                                  .Append<contesto::SessionManager>()
                                  .Append<contesto::NewGameHandler>()
                                  .Append<contesto::GuessHandler>();

  return userver::utils::DaemonMain(argc, argv, component_list);
}