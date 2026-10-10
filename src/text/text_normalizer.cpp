#include "bookmatch/text/text_normalizer.hpp"

#include <cstdint>

namespace bookmatch::text {
namespace {

bool decode_utf8(std::string_view input, std::size_t& index, std::uint32_t& codepoint) {
    const auto byte = static_cast<unsigned char>(input[index]);
    if (byte < 0x80U) {
        codepoint = byte;
        ++index;
        return true;
    }

    std::size_t continuation_count = 0;
    std::uint32_t value = 0;
    std::uint32_t minimum = 0;
    if ((byte & 0xE0U) == 0xC0U) { continuation_count = 1; value = byte & 0x1FU; minimum = 0x80U; }
    else if ((byte & 0xF0U) == 0xE0U) { continuation_count = 2; value = byte & 0x0FU; minimum = 0x800U; }
    else if ((byte & 0xF8U) == 0xF0U) { continuation_count = 3; value = byte & 0x07U; minimum = 0x10000U; }
    else { ++index; return false; }
    if (index + continuation_count >= input.size()) { ++index; return false; }
    for (std::size_t offset = 1; offset <= continuation_count; ++offset) {
        const auto next = static_cast<unsigned char>(input[index + offset]);
        if ((next & 0xC0U) != 0x80U) { ++index; return false; }
        value = (value << 6U) | (next & 0x3FU);
    }
    if (value < minimum || value > 0x10FFFFU || (value >= 0xD800U && value <= 0xDFFFU)) { ++index; return false; }
    index += continuation_count + 1;
    codepoint = value;
    return true;
}

void append_utf8(std::string& output, std::uint32_t codepoint) {
    if (codepoint < 0x80U) output.push_back(static_cast<char>(codepoint));
    else if (codepoint < 0x800U) {
        output.push_back(static_cast<char>(0xC0U | (codepoint >> 6U)));
        output.push_back(static_cast<char>(0x80U | (codepoint & 0x3FU)));
    } else if (codepoint < 0x10000U) {
        output.push_back(static_cast<char>(0xE0U | (codepoint >> 12U)));
        output.push_back(static_cast<char>(0x80U | ((codepoint >> 6U) & 0x3FU)));
        output.push_back(static_cast<char>(0x80U | (codepoint & 0x3FU)));
    } else {
        output.push_back(static_cast<char>(0xF0U | (codepoint >> 18U)));
        output.push_back(static_cast<char>(0x80U | ((codepoint >> 12U) & 0x3FU)));
        output.push_back(static_cast<char>(0x80U | ((codepoint >> 6U) & 0x3FU)));
        output.push_back(static_cast<char>(0x80U | (codepoint & 0x3FU)));
    }
}

bool is_word_codepoint(std::uint32_t value) {
    return (value >= 'a' && value <= 'z') || (value >= 'A' && value <= 'Z') ||
           (value >= 0x0410U && value <= 0x044FU) || value == 0x0401U || value == 0x0451U;
}

std::uint32_t lowercase(std::uint32_t value, bool fold_yo_to_ie) {
    if (value >= 'A' && value <= 'Z') return value + 0x20U;
    if (value >= 0x0410U && value <= 0x042FU) return value + 0x20U;
    if (value == 0x0401U) value = 0x0451U;
    if (fold_yo_to_ie && value == 0x0451U) return 0x0435U;
    return value;
}

}  // namespace

NormalizedText normalize(std::string_view input, const NormalizationOptions& options) {
    NormalizedText result;
    std::string token;
    auto finish_token = [&] {
        if (!token.empty()) {
            result.tokens.push_back(token);
            if (!result.value.empty()) result.value.push_back(' ');
            result.value += token;
            token.clear();
        }
    };
    for (std::size_t index = 0; index < input.size();) {
        std::uint32_t codepoint{};
        if (!decode_utf8(input, index, codepoint)) { finish_token(); continue; }
        if (is_word_codepoint(codepoint)) append_utf8(token, lowercase(codepoint, options.fold_yo_to_ie));
        else finish_token();
    }
    finish_token();
    return result;
}

}  // namespace bookmatch::text
