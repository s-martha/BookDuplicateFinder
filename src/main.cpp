#include "bookmatch/core/document.hpp"
#include "bookmatch/core/file_scanner.hpp"
#include "bookmatch/similarity/similarity.hpp"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace {

constexpr double kDefaultThreshold = 0.80;

void print_usage(std::ostream& output) {
    output << "Usage:\n"
           << "  bookmatch index <dir> [<dir> ...] [--threshold VALUE]\n"
           << "  bookmatch compare <file1> <file2> [--threshold VALUE]\n";
}

bool parse_threshold(const std::string& value, double& threshold) {
    try {
        std::size_t parsed = 0;
        threshold = std::stod(value, &parsed);
        return parsed == value.size() && std::isfinite(threshold) && threshold >= 0.0 && threshold <= 1.0;
    } catch (const std::exception&) {
        return false;
    }
}

void print_comparison(const bookmatch::core::Document& left, const bookmatch::core::Document& right,
                      const bookmatch::similarity::Comparison& comparison) {
    std::cout << bookmatch::similarity::to_string(comparison.kind)
              << "  jaccard=" << std::fixed << std::setprecision(3) << comparison.metrics.jaccard
              << "  length-ratio=" << comparison.metrics.length_ratio
              << "\n  " << left.path.string() << "\n  " << right.path.string() << '\n';
}

int compare_files(const std::string& left_path, const std::string& right_path, double threshold) {
    const auto left = bookmatch::core::load_document(left_path);
    const auto right = bookmatch::core::load_document(right_path);
    if (!left.ok()) std::cerr << "error: " << left.error << '\n';
    if (!right.ok()) std::cerr << "error: " << right.error << '\n';
    if (!left.ok() || !right.ok()) return 2;
    const auto comparison = bookmatch::similarity::compare(
        left.document.file_hash, right.document.file_hash, left.document.normalized_hash, right.document.normalized_hash,
        left.document.normalized.tokens, right.document.normalized.tokens, threshold);
    std::cout << "tokens: " << left.document.normalized.tokens.size() << " / " << right.document.normalized.tokens.size() << '\n';
    print_comparison(left.document, right.document, comparison);
    return 0;
}

int index_directories(const std::vector<std::string>& roots, double threshold) {
    std::vector<std::filesystem::path> paths;
    for (const auto& root : roots) paths.emplace_back(root);
    const auto scan = bookmatch::core::scan_text_files(paths);
    for (const auto& error : scan.errors) std::cerr << "warning: " << error << '\n';
    std::vector<bookmatch::core::Document> documents;
    std::size_t errors = 0;
    for (const auto& path : scan.text_files) {
        auto loaded = bookmatch::core::load_document(path);
        if (!loaded.ok()) { ++errors; std::cerr << "warning: " << loaded.error << '\n'; }
        else documents.push_back(std::move(loaded.document));
    }
    std::size_t matches = 0;
    for (std::size_t left = 0; left < documents.size(); ++left) {
        for (std::size_t right = left + 1; right < documents.size(); ++right) {
            const auto comparison = bookmatch::similarity::compare(
                documents[left].file_hash, documents[right].file_hash,
                documents[left].normalized_hash, documents[right].normalized_hash,
                documents[left].normalized.tokens, documents[right].normalized.tokens, threshold);
            if (comparison.kind != bookmatch::similarity::MatchKind::NoMatch) {
                ++matches;
                print_comparison(documents[left], documents[right], comparison);
            }
        }
    }
    std::cout << "summary: found=" << scan.text_files.size() << " processed=" << documents.size()
              << " errors=" << errors << " matching-pairs=" << matches << '\n';
    return scan.errors.empty() && errors == 0 ? 0 : 2;
}

}  // namespace

int main(int argc, char* argv[]) {
    if (argc < 2) { print_usage(std::cerr); return 1; }
    const std::string command = argv[1];
    std::vector<std::string> positional;
    double threshold = kDefaultThreshold;
    for (int index = 2; index < argc; ++index) {
        const std::string argument = argv[index];
        if (argument == "--threshold") {
            if (++index >= argc || !parse_threshold(argv[index], threshold)) {
                std::cerr << "error: --threshold must be a number between 0 and 1\n";
                return 1;
            }
        } else if (argument.rfind("--", 0) == 0) {
            std::cerr << "error: unknown option " << argument << '\n';
            return 1;
        } else positional.push_back(argument);
    }
    if (command == "compare" && positional.size() == 2) return compare_files(positional[0], positional[1], threshold);
    if (command == "index" && !positional.empty()) return index_directories(positional, threshold);
    print_usage(std::cerr);
    return 1;
}
