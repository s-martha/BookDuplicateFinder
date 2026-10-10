#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace bookmatch::core {

struct ScanResult {
    std::vector<std::filesystem::path> text_files;
    std::vector<std::string> errors;
};

[[nodiscard]] ScanResult scan_text_files(const std::vector<std::filesystem::path>& roots);

}  // namespace bookmatch::core
