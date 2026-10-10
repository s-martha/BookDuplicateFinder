#pragma once

#include "bookmatch/text/text_normalizer.hpp"

#include <filesystem>
#include <string>

namespace bookmatch::core {

struct Document {
    std::filesystem::path path;
    std::string file_hash;
    std::string normalized_hash;
    text::NormalizedText normalized;
};

struct DocumentResult {
    Document document;
    std::string error;

    [[nodiscard]] bool ok() const { return error.empty(); }
};

[[nodiscard]] DocumentResult load_document(const std::filesystem::path& path,
                                           const text::NormalizationOptions& options = {});

}  // namespace bookmatch::core
