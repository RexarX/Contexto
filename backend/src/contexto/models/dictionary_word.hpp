#pragma once

#include <pch.hpp>

#include <Eigen/Dense>

namespace contexto::models {

enum class WordType {
  kUnknown = 0,
  kNoun = 1,
  kVerb = 2,
  kAdjective = 3,
  kAdverb = 4,
  kPronoun = 5,
  kPreposition = 6,
  kConjunction = 7,
  kInterjection = 8,
  kParticle = 9,
  kNumeral = 10,
  kAny = 11
};

constexpr std::array<std::string_view, 12> POS_TAGS = {"NOUN", "VERB",  "ADJ",   "ADV",  "PRON", "ADP",
                                                       "CONJ", "CCONJ", "SCONJ", "INTJ", "PART", "NUM"};

static constexpr bool WordHasPOS(std::string_view word) noexcept {
  if (word.size() < 4 || word.rfind('_') == std::string_view::npos) {
    return false;
  }

  for (const auto tag : POS_TAGS) {
    if (word.ends_with(tag)) return true;
  }
  return false;
}

static constexpr WordType GetWordTypeFromPOS(std::string_view pos_tag) noexcept {
  if (pos_tag == "NOUN") return WordType::kNoun;
  if (pos_tag == "VERB") return WordType::kVerb;
  if (pos_tag == "ADJ") return WordType::kAdjective;
  if (pos_tag == "ADV") return WordType::kAdverb;
  if (pos_tag == "PRON") return WordType::kPronoun;
  if (pos_tag == "ADP") return WordType::kPreposition;
  if (pos_tag == "CONJ" || pos_tag == "CCONJ" || pos_tag == "SCONJ") return WordType::kConjunction;
  if (pos_tag == "INTJ") return WordType::kInterjection;
  if (pos_tag == "PART") return WordType::kParticle;
  if (pos_tag == "NUM") return WordType::kNumeral;
  return WordType::kUnknown;
}

static constexpr std::string_view GetWordFromWordWithPOS(std::string_view word_with_pos) noexcept {
  if (word_with_pos.size() < 4) return {};
  const size_t pos_separator = word_with_pos.rfind('_');
  if (pos_separator != std::string::npos) {
    return {word_with_pos.data(), pos_separator};
  }
  return {};
}

static inline std::string GetWordWithPOS(std::string word, WordType word_type) noexcept {
  std::string_view pos;
  switch (word_type) {
    case WordType::kNoun: {
      pos = "NOUN";
      break;
    }
    case WordType::kVerb: {
      pos = "VERB";
      break;
    }
    case WordType::kAdjective: {
      pos = "ADJ";
      break;
    }
    case WordType::kAdverb: {
      pos = "ADV";
      break;
    }
    case WordType::kPronoun: {
      pos = "PRON";
      break;
    }
    case WordType::kPreposition: {
      pos = "ADP";
      break;
    }
    case WordType::kConjunction: {
      pos = "CONJ";
      break;
    }
    case WordType::kInterjection: {
      pos = "INTJ";
      break;
    }
    case WordType::kParticle: {
      pos = "PART";
      break;
    }
    case WordType::kNumeral: {
      pos = "NUM";
      break;
    }
    default:
      return word;
  }

  word.insert(word.cend(), pos.begin(), pos.end());
  return word;
}

struct DictionaryWord {
  std::string word_with_pos;
  std::string_view word = word_with_pos;
  WordType type = WordType::kUnknown;
  Eigen::VectorXf embedding;

  float CalculateSimilarity(const DictionaryWord& other) const { return embedding.dot(other.embedding); }
};

}  // namespace contexto::models
