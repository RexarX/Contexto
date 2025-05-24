#pragma once

#include <pch.hpp>

#include <Eigen/Dense>

namespace contexto::models {

enum class WordType {
  kUnknown = 0,
  kAdjective,
  kAdposition,
  kAdverb,
  kAuxiliary,
  kCoordinatingConjunction,
  kDeterminer,
  kInterjection,
  kNoun,
  kNumeral,
  kParticle,
  kPronoun,
  kProperNoun,
  kPunctuation,
  kSubordinatingConjunction,
  kSymbol,
  kVerb,
  kOther,
  kAny
};

constexpr std::array<std::string_view, 17> POS_TAGS = {"ADJ",   "ADP",   "ADV", "AUX",  "CCONJ", "DET",
                                                       "INTJ",  "NOUN",  "NUM", "PART", "PRON",  "PROPN",
                                                       "PUNCT", "SCONJ", "SYM", "VERB", "X"};

static constexpr bool WordHasPOS(std::string_view word) noexcept {
  if (word.size() < 3 || word.find_last_of('_') == std::string_view::npos) {
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
  if (pos_tag == "ADP") return WordType::kAdposition;
  if (pos_tag == "AUX") return WordType::kAuxiliary;
  if (pos_tag == "CCONJ") return WordType::kCoordinatingConjunction;
  if (pos_tag == "DET") return WordType::kDeterminer;
  if (pos_tag == "INTJ") return WordType::kInterjection;
  if (pos_tag == "NUM") return WordType::kNumeral;
  if (pos_tag == "PART") return WordType::kParticle;
  if (pos_tag == "PRON") return WordType::kPronoun;
  if (pos_tag == "PROPN") return WordType::kProperNoun;
  if (pos_tag == "PUNCT") return WordType::kPunctuation;
  if (pos_tag == "SCONJ") return WordType::kSubordinatingConjunction;
  if (pos_tag == "SYM") return WordType::kSymbol;
  if (pos_tag == "X") return WordType::kOther;
  return WordType::kUnknown;
}

static constexpr std::string_view GetWordFromWordWithPOS(std::string_view word_with_pos) noexcept {
  if (word_with_pos.size() < 4) return {};
  const size_t pos_separator = word_with_pos.find_last_of('_');
  if (pos_separator != std::string::npos) {
    return {word_with_pos.data(), pos_separator};
  }
  return {};
}

static inline std::string GetWordWithPOS(std::string word, WordType word_type) noexcept {
  std::string_view pos;
  switch (word_type) {
    case WordType::kAdjective: {
      pos = "_ADJ";
      break;
    }
    case WordType::kAdposition: {
      pos = "_ADP";
      break;
    }
    case WordType::kAdverb: {
      pos = "_ADV";
      break;
    }
    case WordType::kAuxiliary: {
      pos = "_AUX";
      break;
    }
    case WordType::kCoordinatingConjunction: {
      pos = "_CCONJ";
      break;
    }
    case WordType::kDeterminer: {
      pos = "_DET";
      break;
    }
    case WordType::kInterjection: {
      pos = "_INTJ";
      break;
    }
    case WordType::kNoun: {
      pos = "_NOUN";
      break;
    }
    case WordType::kNumeral: {
      pos = "_NUM";
      break;
    }
    case WordType::kParticle: {
      pos = "_PART";
      break;
    }
    case WordType::kPronoun: {
      pos = "_PRON";
      break;
    }
    case WordType::kProperNoun: {
      pos = "_PROPN";
      break;
    }
    case WordType::kPunctuation: {
      pos = "_PUNCT";
      break;
    }
    case WordType::kSubordinatingConjunction: {
      pos = "_SCONJ";
      break;
    }
    case WordType::kSymbol: {
      pos = "_SYM";
      break;
    }
    case WordType::kVerb: {
      pos = "_VERB";
      break;
    }
    case WordType::kOther: {
      pos = "_X";
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
  Eigen::VectorXf embedding;

  std::string_view GetWord() const noexcept { return GetWordFromWordWithPOS(word_with_pos); }

  WordType GetType() const noexcept {
    const size_t pos_separator = word_with_pos.find_last_of('_');
    if (pos_separator != std::string::npos) {
      const std::string_view pos_tag_view(word_with_pos.data() + pos_separator + 1,
                                          word_with_pos.size() - pos_separator - 1);
      return GetWordTypeFromPOS(pos_tag_view);
    }
    return WordType::kUnknown;
  }

  float CalculateSimilarity(const DictionaryWord& other) const { return embedding.dot(other.embedding); }
};

}  // namespace contexto::models
