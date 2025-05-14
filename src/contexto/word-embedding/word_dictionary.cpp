#include "word_dictionary.hpp"

#include <userver/logging/log.hpp>
#include <utility>

namespace contexto {

bool WordDictionary::LoadFromVectorFileWithFilter(std::string_view file_path, models::WordType type_filter,
                                                  size_t max_words) {
  std::ifstream file(file_path.data());
  if (!file.is_open()) {
    LOG_ERROR() << "Failed to open vector file: " << file_path;
    return false;
  }

  // Read header: vocabulary_size vector_size
  size_t vocabulary_size = 0;
  size_t vector_size = 0;
  std::string line;

  if (!std::getline(file, line)) {
    LOG_ERROR() << "Failed to read header from vector file";
    return false;
  }

  std::istringstream header(line);
  header >> vocabulary_size >> vector_size;

  if (vector_size <= 0) {
    LOG_ERROR() << "Unexpected embedding dimension: " << vector_size << ", expected: " << models::kEmbeddingDimension;
    return false;
  }

  LOG_INFO() << "Loading word embeddings with dimension " << vector_size << " filtered by word type "
             << std::to_underlying(type_filter);

  // Clear and pre-allocate storage
  words_.clear();
  word_index_.clear();
  normalized_word_index_.clear();
  type_index_.clear();

  // We don't know how many words will pass the filter, so we allocate conservatively
  const size_t estimated_capacity = max_words > 0 ? max_words : vocabulary_size / 4;
  words_.reserve(estimated_capacity);
  word_index_.reserve(estimated_capacity);
  normalized_word_index_.reserve(estimated_capacity);

  size_t loaded_words = 0;
  size_t filtered_words = 0;
  while (std::getline(file, line) && (max_words == 0 || loaded_words < max_words)) {
    std::istringstream iss(line);
    std::string word_with_pos;
    if (!(iss >> word_with_pos)) continue;  // Skip invalid lines

    // Parse word and POS tag
    std::string word;
    models::WordType word_type = models::WordType::kUnknown;

    // Check if the word has a POS tag (format: "word_POS")
    const size_t pos_separator = word_with_pos.find('_');
    if (pos_separator != std::string::npos) {
      const std::string_view word_view(word_with_pos.data(), pos_separator);
      const std::string_view pos_tag_view(word_with_pos.data() + pos_separator + 1,
                                          word_with_pos.size() - pos_separator - 1);
      word = std::string(word_view);
      word_type = models::GetWordTypeFromPOS(pos_tag_view);
    } else {
      // If no POS tag, use the whole string as the word
      word = word_with_pos;
    }

    // Skip words that are too short or have special characters
    if (word.length() < 2) continue;

    // Skip words that don't match our filter
    if (type_filter != models::WordType::kAny && word_type != type_filter) {
      ++filtered_words;
      continue;
    }

    // Create a new dictionary word
    models::DictionaryWord dict_word;
    dict_word.word = word;
    dict_word.normalized_word = NormalizeWord(word);
    dict_word.type = word_type;

    // Read embedding values
    for (size_t i = 0; i < vector_size; ++i) {
      float value = 0.0f;
      if (!(iss >> value)) break;  // Skip if we can't read the entire vector
      dict_word.embedding.resize(vector_size);
      dict_word.embedding[i] = value;
    }

    // Normalize the embedding vector
    const float norm = dict_word.embedding.norm();
    if (norm > 0) {
      dict_word.embedding /= norm;
    }

    // Store the word with its normalized embedding
    const size_t index = words_.size();
    words_.push_back(std::move(dict_word));
    const auto& dict_word_ref = words_[index];
    word_index_[dict_word_ref.word] = index;
    normalized_word_index_[dict_word_ref.normalized_word] = index;
    type_index_[dict_word_ref.type].push_back(index);

    ++loaded_words;

    if (loaded_words % 10000 == 0) {
      LOG_INFO() << "Loaded " << loaded_words << " word embeddings (filtered out " << filtered_words << " words)";
    }
  }

  LOG_INFO() << "Successfully loaded " << words_.size() << " word embeddings after filtering (skipped "
             << filtered_words << " words that didn't match the filter)";
  return !words_.empty();
}

const models::DictionaryWord* WordDictionary::FindWord(std::string_view word) const {
  // First, try direct lookup with original word
  const auto it = word_index_.find(word);
  if (it != word_index_.end()) {
    return &words_[it->second];
  }

  // If not found, try with normalized version
  const std::string normalized = NormalizeWord(word);
  const auto normalized_it = normalized_word_index_.find(normalized);
  if (normalized_it != normalized_word_index_.end()) {
    return &words_[normalized_it->second];
  }

  return nullptr;  // Word not found in dictionary
}

std::vector<const models::DictionaryWord*> WordDictionary::GetRandomWords(size_t count) const {
  if (words_.empty()) return {};

  std::vector<const models::DictionaryWord*> result;
  result.reserve(std::min(count, words_.size()));

  if (count >= words_.size()) {
    // Return all words if requested count is larger than dictionary
    for (const auto& word : words_) {
      result.push_back(&word);
    }
    return result;
  }

  // Fisher-Yates shuffle algorithm for efficient random selection
  std::vector<size_t> indices(words_.size());
  std::iota(indices.begin(), indices.end(), 0);

  for (size_t i = 0; i < count; ++i) {
    std::uniform_int_distribution<size_t> dist(i, indices.size() - 1);
    const size_t index = dist(rng_);
    std::swap(indices[i], indices[index]);
    result.push_back(&words_[indices[i]]);
  }

  return result;
}

std::vector<const models::DictionaryWord*> WordDictionary::GetRandomWordsByType(models::WordType type,
                                                                                size_t count) const {
  const auto it = type_index_.find(type);
  if (it == type_index_.end() || it->second.empty()) {
    return {};
  }

  const auto& indices = it->second;
  std::vector<const models::DictionaryWord*> result;
  result.reserve(std::min(count, indices.size()));

  if (count >= indices.size()) {
    // Return all words of requested type if count is larger
    for (const size_t idx : indices) {
      result.push_back(&words_[idx]);
    }
    return result;
  }

  // Select random indices without replacement
  std::vector<size_t> selected_indices(indices);
  for (size_t i = 0; i < count; ++i) {
    std::uniform_int_distribution<size_t> dist(i, selected_indices.size() - 1);
    const size_t j = dist(rng_);
    std::swap(selected_indices[i], selected_indices[j]);
    result.push_back(&words_[selected_indices[i]]);
  }

  return result;
}

const models::DictionaryWord* WordDictionary::GetRandomWord() const {
  if (words_.empty()) return nullptr;
  std::uniform_int_distribution<size_t> dist(0, words_.size() - 1);
  return &words_[dist(rng_)];
}

float WordDictionary::CalculateSimilarity(std::string_view word1, std::string_view word2) const {
  // If words are identical, return perfect similarity
  if (word1 == word2) return 1.0f;

  const models::DictionaryWord* dict_word1 = FindWord(word1);
  const models::DictionaryWord* dict_word2 = FindWord(word2);

  if (!dict_word1 || !dict_word2) {
    // Fall back to a simple string-based metric if embeddings aren't available
    const std::string norm1 = NormalizeWord(word1);
    const std::string norm2 = NormalizeWord(word2);

    // If normalized words are identical, return high similarity
    if (norm1 == norm2) return 0.9f;

    // Simple fallback using common prefix length
    const size_t common_prefix = utils::utf8::CommonPrefixLength(norm1, norm2);
    const size_t max_len = std::max(utils::utf8::CharCount(norm1), utils::utf8::CharCount(norm2));

    if (max_len == 0) return 0.0f;
    return static_cast<float>(common_prefix) / max_len;
  }

  // Both words have embeddings
  return dict_word1->CalculateSimilarity(*dict_word2);
}

std::vector<std::pair<const models::DictionaryWord*, float>> WordDictionary::GetMostSimilarWords(std::string_view word,
                                                                                                 size_t count) const {
  const models::DictionaryWord* dict_word = FindWord(word);
  if (!dict_word) return {};  // Word not found

  // Calculate similarity with all words
  using WordWithSimilarity = std::pair<const models::DictionaryWord*, float>;
  std::vector<WordWithSimilarity> similarities;
  similarities.reserve(words_.size());

  for (const auto& other_word : words_) {
    if (other_word.word == dict_word->word) continue;  // Skip the same word
    const float sim = dict_word->CalculateSimilarity(other_word);
    similarities.emplace_back(&other_word, sim);
  }

  // Sort by similarity in descending order
  std::partial_sort(
      similarities.begin(), similarities.begin() + std::min(count, similarities.size()), similarities.end(),
      [](const WordWithSimilarity& lhs, const WordWithSimilarity& rhs) { return lhs.second > rhs.second; });

  // Return top N results
  similarities.resize(std::min(count, similarities.size()));
  return similarities;
}

}  // namespace contexto
