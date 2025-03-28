#pragma once

#include "pch.hpp"

namespace contesto::models {

struct Word {
  std::string id;
  double similarity_score = 0.0;
  int64_t rank = 0;
};

}  // namespace contesto::models