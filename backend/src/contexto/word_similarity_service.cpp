#include "word_similarity_service.hpp"

#include <userver/logging/log.hpp>
#include <userver/utils/assert.hpp>

#include <fmt/format.h>

namespace contexto {

void WordSimilarityService::InitializeDictionary() {
  constexpr std::string_view embeddings_path = "assets/ruwikiruscorpora-nobigrams_upos_skipgram_300_5_2018.vec";

  const bool loaded = dictionary_.LoadFromVectorFile(embeddings_path, max_dictionary_words_);

  if (!loaded || dictionary_.Size() == 0) {
    LOG_ERROR() << "Failed to load word embeddings dictionary";
    throw std::runtime_error("Failed to initialize word dictionary");
  }

  LOG_INFO() << "Successfully loaded dictionary with " << dictionary_.Size() << " words";
}

void WordSimilarityService::BuildAdditionalWordsList() {
  // Here we can add custom words that we want to include in the game
  // This can be used to add thematic words or special words that may not be in the dictionary

  // Load additional words from file if needed
  std::ifstream additional_words_file("assets/additional_words.txt");
  if (additional_words_file.is_open()) {
    std::string word;
    while (std::getline(additional_words_file, word)) {
      if (!word.empty() && dictionary_.ContainsWord(word)) {
        additional_words_.push_back(word);
      }
    }
    LOG_INFO() << "Loaded " << additional_words_.size() << " additional words";
  }
}

std::string WordSimilarityService::GenerateNewTargetWord() {
  static std::mt19937 rng(static_cast<uint_fast32_t>(std::chrono::steady_clock::now().time_since_epoch().count()));

  // Prefer nouns for target words as they work better for the game
  const auto noun_candidates = dictionary_.GetRandomWordsByType(preferred_word_type_, 10);

  if (!noun_candidates.empty()) {
    std::uniform_int_distribution<size_t> dist(0, noun_candidates.size() - 1);
    const auto& selected = noun_candidates[dist(rng)];
    LOG_INFO() << "Selected target word: " << selected->word << " (type: noun)";
    return selected->word;
  }

  // If no nouns available or as fallback, select any word
  const auto* random_word = dictionary_.GetRandomWord();
  if (random_word) {
    LOG_INFO() << "Selected random target word: " << random_word->word;
    return random_word->word;
  }

  // Last resort: use from additional words list if available
  if (!additional_words_.empty()) {
    std::uniform_int_distribution<size_t> dist(0, additional_words_.size() - 1);
    return additional_words_[dist(rng)];
  }

  throw std::runtime_error("Dictionary is empty, cannot generate target word");
}

std::vector<models::Word> WordSimilarityService::GetSimilarWords(std::string_view word, std::string_view target_word) {
  if (!ValidateWord(word) || !ValidateWord(target_word)) {
    LOG_WARNING() << "Invalid words: " << word << " or " << target_word;
    return {};
  }

  std::vector<models::Word> similar_words;

  // Calculate similarity between the guess and target word
  const double similarity = CalculateSimilarity(word, target_word);
  LOG_INFO() << "Similarity between " << word << " and " << target_word << ": " << similarity;

  // Get rank based on semantic similarity
  int64_t rank = 0;

  if (word == target_word) {
    // Exact match
    rank = 1;
  } else {
    // Find similar words to the target to determine relative ranking
    const auto similar_to_target = dictionary_.GetMostSimilarWords(target_word, 100);

    // Find the position of the guessed word in the similarity ranking
    const auto it = std::find_if(similar_to_target.begin(), similar_to_target.end(),
                                 [&word](const auto& pair) { return pair.first->word == word; });

    if (it != similar_to_target.end()) {
      // Word is in the top 100 similar words
      rank = std::distance(similar_to_target.begin(), it) + 2;  // +2 because rank 1 is exact match
    } else {
      // Word is not in top similar words, calculate based on similarity score
      if (similarity < 0.01) {
        rank = 100;  // Very dissimilar
      } else {
        // Map similarity from 0-1 to rank 100-2
        // Higher similarity = lower rank (closer to target)
        rank = static_cast<int64_t>(std::max(2.0, std::round(100.0 - (similarity * 98.0))));
      }
    }
  }

  LOG_INFO() << "Final rank for " << word << ": " << rank << " (similarity: " << similarity << ")";

  models::Word model_word{.id = std::string(word), .similarity_score = similarity, .rank = rank};
  similar_words.push_back(std::move(model_word));

  return similar_words;
}

}  // namespace contexto
