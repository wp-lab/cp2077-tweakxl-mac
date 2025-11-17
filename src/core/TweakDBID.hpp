#pragma once

#include "Types.hpp"
#include <string>
#include <functional>

namespace TweakXL {

/**
 * TweakDBID - 64-bit identifier for TweakDB entries
 *
 * TweakDB uses FNV1A64 hash of string identifiers.
 * Format: "Package.Group.Item:Property"
 * Example: "Items.Preset_Katana_Default:quality"
 */
class TweakDBID {
public:
    // Constructors
    TweakDBID() : hash_(0) {}
    explicit TweakDBID(uint64 hash) : hash_(hash) {}
    explicit TweakDBID(const String& name);

    // Get hash value
    uint64 GetHash() const { return hash_; }

    // Check if valid (non-zero)
    bool IsValid() const { return hash_ != 0; }

    // Get original name if known
    const std::optional<String>& GetName() const { return name_; }
    void SetName(const String& n) { name_ = n; }

    // Comparison operators
    bool operator==(const TweakDBID& other) const { return hash_ == other.hash_; }
    bool operator!=(const TweakDBID& other) const { return hash_ != other.hash_; }
    bool operator<(const TweakDBID& other) const { return hash_ < other.hash_; }

    // String conversion
    String ToString() const;

    // Static hash function (FNV1A64)
    static uint64 Hash(const String& name);

    // Parse TweakDBID from string
    static TweakDBID FromString(const String& name);

private:
    uint64 hash_;
    std::optional<String> name_; // Store original name if available
};

} // namespace TweakXL

// Hash function for TweakDBID (for use in unordered containers)
namespace std {
    template<>
    struct hash<TweakXL::TweakDBID> {
        size_t operator()(const TweakXL::TweakDBID& id) const {
            return std::hash<TweakXL::uint64>{}(id.GetHash());
        }
    };
}
