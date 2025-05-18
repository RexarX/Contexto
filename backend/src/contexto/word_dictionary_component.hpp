#pragma once

#include "models/word.hpp"
#include "word-embedding/word_dictionary.hpp"

#include <userver/components/loggable_component_base.hpp>
#include <userver/engine/shared_mutex.hpp>
#include <userver/logging/log.hpp>
#include <userver/yaml_config/schema.hpp>

namespace contexto {

class WordDictionaryComponent final : public userver::components::LoggableComponentBase {
public:
  static constexpr std::string_view kName = "word-dictionary";
  static constexpr int kMaxRank = 1000;

  WordDictionaryComponent(const userver::components::ComponentConfig& config,
                          const userver::components::ComponentContext& context);

  bool ValidateWord(std::string_view word) const {
    if (word.empty()) return false;
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
    if (str == "noun") return models::WordType::kNoun;
    if (str == "verb") return models::WordType::kVerb;
    if (str == "adjective") return models::WordType::kAdjective;
    if (str == "adverb") return models::WordType::kAdverb;
    if (str == "adposition") return models::WordType::kAdposition;
    if (str == "auxiliary") return models::WordType::kAuxiliary;
    if (str == "coordinating_conjunction") return models::WordType::kCoordinatingConjunction;
    if (str == "determiner") return models::WordType::kDeterminer;
    if (str == "interjection") return models::WordType::kInterjection;
    if (str == "numeral") return models::WordType::kNumeral;
    if (str == "particle") return models::WordType::kParticle;
    if (str == "pronoun") return models::WordType::kPronoun;
    if (str == "proper_noun") return models::WordType::kProperNoun;
    if (str == "punctuation") return models::WordType::kPunctuation;
    if (str == "subordinating_conjunction") return models::WordType::kSubordinatingConjunction;
    if (str == "symbol") return models::WordType::kSymbol;
    if (str == "any") return models::WordType::kAny;
    return models::WordType::kUnknown;
  }

  WordDictionary dictionary_;
  size_t max_dictionary_words_ = 0;
  models::WordType embeddings_preferred_word_type_ = models::WordType::kUnknown;
  models::WordType dictionary_preferred_word_type_ = models::WordType::kUnknown;

  mutable std::mt19937 rng_{std::random_device{}()};
};

}  // namespace contexto
