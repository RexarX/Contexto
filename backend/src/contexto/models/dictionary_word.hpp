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
};

// Maps POS tag strings to WordType enum
inline WordType GetWordTypeFromPOS(std::string_view pos_tag) {
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

static constexpr int kEmbeddingDimension = 300;

struct DictionaryWord {
  std::string word;             // Original word form
  std::string normalized_word;  // Lowercase version for lookups
  WordType type = WordType::kUnknown;
  Eigen::Matrix<float, 1, kEmbeddingDimension> embedding;

  // Efficiently calculate similarity with another word
  double CalculateSimilarity(const DictionaryWord& other) const { return embedding.dot(other.embedding); }
};

}  // namespace contexto::models
