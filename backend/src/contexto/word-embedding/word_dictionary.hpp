#pragma once

#include <contexto/models/dictionary_word.hpp>
#include <userver/utils/assert.hpp>

namespace contexto {

class WordDictionary {
public:
  WordDictionary() = default;
  WordDictionary(const WordDictionary&) = delete;
  WordDictionary(WordDictionary&&) noexcept = default;
  ~WordDictionary() = default;

  bool LoadFromVectorFile(std::string_view file_path, bool load_dictionary_from_embeddings = false) {
    return LoadFromVectorFileWithFilter(file_path, models::WordType::kAny, load_dictionary_from_embeddings);
  }

  bool LoadFromVectorFileWithFilter(std::string_view file_path, models::WordType type_filter,
                                    bool load_dictionary_from_embeddings = false);

  bool LoadDictionary(std::string_view dictionary_path, size_t max_words = 0) {
    return LoadDictionaryWithFilter(dictionary_path, models::WordType::kAny, max_words);
  }

  bool LoadDictionaryWithFilter(std::string_view dictionary_path, models::WordType type_filter, size_t max_words = 0);

  const models::DictionaryWord* FindWord(std::string_view word) const {
    const auto it = word_with_pos_index_.find(word);
    if (it != word_with_pos_index_.end()) {
      return &words_with_embeddings_[it->second];
    }

    return nullptr;
  }

  float CalculateSimilarity(std::string_view word1, std::string_view word2) const;

  bool ContainsWord(std::string_view word) const {
    return word_with_pos_index_.contains(word) || word_to_words_with_pos_.contains(word);
  }

  bool DictionaryContains(std::string_view word) const { return words_lookup_.contains(word); }

  const models::DictionaryWord* GetRandomWord() const;
  const models::DictionaryWord* GetRandomWordByType(models::WordType type) const;
  std::vector<const models::DictionaryWord*> GetRandomWords(size_t count) const;
  std::vector<const models::DictionaryWord*> GetRandomWordsByType(models::WordType type, size_t count) const;

  std::vector<std::pair<const models::DictionaryWord*, float>> GetMostSimilarWords(std::string_view word,
                                                                                   size_t count = 10) const;

  const models::DictionaryWord& GetWordWithEmbeddingByIndex(size_t index) const noexcept {
    UASSERT_MSG(index < words_with_embeddings_.size(),
                "Failed to get word with embeddings by index: index is out of range");
    return words_with_embeddings_[index];
  }

  const models::DictionaryWord* TryGetWordWithEmbeddingByIndex(size_t index) const noexcept {
    if (index < words_with_embeddings_.size()) {
      return &words_with_embeddings_[index];
    }
    return nullptr;
  }

  std::span<const size_t> GetIndicesToWordPOSVariations(std::string_view word) const noexcept {
    if (models::WordHasPOS(word)) {
      word = models::GetWordFromWordWithPOS(word);
    }

    const auto it = word_to_words_with_pos_.find(word);
    if (it == word_to_words_with_pos_.end()) return {};
    return it->second;
  }

  WordDictionary& operator=(const WordDictionary&) = delete;
  WordDictionary& operator=(WordDictionary&&) noexcept = default;

  size_t EmbeddingsSize() const noexcept { return words_with_embeddings_.size(); }

  size_t DictionarySize() const noexcept { return words_.size(); }

  bool HasDedicatedDictionary() const noexcept { return has_dedicated_dictionary_; }

private:
  void BuildIndices();
  static std::string NormalizeWord(std::string_view word) { return utils::utf8::ToLower(word); }

  std::vector<models::DictionaryWord> words_with_embeddings_;

  std::vector<std::string_view> words_;
  std::unordered_set<std::string_view> words_lookup_;  // For fast lookup

  // Use indices instead of pointers
  std::unordered_map<std::string_view, size_t> word_with_pos_index_;
  std::unordered_map<std::string_view, std::vector<size_t>> word_to_words_with_pos_;
  std::unordered_map<models::WordType, std::vector<size_t>> type_index_;
  std::unordered_map<models::WordType, std::vector<size_t>> dict_type_index_;

  bool has_dedicated_dictionary_ = false;

  // Random number generator for word selection
  mutable std::mt19937 rng_{std::random_device{}()};
};

}  // namespace contexto
