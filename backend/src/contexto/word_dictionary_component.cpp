#include "word_dictionary_component.hpp"

#include <userver/components/component_config.hpp>
#include <userver/logging/log.hpp>
#include <userver/utils/assert.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

namespace contexto {

WordDictionaryComponent::WordDictionaryComponent(const userver::components::ComponentConfig& config,
                                                 const userver::components::ComponentContext& context)
    : LoggableComponentBase(config, context) {
  std::string embeddings_path;
  if (config.HasMember("embeddings-path")) {
    embeddings_path = config["embeddings-path"].As<std::string>();
  }

  // Parse preferred word type
  std::string embeddings_word_type_str = "any";
  if (config.HasMember("embeddings-preferred-word-type")) {
    embeddings_word_type_str = config["embeddings-preferred-word-type"].As<std::string>();
  }

  // Map string to enum
  embeddings_preferred_word_type_ = StringToWordType(embeddings_word_type_str);
  if (embeddings_preferred_word_type_ == models::WordType::kUnknown) {
    LOG_WARNING() << "Unknown word type in config: " << embeddings_word_type_str << ", defaulting to any";
    embeddings_preferred_word_type_ = models::WordType::kAny;
  }

  LOG_INFO() << "Initializing word embeddings with max words: " << max_dictionary_words_
             << ", preferred word type: " << embeddings_word_type_str;

  // Load embeddings
  bool loaded = false;
  if (embeddings_preferred_word_type_ != models::WordType::kUnknown) {
    // By default, don't load dictionary from embeddings as we'll try dedicated dictionary
    loaded = dictionary_.LoadFromVectorFileWithFilter(embeddings_path, embeddings_preferred_word_type_, true);
  } else {
    loaded = dictionary_.LoadFromVectorFile(embeddings_path, true);
  }

  if (!loaded || dictionary_.EmbeddingsSize() == 0) {
    LOG_ERROR() << "Failed to load word embeddings dictionary";
    throw std::runtime_error("Failed to initialize word dictionary");
  } else {
    LOG_INFO() << "Successfully loaded embeddings with " << dictionary_.EmbeddingsSize() << " words";
  }

  max_dictionary_words_ =
      config.HasMember("max-dictionary-words") ? config["max-dictionary-words"].As<size_t>() : 100000;

  // Parse preferred word type
  std::string dictionary_word_type_str = "any";
  if (config.HasMember("dictionary-preferred-word-type")) {
    dictionary_word_type_str = config["dictionary-preferred-word-type"].As<std::string>();
  }

  // Map string to enum
  dictionary_preferred_word_type_ = StringToWordType(dictionary_word_type_str);
  if (dictionary_preferred_word_type_ == models::WordType::kUnknown) {
    LOG_WARNING() << "Unknown word type in config: " << "'" << dictionary_word_type_str << "'" << ", defaulting to any";
    dictionary_preferred_word_type_ = models::WordType::kAny;
  }

  if (config.HasMember("dictionary-path")) {
    bool dictionary_loaded = false;
    const auto dictionary_path = config["dictionary-path"].As<std::string>();
    if (dictionary_preferred_word_type_ != models::WordType::kUnknown) {
      dictionary_loaded =
          dictionary_.LoadDictionaryWithFilter(dictionary_path, dictionary_preferred_word_type_, max_dictionary_words_);
    } else {
      dictionary_loaded = dictionary_.LoadDictionary(dictionary_path, max_dictionary_words_);
    }

    if (!dictionary_loaded) {
      LOG_WARNING() << "Failed to load dedicated dictionary from " << dictionary_path
                    << ", falling back to embeddings for dictionary";
    }
  }

  LOG_INFO() << "Dictionary loaded with " << dictionary_.DictionarySize() << " words";
}

std::optional<int> WordDictionaryComponent::CalculateRank(std::string_view guessed_word,
                                                          std::string_view target_word) const {
  if (!models::WordHasPOS(target_word)) {
    LOG_ERROR() << "Failed to calculate rank: target_word '" << target_word << "' must have a POS";
    return std::nullopt;
  }

  const std::string_view target_only_word = models::GetWordFromWordWithPOS(target_word);

  // If the words match (ignoring POS), it's rank 1
  if (guessed_word == target_only_word) return 1;

  // Calculate cosine similarity
  float cosine_sim = 0.0f;
  if (models::WordHasPOS(guessed_word)) {
    cosine_sim = dictionary_.CalculateSimilarity(guessed_word, target_word);
  } else {
    // Find best POS variant
    float best_sim = -1.0f;
    const auto indices = dictionary_.GetIndicesToWordPOSVariations(guessed_word);
    for (const size_t index : indices) {
      const models::DictionaryWord& dictionary_word = dictionary_.GetWordWithEmbeddingByIndex(index);
      const float curr_sim = dictionary_.CalculateSimilarity(dictionary_word.word_with_pos, target_word);
      if (curr_sim > best_sim) {
        best_sim = curr_sim;
      }
    }
    cosine_sim = best_sim;
  }

  if (cosine_sim == -1.0f) {
    LOG_ERROR() << "Failed to calculate similarity";
    return std::nullopt;
  }

  const size_t prefix_length = utils::utf8::CommonPrefixLength(guessed_word, target_only_word);
  const size_t min_word_length =
      std::min(utils::utf8::CharCount(guessed_word), utils::utf8::CharCount(target_only_word));

  // Longer prefixes relative to word length indicate likely shared roots
  // Calculate as a ratio but with higher weight for longer prefixes (up to 5 chars)
  const size_t effective_prefix = std::min(prefix_length, size_t(5));
  float prefix_score = static_cast<float>(effective_prefix) / std::min(size_t(5), min_word_length);
  prefix_score = std::min(1.0f, prefix_score);  // Cap at 1.0

  // Check for shared root - if prefix is substantial (>= 4 chars or >50% of shorter word)
  const bool likely_shared_root =
      (prefix_length >= 4 || (prefix_length > 0 && prefix_length >= min_word_length * 0.5f));

  // Boost similarity score for words with likely shared roots
  float morphological_bonus = 0.0f;
  if (likely_shared_root) {
    // Apply stronger bonus for words with longer shared prefixes
    morphological_bonus = 0.15f * prefix_score;
  }

  // Semantic similarity (from embeddings) is most important, but morphology matters too
  const float combined_similarity = (cosine_sim * 0.8f) + (prefix_score * 0.2f) + morphological_bonus;

  // Map combined similarity to rank using a smoother distribution curve
  int rank = 0;

  // Very similar words (likely almost synonyms)
  if (combined_similarity >= 0.95f) {
    rank = static_cast<int>(2 + (1.0f - combined_similarity) * 13 / 0.05f);
  }
  // Highly similar words
  else if (combined_similarity >= 0.85f) {
    rank = static_cast<int>(15 + (0.95f - combined_similarity) * 35 / 0.1f);
  }
  // Moderately similar words
  else if (combined_similarity >= 0.75f) {
    rank = static_cast<int>(50 + (0.85f - combined_similarity) * 50 / 0.1f);
  }
  // Somewhat related words
  else if (combined_similarity >= 0.65f) {
    rank = static_cast<int>(100 + (0.75f - combined_similarity) * 100 / 0.1f);
  }
  // Loosely related words
  else if (combined_similarity >= 0.55f) {
    rank = static_cast<int>(200 + (0.65f - combined_similarity) * 200 / 0.1f);
  }
  // Words with minor relations
  else if (combined_similarity >= 0.45f) {
    rank = static_cast<int>(400 + (0.55f - combined_similarity) * 200 / 0.1f);
  }
  // Distantly related words
  else if (combined_similarity >= 0.35f) {
    rank = static_cast<int>(600 + (0.45f - combined_similarity) * 200 / 0.1f);
  }
  // Barely related words
  else {
    rank = static_cast<int>(800 + (0.35f - std::max(0.0f, combined_similarity)) * 199 / 0.35f);
  }

  // Apply special case handling for words with same root but different forms
  if (likely_shared_root && prefix_length >= 5) {
    // Cap rank for words that clearly share the same root
    // (like предвзятый/предвзятость)
    rank = std::min(rank, 150);
  }

  LOG_DEBUG() << "Word: " << guessed_word << ", Target: " << target_only_word << ", Cosine: " << cosine_sim
              << ", Prefix score: " << prefix_score << ", Morph bonus: " << morphological_bonus
              << ", Combined: " << combined_similarity << ", Shared root: " << (likely_shared_root ? "yes" : "no")
              << ", Final rank: " << rank;

  return std::clamp(rank, 2, kMaxRank);
}

std::vector<models::Word> WordDictionaryComponent::GetSimilarWords(std::string_view word,
                                                                   std::string_view target_word) const {
  if (!ValidateWord(word) || !ValidateWord(target_word)) {
    LOG_WARNING() << "Invalid words: " << word << " or " << target_word;
    return {};
  }

  std::vector<models::Word> similar_words;

  // Calculate similarity between the guess and target word
  const float similarity = dictionary_.CalculateSimilarity(word, target_word);
  LOG_DEBUG() << "Similarity between " << word << " and " << target_word << ": " << similarity;

  const auto rank_result = CalculateRank(word, target_word);
  if (!rank_result) {
    LOG_ERROR() << "Failed to get similar words for '" << word << "' and '" << target_word << "'";
    return similar_words;
  }

  const int rank = *rank_result;
  LOG_INFO() << "Final rank for " << word << ": " << rank << " (similarity: " << similarity << ")";

  models::Word model_word{.id = std::string(word), .similarity_score = similarity, .rank = rank};
  similar_words.push_back(std::move(model_word));

  return similar_words;
}

userver::yaml_config::Schema WordDictionaryComponent::GetStaticConfigSchema() {
  return userver::yaml_config::MergeSchemas<userver::components::LoggableComponentBase>(R"(
type: object
description: Word dictionary component
additionalProperties: false
properties:
  embeddings-path:
    type: string
    description: Path to embeddings
    defaultDescription: assets/ruwikiruscorpora-nobigrams_upos_skipgram_300_5_2018.vec
  embeddings-preferred-word-type:
    type: string
    description: Type of words to prioritize (noun, verb, adjective, etc. or 'any')
    defaultDescription: any
  dictionary-path:
    type: string
    description: Path to dedicated word dictionary file (one word per line)
  max-dictionary-words:
    type: integer
    description: Maximum number of words to load from the dictionary file
    defaultDescription: 0
  dictionary-preferred-word-type:
    type: string
    description: Type of words to prioritize (noun, verb, adjective, etc. or 'any')
    defaultDescription: any
)");
}

}  // namespace contexto
