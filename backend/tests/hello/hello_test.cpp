#include <userver/utest/utest.hpp>

#include "hello/hello_handler.hpp"

UTEST(SayHelloTo, Basic) {
  EXPECT_EQ(app::GetWelcomeMessage("Developer"), "Welcome to App, Developer!");
  EXPECT_EQ(app::GetWelcomeMessage({}), "Welcome to App!");
}