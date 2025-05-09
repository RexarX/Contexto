#pragma once

#include "pch.hpp"

namespace contexto::models {

struct Word {
  std::string id;
  double similarity_score = 0.0;
  int64_t rank = 0;
};

}  // namespace contexto::models