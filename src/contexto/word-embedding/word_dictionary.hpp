#pragma once

#include <contexto/models/dictionary_word.hpp>

namespace contexto {

class WordDictionary {
public:
  WordDictionary() = default;
  WordDictionary(const WordDictionary&) = default;
  WordDictionary(WordDictionary&&) = default;
  ~WordDictionary() = default;

  bool LoadFromVectorFile(std::string_view file_path, size_t max_words = 0) {
    return LoadFromVectorFileWithFilter(file_path, models::WordType::kAny, max_words);
  }

  bool LoadFromVectorFileWithFilter(std::string_view file_path, models::WordType type_filter, size_t max_words = 0);

  const models::DictionaryWord* FindWord(std::string_view word) const;

  std::vector<const models::DictionaryWord*> GetRandomWords(size_t count) const;
  std::vector<const models::DictionaryWord*> GetRandomWordsByType(models::WordType type, size_t count) const;
  const models::DictionaryWord* GetRandomWord() const;

  float CalculateSimilarity(std::string_view word1, std::string_view word2) const;

  bool ContainsWord(std::string_view word) const { return FindWord(word) != nullptr; }

  std::vector<std::pair<const models::DictionaryWord*, float>> GetMostSimilarWords(std::string_view word,
                                                                                    size_t count = 10) const;

  WordDictionary& operator=(const WordDictionary&) = default;
  WordDictionary& operator=(WordDictionary&&) noexcept = default;

  size_t Size() const noexcept { return words_.size(); }

private:
  static std::string NormalizeWord(std::string_view word) { return utils::utf8::ToLower(word); }

  // Main storage for all words
  std::vector<models::DictionaryWord> words_;

  // Indexes for efficient lookups
  std::unordered_map<std::string_view, size_t> word_index_;             // Original word -> index
  std::unordered_map<std::string_view, size_t> normalized_word_index_;  // Normalized word -> index

  // Index by word type
  std::unordered_map<models::WordType, std::vector<size_t>> type_index_;

  // Random number generator for word selection
  mutable std::mt19937 rng_{std::random_device{}()};
};

}  // namespace context
