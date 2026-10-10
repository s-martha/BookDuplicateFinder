#include "bookmatch/core/file_scanner.hpp"

#include <algorithm>
#include <cctype>

namespace bookmatch::core {
namespace {

bool is_text_file(const std::filesystem::path& path) {
    auto extension = path.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char character) {
        return static_cast<char>(std::tolower(character));
    });
    return extension == ".txt";
}

}  // namespace

ScanResult scan_text_files(const std::vector<std::filesystem::path>& roots) {
    ScanResult result;
    for (const auto& root : roots) {
        std::error_code error;
        if (!std::filesystem::is_directory(root, error)) {
            result.errors.push_back("not a readable directory: " + root.string());
            continue;
        }
        std::filesystem::recursive_directory_iterator iterator(
            root, std::filesystem::directory_options::skip_permission_denied, error);
        const std::filesystem::recursive_directory_iterator end;
        while (iterator != end) {
            if (error) {
                result.errors.push_back("cannot inspect path below " + root.string() + ": " + error.message());
                error.clear();
                iterator.increment(error);
                continue;
            }
            const auto& entry = *iterator;
            if (!entry.is_symlink(error) && entry.is_regular_file(error) && is_text_file(entry.path())) result.text_files.push_back(entry.path());
            error.clear();
            iterator.increment(error);
        }
    }
    std::sort(result.text_files.begin(), result.text_files.end());
    return result;
}

}  // namespace bookmatch::core
