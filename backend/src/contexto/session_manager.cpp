#include "session_manager.hpp"

#include <userver/components/component_config.hpp>
#include <userver/logging/log.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

namespace contexto {

SessionManager::SessionManager(const userver::components::ComponentConfig& config,
                               const userver::components::ComponentContext& context)
    : LoggableComponentBase(config, context),
      max_sessions_(config.HasMember("max-sessions") ? config["max-sessions"].As<size_t>() : 10000) {
  LOG_INFO() << "SessionManager initialized with max_sessions=" << max_sessions_;
}

void SessionManager::CleanupSessions() {
  std::lock_guard<userver::engine::Mutex> lock(mutex_);
  if (!game_sessions_.empty()) {
    const auto it = game_sessions_.begin();
    LOG_INFO() << "Cleaning up session: " << it->first;
    game_sessions_.erase(it);
  }
}

void SessionManager::SetTargetWord(const std::string& session_id, const std::string& target_word) {
  std::lock_guard<userver::engine::Mutex> lock(mutex_);
  if (game_sessions_.size() >= max_sessions_) {
    LOG_WARNING() << "Session limit reached, cleaning up old sessions";
    mutex_.unlock();
    CleanupSessions();
    mutex_.lock();
  }

  game_sessions_[session_id] = target_word;
}

userver::yaml_config::Schema SessionManager::GetStaticConfigSchema() {
  return userver::yaml_config::MergeSchemas<userver::components::LoggableComponentBase>(R"(
type: object
description: Game session manager component
additionalProperties: false
properties:
  max-sessions:
    type: integer
    description: maximum number of active game sessions
    defaultDescription: 10000
)");
}

}  // namespace contexto
