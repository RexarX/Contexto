#pragma once

#include "models/word.hpp"
#include "word-embedding/word_dictionary.hpp"

#include <userver/components/loggable_component_base.hpp>
#include <userver/engine/shared_mutex.hpp>
#include <userver/yaml_config/schema.hpp>

namespace contexto {

class WordDictionaryComponent final : public userver::components::LoggableComponentBase {
public:
  static constexpr std::string_view kName = "word-dictionary";
  static constexpr int kMaxRank = 1000;

  WordDictionaryComponent(const userver::components::ComponentConfig& config,
                          const userver::components::ComponentContext& context);

  bool ValidateWord(std::string_view word) const {
    if (word.empty()) return {};
    return dictionary_.ContainsWord(word);
  }

  const models::DictionaryWord* GenerateNewTargetWord() const {
    return dictionary_.GetRandomWordByType(dictionary_preferred_word_type_);
  }

  std::optional<int> CalculateRank(std::string_view guessed_word, std::string_view target_word) const;

  std::vector<models::Word> GetSimilarWords(std::string_view word, std::string_view target_word) const;

  const WordDictionary& GetDictionary() const noexcept { return dictionary_; }

  static userver::yaml_config::Schema GetStaticConfigSchema();

private:
  static constexpr models::WordType StringToWordType(std::string_view str) noexcept {
    if (str == "noun") {
      return models::WordType::kNoun;
    } else if (str == "verb") {
      return models::WordType::kVerb;
    } else if (str == "adjective") {
      return models::WordType::kAdjective;
    } else if (str == "adverb") {
      return models::WordType::kAdverb;
    } else if (str == "pronoun") {
      return models::WordType::kPronoun;
    } else if (str == "any") {
      return models::WordType::kAny;
    }

    return models::WordType::kUnknown;
  }

  WordDictionary dictionary_;
  size_t max_dictionary_words_ = 0;
  models::WordType embeddings_preferred_word_type_ = models::WordType::kUnknown;
  models::WordType dictionary_preferred_word_type_ = models::WordType::kUnknown;

  mutable std::mt19937 rng_{std::random_device{}()};
};

}  // namespace contexto
