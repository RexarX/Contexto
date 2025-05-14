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

int WordDictionaryComponent::CalculateRank(std::string_view guessed_word, std::string_view target_word) const {
  if (!models::WordHasPOS(target_word)) {
    LOG_ERROR() << "Failed to calculate rank: target_word '" << target_word << "' must have a POS";
    return -1;
  }

  if (guessed_word == models::GetWordFromWordWithPOS(target_word)) return 1;

  const auto calculate_rank = [](float similarity) constexpr noexcept -> int {
    // - Very high similarity (0.9-1.0): ranks 2-10
    // - High similarity (0.8-0.9): ranks 11-50
    // - Good similarity (0.7-0.8): ranks 51-150
    // - Medium similarity (0.6-0.7): ranks 151-300
    // - Moderate similarity (0.5-0.6): ranks 301-500
    // - Low similarity (0.4-0.5): ranks 501-700
    // - Very low similarity (0.0-0.4): ranks 701-1000
    int rank = 0;

    if (similarity == 1.0f) {
      return 1;
    } else if (similarity >= 0.9f) {
      // Very high similarity: scale between 2-10
      rank = static_cast<int>(2 + (1.0f - similarity) * 8 / 0.1f);
    } else if (similarity >= 0.8f) {
      // High similarity: scale between 11-50
      rank = static_cast<int>(11 + (0.9f - similarity) * 39 / 0.1f);
    } else if (similarity >= 0.7f) {
      // Good similarity: scale between 51-150
      rank = static_cast<int>(51 + (0.8f - similarity) * 99 / 0.1f);
    } else if (similarity >= 0.6f) {
      // Medium similarity: scale between 151-300
      rank = static_cast<int>(151 + (0.7f - similarity) * 149 / 0.1f);
    } else if (similarity >= 0.5f) {
      // Moderate similarity: scale between 301-500
      rank = static_cast<int>(301 + (0.6f - similarity) * 199 / 0.1f);
    } else if (similarity >= 0.4f) {
      // Low similarity: scale between 501-700
      rank = static_cast<int>(501 + (0.5f - similarity) * 199 / 0.1f);
    } else if (similarity > 0.0f) {
      // Very low similarity: scale between 701-999
      rank = static_cast<int>(701 + (0.4f - std::min(0.4f, similarity)) * 298 / 0.4f);
    } else {
      // Zero or negative similarity
      rank = kMaxRank;
    }

    // Ensure rank is within bounds
    return std::clamp(rank, 2, kMaxRank);
  };

  if (models::WordHasPOS(guessed_word)) {
    const float similarity = dictionary_.CalculateSimilarity(guessed_word, target_word);
    if (similarity == -1.0f) {
      LOG_ERROR() << "Failed to calculate similarity for '" << guessed_word << "' and '" << target_word << "'";
      return -1;
    }
    return calculate_rank(similarity);
  }

  int min_rank = std::numeric_limits<int>::max();
  const auto indices = dictionary_.GetIndicesToWordPOSVariations(guessed_word);
  for (const size_t index : indices) {
    const models::DictionaryWord& dictionary_word = dictionary_.GetWordWithEmbeddingByIndex(index);
    const float similarity = dictionary_.CalculateSimilarity(dictionary_word.word_with_pos, target_word);
    if (similarity == -1.0f) {
      LOG_ERROR() << "Failed to calculate similarity for '" << dictionary_word.word_with_pos << "' and '" << target_word
                  << "'";
      return -1;
    }

    const int rank = calculate_rank(similarity);
    min_rank = std::min(min_rank, rank);
  }

  return min_rank == std::numeric_limits<int>::max() ? -1 : min_rank;
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

  // Calculate rank using our improved method
  const int rank = CalculateRank(word, target_word);
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
