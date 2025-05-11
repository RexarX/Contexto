#pragma once

#include "models/word.hpp"
#include "word-embedding/word_dictionary.hpp"

#include <userver/components/component_config.hpp>
#include <userver/engine/mutex.hpp>

namespace contexto {

class WordSimilarityService {
public:
  WordSimilarityService() {
    InitializeDictionary();
    BuildAdditionalWordsList();
  }

  ~WordSimilarityService() = default;

  std::string GenerateNewTargetWord();
  std::vector<models::Word> GetSimilarWords(std::string_view word, std::string_view target_word);

  bool ValidateWord(std::string_view word) const { return dictionary_.ContainsWord(word); }

private:
  double CalculateSimilarity(std::string_view lhs, std::string_view rhs) const {
    return dictionary_.CalculateSimilarity(lhs, rhs);
  }

  void InitializeDictionary();
  void BuildAdditionalWordsList();

  WordDictionary dictionary_;
  userver::engine::Mutex mutex_;

  // Special list of words that should be included in the game
  std::vector<std::string> additional_words_;

  size_t max_dictionary_words_ = 100000;
  models::WordType preferred_word_type_ = models::WordType::kNoun;
};

}  // namespace contexto
