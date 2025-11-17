#include "TweakDBID.hpp"
#include <algorithm>
#include <cctype>

namespace TweakXL {

// FNV1A64 hash constants
constexpr uint64 FNV_OFFSET_BASIS = 0xcbf29ce484222325ULL;
constexpr uint64 FNV_PRIME = 0x100000001b3ULL;

TweakDBID::TweakDBID(const String& name)
    : hash_(Hash(name))
    , name_(name)
{
}

uint64 TweakDBID::Hash(const String& name) {
    if (name.empty()) {
        return 0;
    }

    // FNV1A64 hash algorithm
    uint64 hash = FNV_OFFSET_BASIS;

    for (char c : name) {
        // TweakDB appears to be case-insensitive
        char lower = std::tolower(static_cast<unsigned char>(c));
        hash ^= static_cast<uint64>(static_cast<unsigned char>(lower));
        hash *= FNV_PRIME;
    }

    return hash;
}

TweakDBID TweakDBID::FromString(const String& name) {
    return TweakDBID(name);
}

String TweakDBID::ToString() const {
    if (name_) {
        return *name_;
    }

    // If we don't have the original name, return the hash as hex
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "0x%016llx",
             static_cast<unsigned long long>(hash_));
    return String(buffer);
}

// CName implementation
CName::CName(const String& str)
    : hash(TweakDBID::Hash(str))
    , value(str)
{
}

} // namespace TweakXL
