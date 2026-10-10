#include "bookmatch/core/document.hpp"

#include "bookmatch/similarity/similarity.hpp"

#include <fstream>
#include <iterator>

namespace bookmatch::core {

DocumentResult load_document(const std::filesystem::path& path, const text::NormalizationOptions& options) {
    try {
        std::ifstream input(path, std::ios::binary);
        if (!input) return {{}, "cannot read file: " + path.string()};
        const std::string source((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
        if (!input.eof() && input.fail()) return {{}, "error while reading file: " + path.string()};
        auto normalized = text::normalize(source, options);
        Document document{path, similarity::sha256_file(path.string()), similarity::sha256_text(normalized.value), std::move(normalized)};
        return {std::move(document), {}};
    } catch (const std::exception& error) {
        return {{}, error.what()};
    }
}

}  // namespace bookmatch::core
