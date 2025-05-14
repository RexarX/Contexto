#pragma once

#include <pch.hpp>

namespace contexto::models {

struct Word {
  std::string id;
  float similarity_score = 0.0f;
  int rank = -1;
};

}  // namespace contexto::models
