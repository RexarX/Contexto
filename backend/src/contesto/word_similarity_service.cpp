#include "word_similarity_service.hpp"

#include <userver/components/component_context.hpp>
#include <userver/logging/log.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/utils/assert.hpp>

#include <fmt/format.h>

namespace contesto {

WordSimilarityService::WordSimilarityService(const userver::components::ComponentConfig&,
                                             const userver::components::ComponentContext& context)
/*: pg_cluster_(context.FindComponent<userver::components::Postgres>("postgres-db-1").GetCluster())*/ {
  LoadDictionary();
  LoadWordEmbeddings();
}

std::string WordSimilarityService::GenerateNewTargetWord() {
  std::random_device rd;
  std::mt19937 generator(rd());

  if (dictionary_.empty()) {
    throw std::runtime_error("Dictionary is empty, cannot generate target word");
  }

  std::uniform_int_distribution<size_t> distribution(0, dictionary_.size() - 1);
  return dictionary_[distribution(generator)];
}

std::vector<models::Word> WordSimilarityService::GetSimilarWords(std::string_view word, std::string_view target_word) {
  if (!ValidateWord(word) || !ValidateWord(target_word)) {
    return {};
  }

  std::vector<models::Word> similar_words;

  // For now, just provide a ranking based on similarity to target word
  const double similarity = CalculateSimilarity(word, target_word);

  // If the word is exactly the target word, rank is 1
  const int64_t rank = (word == target_word) ? 1 : static_cast<int>(100.0 * (1.0 - similarity)) + 2;

  similar_words.push_back({word.data(), similarity, rank});

  return similar_words;
}

double WordSimilarityService::CalculateSimilarity(std::string_view word1, std::string_view word2) {
  // Simple case - exact match
  if (word1 == word2) {
    return 1.0;
  }

  // Check if both words are in our embeddings
  const auto it1 = word_embeddings_.find(word1);
  const auto it2 = word_embeddings_.find(word2);

  if (it1 == word_embeddings_.end() || it2 == word_embeddings_.end()) {
    // Fall back to a basic string similarity if no embeddings

    // Simple Levenshtein-like measure
    const size_t len1 = word1.length();
    const size_t len2 = word2.length();
    const size_t max_len = std::max(len1, len2);

    // Both empty strings
    if (max_len == 0) {
      return 1.0;
    }

    // Common prefix
    size_t common_prefix = 0;
    const size_t min_len = std::min(len1, len2);
    for (size_t i = 0; i < min_len; ++i) {
      if (word1[i] == word2[i]) {
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

  if (norm1 == 0.0 || norm2 == 0.0) {
    return 0.0;
  }

  return dot_product / (std::sqrt(norm1) * std::sqrt(norm2));
}

void WordSimilarityService::LoadWordEmbeddings() {
  // In a real application, you would load pre-trained word embeddings
  // For this example, we'll create some simple dummy embeddings

  // This would typically load from a file or database
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
  // In a real application, load from a file or database
  // For this example, we'll use a small set of hardcoded words
  dictionary_ = {"кот",       "собака", "дом",    "машина",  "дерево",  "стол",   "книга",  "ручка", "телефон",
                 "компьютер", "окно",   "дверь",  "человек", "ребенок", "город",  "страна", "море",  "река",
                 "гора",      "небо",   "солнце", "луна",    "звезда",  "цветок", "трава"};

  LOG_INFO() << "Loaded " << dictionary_.size() << " words in dictionary";
}

}  // namespace contesto