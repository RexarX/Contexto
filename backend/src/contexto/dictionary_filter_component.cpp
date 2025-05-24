#include "dictionary_filter_component.hpp"

#include <userver/components/component_config.hpp>
#include <userver/logging/log.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

namespace contexto {

DictionaryFilterComponent::DictionaryFilterComponent(const userver::components::ComponentConfig& config,
                                                     const userver::components::ComponentContext& context)
    : LoggableComponentBase(config, context) {
  if (config.HasMember("blacklisted-words-path")) {
    const auto blacklist_path = config["blacklisted-words-path"].As<std::string>();
    LOG_INFO() << "Loading blacklisted words from: " << blacklist_path;

    if (!LoadBlacklistedWords(blacklist_path)) {
      LOG_WARNING() << "Failed to load blacklisted words from " << blacklist_path;
    } else {
      LOG_INFO() << "Loaded " << blacklisted_words_.size() << " blacklisted words";
    }
  } else {
    LOG_INFO() << "No blacklisted words path specified";
  }

  if (config.HasMember("min-word-length")) {
    min_word_length_ = config["min-word-length"].As<size_t>();
    LOG_INFO() << "Setting minimum word length to: " << min_word_length_;
  }

  // Load embedding preferred types
  if (config.HasMember("embedding-preferred-types")) {
    const auto& word_types = config["embedding-preferred-types"].As<std::vector<std::string>>();
    for (const auto& type_str : word_types) {
      const auto word_type = StringToWordType(type_str);
      if (word_type == models::WordType::kUnknown) {
        LOG_WARNING() << "Unknown word type: '" << type_str << "', ignoring";
        continue;
      }

      // If "any" is specified, clear other types and use only "any"
      if (word_type == models::WordType::kAny) {
        embedding_preferred_types_.clear();
        embedding_preferred_types_.push_back(models::WordType::kAny);
        LOG_INFO() << "Setting embedding preferred type to: any (accepting all types)";
        break;
      }

      if (std::ranges::find(embedding_preferred_types_, word_type) == embedding_preferred_types_.end()) {
        embedding_preferred_types_.push_back(word_type);
        LOG_INFO() << "Adding embedding preferred type: " << type_str;
      }
    }
  }

  // If no valid embedding types were specified, default to any
  if (embedding_preferred_types_.empty()) {
    embedding_preferred_types_.push_back(models::WordType::kAny);
    LOG_INFO() << "No valid embedding word types specified, defaulting to: any";
  }

  // Load dictionary preferred types
  if (config.HasMember("dictionary-preferred-types")) {
    const auto& word_types = config["dictionary-preferred-types"].As<std::vector<std::string>>();
    for (const auto& type_str : word_types) {
      const models::WordType word_type = StringToWordType(type_str);
      if (word_type == models::WordType::kUnknown) {
        LOG_WARNING() << "Unknown dictionary word type: '" << type_str << "', ignoring";
        continue;
      }

      // If "any" is specified, clear other types and use only "any"
      if (word_type == models::WordType::kAny) {
        dictionary_preferred_types_.clear();
        dictionary_preferred_types_.push_back(models::WordType::kAny);
        LOG_INFO() << "Setting dictionary preferred type to: any (accepting all types)";
        break;
      }

      if (std::ranges::find(dictionary_preferred_types_, word_type) == dictionary_preferred_types_.end()) {
        dictionary_preferred_types_.push_back(word_type);
        LOG_INFO() << "Adding dictionary preferred type: " << type_str;
      }
    }
  }

  // If no valid dictionary types were specified, default to any
  if (dictionary_preferred_types_.empty()) {
    dictionary_preferred_types_.push_back(models::WordType::kAny);
    LOG_INFO() << "No valid dictionary word types specified, defaulting to: any";
  }

  LOG_INFO() << "Dictionary filter initialized with " << embedding_preferred_types_.size()
             << " embedding preferred types, " << dictionary_preferred_types_.size()
             << " dictionary preferred types, min_length=" << min_word_length_
             << ", blacklist_size=" << blacklisted_words_.size();
}

bool DictionaryFilterComponent::LoadBlacklistedWords(std::string_view file_path) {
  std::ifstream file(file_path.data());
  if (!file.is_open()) {
    LOG_ERROR() << "Failed to open blacklisted words file: " << file_path;
    return false;
  }

  blacklisted_words_.clear();

  std::string line;
  size_t line_count = 0;

  while (std::getline(file, line)) {
    ++line_count;

    // Trim trailing whitespace
    const size_t end_pos = line.find_last_not_of(" \t\r\n");
    if (end_pos == std::string::npos) continue;
    line.resize(end_pos + 1);

    // Skip empty lines and comments
    if (line.empty() || line.front() == '#') continue;

    const size_t pos_separator = line.find_last_not_of('_');
    if (pos_separator != std::string::npos) {
      line.resize(pos_separator + 1);
    }

    line = utils::utf8::ToLower(line);
    blacklisted_words_.insert(std::move(line));
  }

  LOG_INFO() << "Processed " << line_count << " lines from blacklist file, "
             << "loaded " << blacklisted_words_.size() << " unique blacklisted words";
  return true;
}

bool DictionaryFilterComponent::FilterByWordType(std::string_view word,
                                                 const std::vector<models::WordType>& preferred_types) const {
  // Check if "any" type is allowed
  for (const auto& type : preferred_types) {
    if (type == models::WordType::kAny) return false;  // Don't filter if any type is allowed
  }

  if (!models::WordHasPOS(word)) {
    // Word without POS and we're filtering by type
    return true;  // Filter out
  }

  // Check word type against preferred types
  const size_t pos_separator = word.find_last_of('_');
  if (pos_separator != std::string::npos) {
    const std::string_view pos_tag_view(word.data() + pos_separator + 1, word.size() - pos_separator - 1);
    const models::WordType word_type = models::GetWordTypeFromPOS(pos_tag_view);

    // Check if this word type is in preferred types
    for (const auto& type : preferred_types) {
      if (type == word_type) return false;  // Don't filter if it matches a preferred type
    }
  }

  return true;  // Filter out if no match found
}

bool DictionaryFilterComponent::ShouldFilterOutEmbedding(std::string_view word) const {
  if (models::WordHasPOS(word)) {
    const std::string_view word_part = models::GetWordFromWordWithPOS(word);

    if (utils::utf8::CharCount(word_part) < min_word_length_) {
      return true;
    }

    if (IsBlacklisted(std::string(word_part))) return true;

    return FilterByWordType(word, embedding_preferred_types_);
  } else {
    // Word without POS
    if (utils::utf8::CharCount(word) < min_word_length_) {
      return true;
    }

    if (IsBlacklisted(std::string(word))) return true;

    // If we're strict about types and types are specified, filter words without POS
    const bool result = std::ranges::all_of(embedding_preferred_types_,
                                            [](models::WordType type) { return type != models::WordType::kAny; });
    return result;  // Filter out words without POS when we care about types
  }
}

bool DictionaryFilterComponent::ShouldFilterOutDictionary(std::string_view word) const {
  if (models::WordHasPOS(word)) {
    const std::string_view word_part = models::GetWordFromWordWithPOS(word);

    if (utils::utf8::CharCount(word_part) < min_word_length_) {
      return true;
    }

    if (IsBlacklisted(std::string(word_part))) return true;

    return FilterByWordType(word, dictionary_preferred_types_);
  } else {
    // Word without POS
    if (utils::utf8::CharCount(word) < min_word_length_) {
      return true;
    }

    if (IsBlacklisted(std::string(word))) return true;

    // If we're strict about types and types are specified, filter words without POS
    const bool result = std::ranges::all_of(embedding_preferred_types_,
                                            [](models::WordType type) { return type != models::WordType::kAny; });

    return result;  // Filter out words without POS when we care about types
  }
}

models::WordType DictionaryFilterComponent::StringToWordType(std::string_view str) noexcept {
  if (str == "noun") return models::WordType::kNoun;
  if (str == "verb") return models::WordType::kVerb;
  if (str == "adjective") return models::WordType::kAdjective;
  if (str == "adverb") return models::WordType::kAdverb;
  if (str == "adposition") return models::WordType::kAdposition;
  if (str == "auxiliary") return models::WordType::kAuxiliary;
  if (str == "coordinating_conjunction") return models::WordType::kCoordinatingConjunction;
  if (str == "determiner") return models::WordType::kDeterminer;
  if (str == "interjection") return models::WordType::kInterjection;
  if (str == "numeral") return models::WordType::kNumeral;
  if (str == "particle") return models::WordType::kParticle;
  if (str == "pronoun") return models::WordType::kPronoun;
  if (str == "proper_noun") return models::WordType::kProperNoun;
  if (str == "punctuation") return models::WordType::kPunctuation;
  if (str == "subordinating_conjunction") return models::WordType::kSubordinatingConjunction;
  if (str == "symbol") return models::WordType::kSymbol;
  if (str == "any") return models::WordType::kAny;
  return models::WordType::kUnknown;
}

userver::yaml_config::Schema DictionaryFilterComponent::GetStaticConfigSchema() {
  return userver::yaml_config::MergeSchemas<userver::components::LoggableComponentBase>(R"(
type: object
description: Dictionary filter component for managing word filtering
additionalProperties: false
properties:
  blacklisted-words-path:
    type: string
    description: Path to file containing blacklisted words, one per line
  embedding-preferred-types:
    type: array
    description: Types of words to include in embeddings (noun, verb, adjective, etc.)
    items:
      type: string
      description: Word type (noun, verb, adjective, etc.)
    defaultDescription: "any"
  dictionary-preferred-types:
    type: array
    description: Types of words to include in dictionary (noun, verb, adjective, etc.)
    items:
      type: string
      description: Word type (noun, verb, adjective, etc.)
    defaultDescription: "any"
  min-word-length:
    type: integer
    description: Minimum length of words to include in the dictionary
    defaultDescription: 2
)");
}

}  // namespace contexto
