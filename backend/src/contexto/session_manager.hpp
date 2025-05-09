#pragma once

#include "pch.hpp"

#include <userver/components/loggable_component_base.hpp>
#include <userver/engine/mutex.hpp>

namespace contexto {

class SessionManager final : public userver::components::LoggableComponentBase {
public:
  static constexpr std::string_view kName = "session-manager";

  SessionManager(const userver::components::ComponentConfig&, const userver::components::ComponentContext&);

  void RemoveSession(const std::string& session_id) {
    std::lock_guard lock(mutex_);
    game_sessions_.erase(session_id);
  }

  void CleanupSessions();

  bool HasSession(const std::string& session_id) const {
    std::lock_guard lock(mutex_);
    return game_sessions_.find(session_id) != game_sessions_.end();
  }

  void SetTargetWord(const std::string& session_id, const std::string& target_word);
  std::string GetTargetWord(const std::string& session_id) const {
    std::lock_guard lock(mutex_);
    const auto it = game_sessions_.find(session_id);
    if (it == game_sessions_.end()) return {};
    return it->second;
  }

  static userver::yaml_config::Schema GetStaticConfigSchema();

private:
  mutable userver::engine::Mutex mutex_;
  std::unordered_map<std::string, std::string> game_sessions_;
  size_t max_sessions_ = 0;
};

}  // namespace contexto
