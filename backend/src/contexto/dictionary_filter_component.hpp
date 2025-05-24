#pragma once

#include "models/dictionary_word.hpp"

#include <userver/components/loggable_component_base.hpp>
#include <userver/yaml_config/schema.hpp>

namespace contexto {

class DictionaryFilterComponent final : public userver::components::LoggableComponentBase {
public:
  static constexpr std::string_view kName = "dictionary-filter";

  DictionaryFilterComponent(const userver::components::ComponentConfig& config,
                            const userver::components::ComponentContext& context);

  bool ShouldFilterOutEmbedding(std::string_view word) const;
  bool ShouldFilterOutDictionary(std::string_view word) const;

  bool ShouldFilterOutEmbedding(const models::DictionaryWord& dict_word) const {
    return ShouldFilterOutEmbedding(dict_word.word_with_pos);
  }

  bool ShouldFilterOutDictionary(const models::DictionaryWord& dict_word) const {
    return ShouldFilterOutDictionary(dict_word.word_with_pos);
  }

  bool IsBlacklisted(const std::string& word) const {
    if (blacklisted_words_.empty()) return false;
    return blacklisted_words_.contains(word);
  }

  bool IsBlacklisted(const models::DictionaryWord& dict_word) const {
    return IsBlacklisted(std::string(dict_word.GetWord()));
  }

  bool HasPreferredEmbeddingType(models::WordType word_type) const {
    if (embedding_preferred_types_.empty()) return false;
    const auto it = std::ranges::find(embedding_preferred_types_, word_type);
    return it != embedding_preferred_types_.end();
  }

  bool HasPreferredDictionaryType(models::WordType word_type) const {
    if (dictionary_preferred_types_.empty()) return false;
    const auto it = std::ranges::find(dictionary_preferred_types_, word_type);
    return it != dictionary_preferred_types_.end();
  }

  bool HasPreferredEmbeddingTypes() const { return !HasPreferredEmbeddingType(models::WordType::kAny); }
  bool HasPreferredDictionaryTypes() const { return !HasPreferredDictionaryType(models::WordType::kAny); }

  size_t GetBlacklistSize() const noexcept { return blacklisted_words_.size(); }

  std::span<const models::WordType> GetEmbeddingPreferredTypes() const noexcept { return embedding_preferred_types_; }
  std::span<const models::WordType> GetDictionaryPreferredTypes() const noexcept { return dictionary_preferred_types_; }

  static userver::yaml_config::Schema GetStaticConfigSchema();

private:
  static models::WordType StringToWordType(std::string_view str) noexcept;
  bool LoadBlacklistedWords(std::string_view file_path);
  bool FilterByWordType(std::string_view word, const std::vector<models::WordType>& preferred_types) const;

  size_t min_word_length_ = 2;
  std::vector<models::WordType> embedding_preferred_types_;
  std::vector<models::WordType> dictionary_preferred_types_;
  std::unordered_set<std::string> blacklisted_words_;
};

}  // namespace contexto
