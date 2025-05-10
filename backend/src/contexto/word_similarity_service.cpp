#include "word_similarity_service.hpp"

#include <userver/components/component_context.hpp>
#include <userver/logging/log.hpp>
#include <userver/utils/assert.hpp>

#include <fmt/format.h>

namespace contexto {

WordSimilarityService::WordSimilarityService(const userver::components::ComponentConfig&,
                                             const userver::components::ComponentContext& context) {
  LoadDictionary();
  LoadWordEmbeddings();
}

std::string WordSimilarityService::GenerateNewTargetWord() {
  static std::linear_congruential_engine<uint_fast32_t, 1664525, 1013904223, std::numeric_limits<uint_fast32_t>::max()>
      engine(static_cast<uint_fast32_t>(std::chrono::steady_clock::now().time_since_epoch().count()));

  std::random_device rd;
  std::mt19937 generator(rd());

  if (dictionary_.empty()) {
    throw std::runtime_error("Dictionary is empty, cannot generate target word");
  }

  const size_t index = engine() % dictionary_.size();
  return dictionary_[index];
}

std::vector<models::Word> WordSimilarityService::GetSimilarWords(std::string_view word, std::string_view target_word) {
  if (!ValidateWord(word) || !ValidateWord(target_word)) return {};

  std::vector<models::Word> similar_words;

  // For now, just provide a ranking based on similarity to target word
  const double similarity = CalculateSimilarity(word, target_word);

  // If the word is exactly the target word, rank is 1
  const int64_t rank = (word == target_word) ? 1 : static_cast<int>(100.0 * (1.0 - similarity)) + 2;

  similar_words.push_back({word.data(), similarity, rank});

  return similar_words;
}

double WordSimilarityService::CalculateSimilarity(std::string_view lhs, std::string_view rhs) const {
  // Simple case - exact match
  if (lhs == rhs) return 1.0;

  // Check if both words are in our embeddings
  const auto it1 = word_embeddings_.find(lhs);
  const auto it2 = word_embeddings_.find(rhs);

  if (it1 == word_embeddings_.end() || it2 == word_embeddings_.end()) {
    // Fall back to a basic string similarity if no embeddings

    // Simple Levenshtein-like measure
    const size_t len1 = lhs.length();
    const size_t len2 = rhs.length();
    const size_t max_len = std::max(len1, len2);

    // Both empty strings
    if (max_len == 0) return 1.0;

    // Common prefix
    size_t common_prefix = 0;
    const size_t min_len = std::min(len1, len2);
    for (size_t i = 0; i < min_len; ++i) {
      if (lhs[i] == rhs[i]) {
        ++common_prefix;
      } else {
        break;
      }
    }

    return static_cast<double>(common_prefix) / max_len;
  }

  // Compute cosine similarity between word embeddings
  const auto& vec1 = it1->second;
  const auto& vec2 = it2->second;

  double dot_product = 0.0;
  double norm1 = 0.0;
  double norm2 = 0.0;

  for (size_t i = 0; i < vec1.size(); ++i) {
    dot_product += vec1[i] * vec2[i];
    norm1 += vec1[i] * vec1[i];
    norm2 += vec2[i] * vec2[i];
  }

  if (norm1 == 0.0 || norm2 == 0.0) return 0.0;

  return dot_product / (std::sqrt(norm1) * std::sqrt(norm2));
}

void WordSimilarityService::LoadWordEmbeddings() {
  for (const auto& word : dictionary_) {
    std::vector<float> embedding;
    embedding.reserve(50);  // 50-dimensional embeddings

    // Create deterministic but unique embeddings for each word
    std::hash<std::string> hasher;
    std::mt19937 gen(hasher(word));
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    for (int i = 0; i < 50; ++i) {
      embedding.push_back(dist(gen));
    }

    // Normalize
    float norm = 0.0f;
    for (const auto val : embedding) {
      norm += val * val;
    }
    norm = std::sqrt(norm);

    if (norm > 0.0f) {
      for (auto& val : embedding) {
        val /= norm;
      }
    }

    word_embeddings_[word] = std::move(embedding);
  }

  LOG_INFO() << "Loaded " << word_embeddings_.size() << " word embeddings";
}

void WordSimilarityService::LoadDictionary() {
  std::ifstream dictionary_file("assets/dictionary.txt");
  if (!dictionary_file.is_open()) {
    LOG_ERROR() << "Failed to open assets/dictionary.txt";
    return;
  }

  dictionary_.clear();

  constexpr size_t INCLUDED_WORDS_COUNT = 3000;
  size_t already_included = 0;

  std::string word;
  while (std::getline(dictionary_file, word)) {
    // Skip empty lines or lines that might be comments
    if (word.empty() || word.starts_with("//")) continue;

    // Remove numbers (including float numbers) before the word itself
    const size_t first_non_digit = word.find_first_not_of("0123456789. ");
    if (first_non_digit != std::string::npos) {
      word = word.substr(first_non_digit);
    }

    if (word.empty() || word.starts_with("//")) continue;
    dictionary_.push_back(word);

    if (++already_included >= INCLUDED_WORDS_COUNT) break;
  }

  dictionary_file.close();
  LOG_INFO() << "Loaded " << dictionary_.size() << " words in dictionary";
}

std::string WordSimilarityService::GetHintWord(std::string_view target_word,
                                               std::span<const std::string> guessed_words) const {
  std::unordered_set<std::string_view> guessed_set;
  for (const auto& word : guessed_words) {
    guessed_set.insert(word);
  }

  // Find a similar but not identical word
  std::string best_hint;
  double best_similarity = 0.0;

  // We want a word that's close but not too obvious
  // Aiming for a similarity between 0.6 and 0.8 is good
  const double target_similarity_min = 0.6;
  const double target_similarity_max = 0.8;

  if (dictionary_.empty()) {
    LOG_ERROR() << "Dictionary is empty when trying to find a hint word";
    return "подсказка";  // Default hint if dictionary is empty
  }

  for (const auto& candidate : dictionary_) {
    // Skip if this word was already guessed and skip the actual target word - we don't want to give it away
    if (guessed_set.contains(candidate) || candidate == target_word) continue;

    const double similarity = CalculateSimilarity(candidate, target_word);

    // If this is our first candidate or if it's better than what we have
    if (best_hint.empty() || (similarity >= target_similarity_min && similarity <= target_similarity_max &&
                              std::abs(similarity - 0.7) < std::abs(best_similarity - 0.7))) {
      best_hint = candidate;
      best_similarity = similarity;
    }
  }

  // If we couldn't find a good hint in the desired range, just return the most similar word
  if (best_hint.empty()) {
    for (const auto& candidate : dictionary_) {
      if (guessed_set.contains(candidate) || candidate == target_word) continue;

      const double similarity = CalculateSimilarity(candidate, target_word);

      if (best_hint.empty() || similarity > best_similarity) {
        best_hint = candidate;
        best_similarity = similarity;
      }
    }
  }

  if (best_hint.empty()) {
    LOG_WARNING() << "Could not find any hint word, returning default word";
    return "подсказка";
  }

  LOG_INFO() << "Generated hint: " << best_hint << " with similarity " << best_similarity;
  return best_hint;
}

}  // namespace contexto
