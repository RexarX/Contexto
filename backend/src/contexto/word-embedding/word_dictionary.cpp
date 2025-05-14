#include "word_dictionary.hpp"

#include <userver/logging/log.hpp>

namespace contexto {

bool WordDictionary::LoadFromVectorFileWithFilter(std::string_view file_path, models::WordType type_filter,
                                                  bool load_dictionary_from_embeddings) {
  std::ifstream file(file_path.data());
  if (!file.is_open()) {
    LOG_ERROR() << "Failed to open vector file: " << file_path;
    return false;
  }

  // Read header: vocabulary_size vector_size
  int64_t vocabulary_size = 0;
  int64_t vector_size = 0;
  std::string line;

  if (!std::getline(file, line)) {
    LOG_ERROR() << "Failed to read header from vector file";
    return false;
  }

  std::istringstream header(line);
  header >> vocabulary_size >> vector_size;

  if (vocabulary_size < 0) {
    LOG_ERROR() << "Vocabulary size cannot be negative";
    LOG_WARNING() << "Vocabulary size will be detected automaticly";
  }

  if (vector_size < 0) {
    LOG_ERROR() << "Vector size cannot be negative";
    return false;
  }

  LOG_INFO() << "Loading word embeddings with dimension " << vector_size << " filtered by word type "
             << std::to_underlying(type_filter);

  // Clear and pre-allocate storage
  words_with_embeddings_.clear();
  word_with_pos_index_.clear();
  word_to_words_with_pos_.clear();
  type_index_.clear();

  if (load_dictionary_from_embeddings) {
    words_.clear();
    words_lookup_.clear();
  }

  // We don't know how many words will pass the filter, so we allocate conservatively
  const size_t estimated_capacity = vocabulary_size / 4;
  words_with_embeddings_.reserve(estimated_capacity);
  word_with_pos_index_.reserve(estimated_capacity);
  word_to_words_with_pos_.reserve(estimated_capacity);

  if (load_dictionary_from_embeddings) {
    words_.reserve(estimated_capacity);
    words_lookup_.reserve(estimated_capacity);
  }

  size_t loaded_words = 0;
  size_t filtered_words = 0;
  while (std::getline(file, line)) {
    std::istringstream iss(line);
    std::string word_with_pos;
    if (!(iss >> word_with_pos)) continue;  // Skip invalid lines

    // Parse word and POS tag
    models::WordType word_type = models::WordType::kUnknown;

    // Check if the word has a POS tag (format: "word_POS")
    const size_t pos_separator = word_with_pos.find('_');
    if (pos_separator != std::string::npos) {
      const std::string_view word_view(word_with_pos.data(), pos_separator);
      const std::string_view pos_tag_view(word_with_pos.data() + pos_separator + 1,
                                          word_with_pos.size() - pos_separator - 1);
      word_type = models::GetWordTypeFromPOS(pos_tag_view);
      if (type_filter != models::WordType::kAny && word_type != type_filter) {
        ++filtered_words;
        continue;
      }
    }

    models::DictionaryWord dict_word{.word_with_pos = std::move(word_with_pos), .type = word_type};
    const std::string_view word = models::GetWordFromWordWithPOS(dict_word.word_with_pos);
    if (word.length() < 2) continue;  // Skip words that are too short
    dict_word.embedding.resize(vector_size);

    // Read embedding values
    for (int64_t i = 0; i < vector_size; ++i) {
      float value = 0.0f;
      if (!(iss >> value)) break;  // Skip if we can't read the entire vector
      dict_word.embedding[i] = value;
    }

    // Normalize the embedding vector
    const float norm = dict_word.embedding.norm();
    if (norm > 0) {
      dict_word.embedding /= norm;
    }

    // Store the word with its normalized embedding
    const size_t word_idx = words_with_embeddings_.size();
    words_with_embeddings_.push_back(std::move(dict_word));
    auto& ref = words_with_embeddings_[word_idx];
    ref.word = models::GetWordFromWordWithPOS(ref.word_with_pos);
    word_with_pos_index_[ref.word_with_pos] = word_idx;
    word_to_words_with_pos_[ref.word].push_back(word_idx);
    type_index_[ref.type].push_back(word_idx);

    // If we're using embeddings as dictionary
    if (load_dictionary_from_embeddings) {
      if (!words_lookup_.contains(ref.word)) {
        words_.push_back(ref.word);
        words_lookup_.insert(ref.word);
      }
    }

    ++loaded_words;

    if (loaded_words % 10000 == 0) {
      LOG_INFO() << "Loaded " << loaded_words << " word embeddings (filtered out " << filtered_words << " words)";
    }
  }

  LOG_INFO() << "Successfully loaded " << words_with_embeddings_.size() << " word embeddings after filtering (skipped "
             << filtered_words << " words that didn't match the filter)";

  if (load_dictionary_from_embeddings) {
    has_dedicated_dictionary_ = false;
    LOG_INFO() << "Using " << words_.size() << " words from embeddings as dictionary";
  }

  if (load_dictionary_from_embeddings) {
    words_.shrink_to_fit();
  }

  return !words_with_embeddings_.empty();
}

bool WordDictionary::LoadDictionaryWithFilter(std::string_view dictionary_path, models::WordType type_filter,
                                              size_t max_words) {
  std::ifstream file(dictionary_path.data());
  if (!file.is_open()) {
    LOG_ERROR() << "Failed to open dictionary file: " << dictionary_path;
    return false;
  }

  words_.clear();
  words_lookup_.clear();
  dict_type_index_.clear();

  // Read number of words if present
  std::string line;
  size_t word_count = 0;
  if (std::getline(file, line)) {
    try {
      word_count = std::stoul(line);
      words_.reserve(word_count);
      words_lookup_.reserve(word_count);

    } catch (const std::exception& e) {
      LOG_WARNING() << "Failed to parse word count, using the line as a word: " << e.what();
      // Treat the first line as a word
      file.seekg(0);
    }
  }

  size_t loaded_words = 0;
  size_t skipped_words = 0;

  while (std::getline(file, line) && (max_words == 0 || loaded_words < max_words)) {
    // Remove trailing whitespace
    line = line.substr(0, line.find_last_not_of(" \t\r\n") + 1);
    if (line.empty()) continue;

    // Parse word and POS tag if present
    models::WordType word_type = models::WordType::kUnknown;
    const size_t pos_separator = line.find('_');
    if (pos_separator != std::string::npos) {
      const std::string_view pos_tag_view(line.data() + pos_separator + 1, line.size() - pos_separator - 1);
      word_type = models::GetWordTypeFromPOS(pos_tag_view);
      if (type_filter != models::WordType::kAny && word_type != type_filter) continue;
    } else {
      word_type = type_filter;
    }

    // Only add the word if we don't already have it
    if (!words_lookup_.contains(line)) {
      // Try to find in embeddings
      const auto it = word_with_pos_index_.find(line);
      if (it != word_with_pos_index_.end()) {
        words_.push_back(words_with_embeddings_[it->second].word_with_pos);
        words_lookup_.insert(words_.back());
        dict_type_index_[word_type].push_back(it->second);
        ++loaded_words;
      } else {
        ++skipped_words;
        continue;
      }
    } else {
      ++skipped_words;
    }
  }

  has_dedicated_dictionary_ = true;
  words_.shrink_to_fit();

  LOG_INFO() << "Loaded " << loaded_words << " unique words from dedicated dictionary (skipped " << skipped_words
             << " duplicates or filtered words)";
  return !words_.empty();
}

float WordDictionary::CalculateSimilarity(std::string_view word1, std::string_view word2) const {
  // If words are identical, return perfect similarity
  if (word1 == word2) return 1.0f;

  const models::DictionaryWord* dict_word1 = FindWord(word1);
  const models::DictionaryWord* dict_word2 = FindWord(word2);

  if (!dict_word1) {
    LOG_ERROR() << "'" << word1 << "' was not found in dictionary";
    return -1.0f;
  }

  if (!dict_word2) {
    LOG_ERROR() << "'" << word2 << "' was not found in dictionary";
    return -1.0f;
  }

  // Both words have embeddings
  return dict_word1->CalculateSimilarity(*dict_word2);
}

const models::DictionaryWord* WordDictionary::GetRandomWord() const {
  if (words_with_embeddings_.empty()) {
    LOG_ERROR() << "No words available in dictionary";
    return nullptr;
  }

  if (has_dedicated_dictionary_ && !words_.empty()) {
    // Use the dedicated dictionary when available
    std::uniform_int_distribution<size_t> dist(0, words_.size() - 1);
    const std::string_view word = words_[dist(rng_)];

    // Find this word in the embeddings
    const auto it = word_with_pos_index_.find(word);
    if (it != word_with_pos_index_.end()) {
      return &words_with_embeddings_[it->second];
    } else {
      LOG_WARNING() << "Word '" << word << "' from dedicated dictionary has no embedding";

      // Fall back to any word with embedding
      std::uniform_int_distribution<size_t> embed_dist(0, words_with_embeddings_.size() - 1);
      return &words_with_embeddings_[embed_dist(rng_)];
    }
  } else if (!words_with_embeddings_.empty()) {
    // Fall back to embeddings dictionary
    std::uniform_int_distribution<size_t> dist(0, words_with_embeddings_.size() - 1);
    return &words_with_embeddings_[dist(rng_)];
  }

  return nullptr;
}

const models::DictionaryWord* WordDictionary::GetRandomWordByType(models::WordType type) const {
  if (words_with_embeddings_.empty()) {
    LOG_ERROR() << "No words available in dictionary";
    return nullptr;
  }

  if (has_dedicated_dictionary_ && !words_.empty()) {
    // Use the dedicated dictionary when available
    std::uniform_int_distribution<size_t> dist(0, words_.size() - 1);
    const std::string_view word = words_[dist(rng_)];

    // Find this word in the embeddings
    const auto it = word_with_pos_index_.find(word);
    if (it != word_with_pos_index_.end()) {
      return &words_with_embeddings_[it->second];
    } else {
      LOG_WARNING() << "Word '" << word << "' from dedicated dictionary has no embedding";

      // Fall back to any word with embedding

      const auto indices_it = dict_type_index_.find(type);
      if (indices_it == dict_type_index_.end()) {
        LOG_ERROR() << "Failed to get random word by type '" << std::to_underlying(type) << "'";
        return nullptr;
      }
      const auto& indices = indices_it->second;
      std::uniform_int_distribution<size_t> embed_dist(0, indices.size() - 1);
      return &words_with_embeddings_[indices[embed_dist(rng_)]];
    }
  } else if (!words_with_embeddings_.empty()) {
    // Fall back to embeddings dictionary
    std::uniform_int_distribution<size_t> dist(0, words_with_embeddings_.size() - 1);
    return &words_with_embeddings_[dist(rng_)];
  }

  return nullptr;
}

std::vector<const models::DictionaryWord*> WordDictionary::GetRandomWords(size_t count) const {
  std::vector<const models::DictionaryWord*> result;

  if (words_with_embeddings_.empty()) {
    LOG_ERROR() << "No words with embeddings available";
    return result;
  }

  if (count == 0) return result;
  if (count == 1) {
    result.push_back(GetRandomWord());
    return result;
  }

  result.reserve(std::min(count, words_with_embeddings_.size()));

  // Generate unique random indices
  std::unordered_set<size_t> unique_indices;
  std::uniform_int_distribution<size_t> dist(0, words_with_embeddings_.size() - 1);

  while (unique_indices.size() < std::min(count, words_with_embeddings_.size())) {
    unique_indices.insert(dist(rng_));
  }

  // Collect words
  for (const size_t idx : unique_indices) {
    result.push_back(&words_with_embeddings_[idx]);
  }

  return result;
}

std::vector<const models::DictionaryWord*> WordDictionary::GetRandomWordsByType(models::WordType type,
                                                                                size_t count) const {
  std::vector<const models::DictionaryWord*> result;

  if (words_with_embeddings_.empty()) {
    LOG_ERROR() << "No words with embeddings available";
    return result;
  }

  if (count == 0) return result;
  if (count == 1) {
    result.push_back(GetRandomWordByType(type));
    return result;
  }

  // Use appropriate index based on dictionary availability
  const auto& type_indices = has_dedicated_dictionary_ ? dict_type_index_ : type_index_;

  // If type filter is "Any", just pick random words from all available
  if (type == models::WordType::kAny) {
    result.reserve(std::min(count, words_with_embeddings_.size()));

    // Generate unique random indices
    std::unordered_set<size_t> unique_indices;
    std::uniform_int_distribution<size_t> dist(0, words_with_embeddings_.size() - 1);

    while (unique_indices.size() < std::min(count, words_with_embeddings_.size())) {
      unique_indices.insert(dist(rng_));
    }

    // Collect words
    for (const size_t idx : unique_indices) {
      result.push_back(&words_with_embeddings_[idx]);
    }

    return result;
  }

  // Check for words of the specific type
  const auto it = type_indices.find(type);
  if (it == type_indices.end() || it->second.empty()) {
    LOG_WARNING() << "No words found for type '" << std::to_underlying(type) << "', falling back to any type";
    return result;
  }

  const auto& indices_of_type = it->second;
  result.reserve(std::min(count, indices_of_type.size()));

  if (count >= indices_of_type.size()) {
    // Return all words of requested type
    for (const auto idx : indices_of_type) {
      result.push_back(&words_with_embeddings_[idx]);
    }
    return result;
  }

  // Select random words without replacement
  std::unordered_set<size_t> unique_positions;
  std::uniform_int_distribution<size_t> dist(0, indices_of_type.size() - 1);

  while (unique_positions.size() < count) {
    unique_positions.insert(dist(rng_));
  }

  for (const size_t pos : unique_positions) {
    result.push_back(&words_with_embeddings_[indices_of_type[pos]]);
  }

  return result;
}

std::vector<std::pair<const models::DictionaryWord*, float>> WordDictionary::GetMostSimilarWords(std::string_view word,
                                                                                                 size_t count) const {
  const models::DictionaryWord* dict_word = FindWord(word);
  if (!dict_word) return {};  // Word not found

  // Calculate similarity with all words
  using WordWithSimilarity = std::pair<const models::DictionaryWord*, float>;
  std::vector<WordWithSimilarity> similarities;
  similarities.reserve(words_with_embeddings_.size());

  for (const auto& other_word : words_with_embeddings_) {
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
