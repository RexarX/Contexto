#pragma once

#include "models/word.hpp"

#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/engine/mutex.hpp>

namespace contexto {

class WordSimilarityService {
public:
  explicit WordSimilarityService(const userver::components::ComponentConfig&,
                                 const userver::components::ComponentContext&);

  std::string GenerateNewTargetWord();
  std::vector<models::Word> GetSimilarWords(std::string_view word, std::string_view target_word);

  bool ValidateWord(std::string_view word) const {
    return !word.empty() && std::find(dictionary_.begin(), dictionary_.end(), word) != dictionary_.end();
  }

  std::string GetHintWord(std::string_view target_word, std::span<const std::string> guessed_words) const;

private:
  double CalculateSimilarity(std::string_view lhs, std::string_view rhs) const;
  void LoadWordEmbeddings();
  void LoadDictionary();

  userver::engine::Mutex mutex_;

  std::unordered_map<std::string_view, std::vector<float>> word_embeddings_;
  std::vector<std::string> dictionary_;
};

}  // namespace contexto
