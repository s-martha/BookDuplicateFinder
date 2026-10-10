#include "bookmatch/similarity/similarity.hpp"

#include <algorithm>
#include <array>
#include <fstream>
#include <iomanip>
#include <memory>
#include <openssl/evp.h>
#include <sstream>
#include <stdexcept>

namespace bookmatch::similarity {
namespace {

std::string digest_sha256(const unsigned char* data, std::size_t size) {
    EVP_MD_CTX* raw_context = EVP_MD_CTX_new();
    if (raw_context == nullptr) throw std::runtime_error("cannot create SHA-256 context");
    struct ContextDeleter { void operator()(EVP_MD_CTX* context) const { EVP_MD_CTX_free(context); } };
    const std::unique_ptr<EVP_MD_CTX, ContextDeleter> context(raw_context);
    if (EVP_DigestInit_ex(context.get(), EVP_sha256(), nullptr) != 1 ||
        EVP_DigestUpdate(context.get(), data, size) != 1) {
        throw std::runtime_error("cannot calculate SHA-256");
    }
    std::array<unsigned char, EVP_MAX_MD_SIZE> digest{};
    unsigned int digest_size = 0;
    if (EVP_DigestFinal_ex(context.get(), digest.data(), &digest_size) != 1) {
        throw std::runtime_error("cannot finalize SHA-256");
    }
    std::ostringstream output;
    for (unsigned int index = 0; index < digest_size; ++index) {
        output << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(digest[index]);
    }
    return output.str();
}

double ratio(std::size_t left, std::size_t right) {
    if (left == 0 && right == 0) return 1.0;
    if (left == 0 || right == 0) return 0.0;
    return static_cast<double>(std::min(left, right)) / static_cast<double>(std::max(left, right));
}

}  // namespace

std::string sha256_text(std::string_view text) {
    return digest_sha256(reinterpret_cast<const unsigned char*>(text.data()), text.size());
}

std::string sha256_file(const std::string& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) throw std::runtime_error("cannot read file: " + path);
    EVP_MD_CTX* raw_context = EVP_MD_CTX_new();
    if (raw_context == nullptr) throw std::runtime_error("cannot create SHA-256 context");
    struct ContextDeleter { void operator()(EVP_MD_CTX* context) const { EVP_MD_CTX_free(context); } };
    const std::unique_ptr<EVP_MD_CTX, ContextDeleter> context(raw_context);
    if (EVP_DigestInit_ex(context.get(), EVP_sha256(), nullptr) != 1) throw std::runtime_error("cannot initialize SHA-256");
    std::array<char, 65536> buffer{};
    while (input.read(buffer.data(), static_cast<std::streamsize>(buffer.size())) || input.gcount() > 0) {
        if (EVP_DigestUpdate(context.get(), buffer.data(), static_cast<std::size_t>(input.gcount())) != 1) {
            throw std::runtime_error("cannot calculate SHA-256");
        }
    }
    if (!input.eof()) throw std::runtime_error("error while reading file: " + path);
    std::array<unsigned char, EVP_MAX_MD_SIZE> digest{};
    unsigned int digest_size = 0;
    if (EVP_DigestFinal_ex(context.get(), digest.data(), &digest_size) != 1) throw std::runtime_error("cannot finalize SHA-256");
    std::ostringstream output;
    for (unsigned int index = 0; index < digest_size; ++index) output << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(digest[index]);
    return output.str();
}

std::unordered_set<std::string> make_shingles(const std::vector<std::string>& tokens, std::size_t shingle_size) {
    std::unordered_set<std::string> shingles;
    if (shingle_size == 0 || tokens.size() < shingle_size) return shingles;
    for (std::size_t start = 0; start + shingle_size <= tokens.size(); ++start) {
        std::string shingle = tokens[start];
        for (std::size_t offset = 1; offset < shingle_size; ++offset) shingle += "\x1f" + tokens[start + offset];
        shingles.insert(std::move(shingle));
    }
    return shingles;
}

Metrics calculate_metrics(const std::vector<std::string>& left_tokens,
                          const std::vector<std::string>& right_tokens,
                          std::size_t shingle_size) {
    const auto left = make_shingles(left_tokens, shingle_size);
    const auto right = make_shingles(right_tokens, shingle_size);
    std::size_t intersection = 0;
    for (const auto& shingle : left) if (right.contains(shingle)) ++intersection;
    const auto union_size = left.size() + right.size() - intersection;
    return {union_size == 0 ? 0.0 : static_cast<double>(intersection) / static_cast<double>(union_size),
            ratio(left_tokens.size(), right_tokens.size()), left.size(), right.size()};
}

Comparison compare(std::string_view left_file_hash, std::string_view right_file_hash,
                   std::string_view left_normalized_hash, std::string_view right_normalized_hash,
                   const std::vector<std::string>& left_tokens, const std::vector<std::string>& right_tokens,
                   double threshold, std::size_t shingle_size) {
    const auto metrics = calculate_metrics(left_tokens, right_tokens, shingle_size);
    if (left_file_hash == right_file_hash) return {MatchKind::IdenticalFile, metrics};
    if (left_normalized_hash == right_normalized_hash) return {MatchKind::IdenticalNormalizedText, metrics};
    if (metrics.jaccard >= threshold) return {MatchKind::LikelyDuplicate, metrics};
    if (metrics.jaccard > 0.0) return {MatchKind::PartialMatch, metrics};
    return {MatchKind::NoMatch, metrics};
}

const char* to_string(MatchKind kind) {
    switch (kind) {
        case MatchKind::IdenticalFile: return "identical-file";
        case MatchKind::IdenticalNormalizedText: return "identical-normalized-text";
        case MatchKind::LikelyDuplicate: return "likely-duplicate";
        case MatchKind::PartialMatch: return "partial-match";
        case MatchKind::NoMatch: return "no-match";
    }
    return "unknown";
}

}  // namespace bookmatch::similarity
