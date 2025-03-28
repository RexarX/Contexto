#pragma once

#include "models/word.hpp"

#include <userver/engine/mutex.hpp>
#include <userver/storages/postgres/cluster.hpp>

namespace contesto {

class WordSimilarityService {
public:
  explicit WordSimilarityService(const userver::components::ComponentConfig&,
                                 const userver::components::ComponentContext&);

  std::string GenerateNewTargetWord();
  std::vector<models::Word> GetSimilarWords(std::string_view word, std::string_view target_word);

  inline bool ValidateWord(std::string_view word) {
    return std::find(dictionary_.begin(), dictionary_.end(), word) != dictionary_.end();
  }

private:
  double CalculateSimilarity(std::string_view word1, std::string_view word2);
  void LoadWordEmbeddings();
  void LoadDictionary();

private:
  userver::storages::postgres::ClusterPtr pg_cluster_;
  userver::engine::Mutex mutex_;

  std::unordered_map<std::string_view, std::vector<float>> word_embeddings_;
  std::vector<std::string> dictionary_;
};

}  // namespace contesto