#include <userver/utest/utest.hpp>

#include "hello/hello_handler.hpp"

UTEST(SayHelloTo, Basic) {
  EXPECT_EQ(app::SayHelloTo("Developer"), "Welcome to App, Developer!");
  EXPECT_EQ(app::SayHelloTo({}), "Welcome to App!");
}