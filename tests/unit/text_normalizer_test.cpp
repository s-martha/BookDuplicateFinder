#include "bookmatch/text/text_normalizer.hpp"

#include <gtest/gtest.h>

TEST(TextNormalizer, NormalizesCaseWhitespaceAndPunctuation) {
    const auto result = bookmatch::text::normalize("  Hello,\tWORLD!\nПрИвЕт... ");
    EXPECT_EQ(result.value, "hello world привет");
    EXPECT_EQ(result.tokens, (std::vector<std::string>{"hello", "world", "привет"}));
}

TEST(TextNormalizer, ConfiguresYoFolding) {
    EXPECT_EQ(bookmatch::text::normalize("Ёжик").value, "ежик");
    EXPECT_EQ(bookmatch::text::normalize("Ёжик", {.fold_yo_to_ie = false}).value, "ёжик");
}

TEST(TextNormalizer, InvalidUtf8SeparatesTokens) {
    const std::string source{"one\xFFtwo"};
    EXPECT_EQ(bookmatch::text::normalize(source).tokens, (std::vector<std::string>{"one", "two"}));
}

TEST(TextNormalizer, EmptyTextProducesNoTokens) {
    const auto result = bookmatch::text::normalize("");
    EXPECT_TRUE(result.value.empty());
    EXPECT_TRUE(result.tokens.empty());
}
