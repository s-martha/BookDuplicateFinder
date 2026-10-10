#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace bookmatch::similarity {

enum class MatchKind {
    IdenticalFile,
    IdenticalNormalizedText,
    LikelyDuplicate,
    PartialMatch,
    NoMatch,
};

struct Metrics {
    double jaccard{0.0};
    double length_ratio{0.0};
    std::size_t left_shingle_count{0};
    std::size_t right_shingle_count{0};
};

struct Comparison {
    MatchKind kind{MatchKind::NoMatch};
    Metrics metrics;
};

[[nodiscard]] std::string sha256_file(const std::string& path);
[[nodiscard]] std::string sha256_text(std::string_view text);
[[nodiscard]] std::unordered_set<std::string> make_shingles(
    const std::vector<std::string>& tokens, std::size_t shingle_size = 5);
[[nodiscard]] Metrics calculate_metrics(const std::vector<std::string>& left_tokens,
                                        const std::vector<std::string>& right_tokens,
                                        std::size_t shingle_size = 5);
[[nodiscard]] Comparison compare(std::string_view left_file_hash,
                                 std::string_view right_file_hash,
                                 std::string_view left_normalized_hash,
                                 std::string_view right_normalized_hash,
                                 const std::vector<std::string>& left_tokens,
                                 const std::vector<std::string>& right_tokens,
                                 double threshold,
                                 std::size_t shingle_size = 5);
[[nodiscard]] const char* to_string(MatchKind kind);

}  // namespace bookmatch::similarity
