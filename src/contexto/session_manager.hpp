#pragma once

#include <pch.hpp>

#include <userver/components/loggable_component_base.hpp>
#include <userver/engine/shared_mutex.hpp>

namespace contexto {

struct GuessInfo {
  std::string_view word;
  int rank = -1;
};

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
    std::shared_lock lock(mutex_);
    return game_sessions_.find(session_id) != game_sessions_.end();
  }

  void AddGuess(const std::string& session_id, std::string_view word, int rank) {
    std::lock_guard lock(mutex_);
    auto& guesses = session_guesses_[session_id];
    GuessInfo info{.word = word, .rank = rank};
    guesses.push_back(info);
  }

  void SetTargetWord(const std::string& session_id, std::string_view target_word);

  std::string_view GetTargetWord(const std::string& session_id) const {
    std::shared_lock lock(mutex_);
    const auto it = game_sessions_.find(session_id);
    if (it == game_sessions_.end()) return {};
    return it->second;
  }

  std::vector<std::string_view> GetGuessedWords(const std::string& session_id) const;

  std::optional<GuessInfo> GetClosestGuess(const std::string& session_id) const;

  static userver::yaml_config::Schema GetStaticConfigSchema();

private:
  mutable userver::engine::SharedMutex mutex_;
  std::unordered_map<std::string, std::string> game_sessions_;
  size_t max_sessions_ = 0;

  std::unordered_map<std::string, std::vector<GuessInfo>> session_guesses_;
};

}  // namespace contexto
