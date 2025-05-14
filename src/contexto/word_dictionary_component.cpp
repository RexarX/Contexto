#include "word_dictionary_component.hpp"

#include <userver/components/component_config.hpp>
#include <userver/logging/log.hpp>
#include <userver/utils/assert.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

namespace contexto {

WordDictionaryComponent::WordDictionaryComponent(const userver::components::ComponentConfig& config,
                                                 const userver::components::ComponentContext& context)
    : LoggableComponentBase(config, context) {
  max_dictionary_words_ =
      config.HasMember("max-dictionary-words") ? config["max-dictionary-words"].As<size_t>() : 100000;

  std::string embeddings_path;
  if (config.HasMember("model-path")) {
    embeddings_path = config["model-path"].As<std::string>();
  }

  // Parse preferred word type
  std::string word_type_str = "noun";
  if (config.HasMember("preferred-word-type")) {
    word_type_str = config["preferred-word-type"].As<std::string>();
  }

  // Map string to enum
  if (word_type_str == "noun") {
    preferred_word_type_ = models::WordType::kNoun;
  } else if (word_type_str == "verb") {
    preferred_word_type_ = models::WordType::kVerb;
  } else if (word_type_str == "adjective") {
    preferred_word_type_ = models::WordType::kAdjective;
  } else if (word_type_str == "adverb") {
    preferred_word_type_ = models::WordType::kAdverb;
  } else if (word_type_str == "pronoun") {
    preferred_word_type_ = models::WordType::kPronoun;
  } else if (word_type_str == "any") {
    preferred_word_type_ = models::WordType::kAny;
  } else {
    LOG_WARNING() << "Unknown word type in config: " << word_type_str << ", defaulting to any";
    preferred_word_type_ = models::WordType::kAny;
  }

  LOG_INFO() << "Initializing word dictionary with max words: " << max_dictionary_words_
             << ", preferred word type: " << word_type_str;

  bool loaded = false;
  if (preferred_word_type_ != models::WordType::kUnknown) {
    loaded = dictionary_.LoadFromVectorFileWithFilter(embeddings_path, preferred_word_type_, max_dictionary_words_);
  } else {
    loaded = dictionary_.LoadFromVectorFile(embeddings_path, max_dictionary_words_);
  }

  if (!loaded || dictionary_.Size() == 0) {
    LOG_ERROR() << "Failed to load word embeddings dictionary";
    throw std::runtime_error("Failed to initialize word dictionary");
  }

  LOG_INFO() << "Successfully loaded dictionary with " << dictionary_.Size() << " words";
}

std::string_view WordDictionaryComponent::GenerateNewTargetWord() const {
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

  LOG_ERROR() << "Dictionary is empty, cannot generate target word";
  return {};
}

std::vector<models::Word> WordDictionaryComponent::GetSimilarWords(std::string_view word,
                                                                   std::string_view target_word) const {
  if (!ValidateWord(word) || !ValidateWord(target_word)) {
    LOG_WARNING() << "Invalid words: " << word << " or " << target_word;
    return {};
  }

  std::vector<models::Word> similar_words;

  const float similarity = dictionary_.CalculateSimilarity(word, target_word);
  LOG_DEBUG() << "Similarity between " << word << " and " << target_word << ": " << similarity;

  // Get rank based on semantic similarity
  int rank = 0;
  if (word == target_word) {
    // Exact match
    rank = 1;
  } else {
    // Find similar words to the target to determine relative ranking
    const auto similar_to_target = dictionary_.GetMostSimilarWords(target_word, kMaxRank);

    // Find the position of the guessed word in the similarity ranking
    const auto it = std::find_if(similar_to_target.begin(), similar_to_target.end(),
                                 [word](const auto pair) { return pair.first->word == word; });

    if (it != similar_to_target.end()) {
      // Word is in the top 100 similar words
      rank = std::distance(similar_to_target.begin(), it) + 2;  // +2 because rank 1 is exact match
    } else {
      // Word is not in top similar words, calculate based on similarity score
      if (similarity < 0.01f) {
        rank = kMaxRank;  // Very dissimilar
      } else {
        // Map similarity from [0.0f; 1.0f] to rank [kMaxRank; 2]
        // Higher similarity = lower rank (closer to target)
        rank =
            static_cast<int>(std::max(2.0f, std::round(static_cast<float>(kMaxRank) - (similarity * (kMaxRank - 2)))));
      }
    }
  }

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
  model-path:
    type: string
    description: Path to model
    defaultDescription: assets/ruwikiruscorpora-nobigrams_upos_skipgram_300_5_2018.vec
  max-dictionary-words:
    type: integer
    description: Maximum number of words to load from the embeddings file
    defaultDescription: 50000
  preferred-word-type:
    type: string
    description: Type of words to prioritize (noun, verb, adjective, etc. or 'any')
    defaultDescription: any
)");
}

}  // namespace contexto
