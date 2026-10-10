#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace bookmatch::text {

struct NormalizationOptions {
    bool fold_yo_to_ie{true};
};

struct NormalizedText {
    std::string value;
    std::vector<std::string> tokens;
};

[[nodiscard]] NormalizedText normalize(std::string_view input,
                                        const NormalizationOptions& options = {});

}  // namespace bookmatch::text
