#pragma once

#include "pch.hpp"

#include <userver/components/component_list.hpp>

#include <fmt/format.h>

namespace app {

static std::string GetWelcomeMessage(std::string_view name) {
  if (!name.empty()) {
    return fmt::format("Привет, {}! Добро пожаловать в аудиогид.", name);
  } else {
    return "Добро пожаловать в аудиогид! Выберите интересующий вас тур или скажите его название.";
  }
}

void AppendHello(userver::components::ComponentList& component_list);

}  // namespace app
