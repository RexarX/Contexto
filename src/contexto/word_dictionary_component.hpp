#pragma once

#include "models/word.hpp"
#include "word-embedding/word_dictionary.hpp"

#include <userver/components/loggable_component_base.hpp>
#include <userver/yaml_config/schema.hpp>

namespace contexto {

class WordDictionaryComponent final : public userver::components::LoggableComponentBase {
public:
  static constexpr std::string_view kName = "word-dictionary";
  static constexpr int kMaxRank = 1000;

  WordDictionaryComponent(const userver::components::ComponentConfig& config,
                          const userver::components::ComponentContext& context);

  bool ValidateWord(std::string_view word) const { return dictionary_.ContainsWord(word); }

  std::string_view GenerateNewTargetWord() const;

  int CalculateRank(std::string_view lhs, std::string_view rhs) const {
    if (lhs == rhs) return 1;

    const float similarity = dictionary_.CalculateSimilarity(lhs, rhs);
    int rank = 0;
    if (similarity < 0.01f) {
      rank = kMaxRank;
    } else {
      // Map similarity from [0.0f; 1.0f] to rank [kMaxRank; 2]
      // Higher similarity = lower rank (closer to target)
      rank = static_cast<int>(std::max(2.0f, std::round(static_cast<float>(kMaxRank) - (similarity * (kMaxRank - 2)))));
    }

    return rank;
  }

  std::vector<models::Word> GetSimilarWords(std::string_view word, std::string_view target_word) const;

  const WordDictionary& GetDictionary() const noexcept { return dictionary_; }

  static userver::yaml_config::Schema GetStaticConfigSchema();

private:
  WordDictionary dictionary_;
  size_t max_dictionary_words_ = 0;
  models::WordType preferred_word_type_ = models::WordType::kUnknown;
};

}  // namespace contexto
