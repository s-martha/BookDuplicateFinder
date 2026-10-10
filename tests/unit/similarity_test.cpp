#include "bookmatch/similarity/similarity.hpp"

#include <gtest/gtest.h>

TEST(Similarity, MakesExpectedShingles) {
    const std::vector<std::string> tokens{"one", "two", "three", "four"};
    const auto shingles = bookmatch::similarity::make_shingles(tokens, 3);
    EXPECT_EQ(shingles.size(), 2U);
    EXPECT_TRUE(shingles.contains("one" "\x1f" "two" "\x1f" "three"));
    EXPECT_TRUE(shingles.contains("two" "\x1f" "three" "\x1f" "four"));
}

TEST(Similarity, CalculatesJaccard) {
    const std::vector<std::string> left{"a", "b", "c", "d"};
    const std::vector<std::string> right{"a", "b", "c", "x"};
    const auto metrics = bookmatch::similarity::calculate_metrics(left, right, 2);
    EXPECT_DOUBLE_EQ(metrics.jaccard, 0.5);
    EXPECT_DOUBLE_EQ(metrics.length_ratio, 1.0);
}

TEST(Similarity, ClassifiesExactAndApproximateMatches) {
    const std::vector<std::string> tokens{"a", "b", "c", "d", "e"};
    EXPECT_EQ(bookmatch::similarity::compare("file", "file", "left", "right", tokens, {}, 0.8).kind,
              bookmatch::similarity::MatchKind::IdenticalFile);
    EXPECT_EQ(bookmatch::similarity::compare("left", "right", "text", "text", tokens, {}, 0.8).kind,
              bookmatch::similarity::MatchKind::IdenticalNormalizedText);
    EXPECT_EQ(bookmatch::similarity::compare("left", "right", "l", "r", tokens, tokens, 0.8).kind,
              bookmatch::similarity::MatchKind::LikelyDuplicate);
}

TEST(Similarity, EmptyDistinctDocumentsAreNotApproximateMatches) {
    const std::vector<std::string> empty_tokens;
    const auto comparison = bookmatch::similarity::compare("file-a", "file-b", "text-a", "text-b",
                                                            empty_tokens, empty_tokens, 0.8);
    EXPECT_EQ(comparison.kind, bookmatch::similarity::MatchKind::NoMatch);
    EXPECT_DOUBLE_EQ(comparison.metrics.jaccard, 0.0);
}
