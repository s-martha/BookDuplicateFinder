#include "bookmatch/core/file_scanner.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <fstream>

TEST(FileScanner, FindsTxtFilesRecursivelyAndIgnoresOtherFormats) {
    const auto root = std::filesystem::temp_directory_path() /
                      ("bookmatch-scan-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::filesystem::create_directories(root / "nested");
    { std::ofstream(root / "first.txt") << "first"; }
    { std::ofstream(root / "nested" / "second.TXT") << "second"; }
    { std::ofstream(root / "skip.epub") << "not text"; }
    const auto result = bookmatch::core::scan_text_files({root});
    EXPECT_TRUE(result.errors.empty());
    EXPECT_EQ(result.text_files.size(), 2U);
    std::filesystem::remove_all(root);
}
