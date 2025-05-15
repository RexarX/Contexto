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
  std::lock_guard lock(mutex_);
  if (!game_sessions_.empty()) {
    const auto it = game_sessions_.begin();
    LOG_INFO() << "Cleaning up session: " << it->first;
    game_sessions_.erase(it);
  }
}

void SessionManager::SetTargetWord(const std::string& session_id, std::string_view word_with_pos) {
  std::unique_lock lock(mutex_);
  if (game_sessions_.size() >= max_sessions_) {
    LOG_WARNING() << "Session limit reached, cleaning up old sessions";
    mutex_.unlock();
    CleanupSessions();
    mutex_.lock();
  }

  game_sessions_[session_id] = word_with_pos;
}

std::vector<std::string_view> SessionManager::GetGuessedWords(const std::string& session_id) const {
  std::shared_lock lock(mutex_);
  std::vector<std::string_view> words;

  const auto it = session_guesses_.find(session_id);
  if (it == session_guesses_.end()) return words;
  words.reserve(it->second.size());
  for (const auto& guess : it->second) {
    words.push_back(guess.word);
  }
  return words;
}

std::optional<GuessInfo> SessionManager::GetClosestGuess(const std::string& session_id) const {
  std::shared_lock lock(mutex_);

  const auto it = session_guesses_.find(session_id);
  const auto& guesses = it->second;
  if (it == session_guesses_.end() || guesses.empty()) {
    return std::nullopt;
  }

  // Find the guess with the lowest rank (closest to target)
  const auto closest = std::min_element(guesses.begin(), guesses.end(),
                                        [](const GuessInfo& lhs, const GuessInfo& rhs) { return lhs.rank < rhs.rank; });

  return *closest;
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
