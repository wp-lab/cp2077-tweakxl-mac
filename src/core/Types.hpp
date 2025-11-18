#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <optional>

namespace TweakXL {

// Basic integer types
using uint8 = std::uint8_t;
using uint16 = std::uint16_t;
using uint32 = std::uint32_t;
using uint64 = std::uint64_t;

using int8 = std::int8_t;
using int16 = std::int16_t;
using int32 = std::int32_t;
using int64 = std::int64_t;

// String types
using String = std::string;

// TweakDB type identifiers
// These will be expanded as we understand more about the binary format
enum class TypeID : uint32 {
    Unknown = 0,

    // Primitive types
    Bool = 1,
    Int32 = 2,
    Float = 3,
    String = 4,

    // Special types
    CName = 10,
    TweakDBID = 11,
    LocKey = 12,
    Resource = 13,

    // Container types
    Array = 20,

    // Complex types (to be defined)
    Quaternion = 30,
    EulerAngles = 31,
    Vector2 = 32,
    Vector3 = 33,
    Color = 34,
};

// CName - Hashed string identifier
struct CName {
    uint64 hash;
    std::optional<String> value; // Original string if known

    CName() : hash(0) {}
    explicit CName(uint64 h) : hash(h) {}
    explicit CName(const String& str);

    bool operator==(const CName& other) const { return hash == other.hash; }
    bool operator!=(const CName& other) const { return hash != other.hash; }
    bool operator<(const CName& other) const { return hash < other.hash; }
};

// LocKey - Localization key
struct LocKey {
    uint64 key;

    LocKey() : key(0) {}
    explicit LocKey(uint64 k) : key(k) {}

    bool operator==(const LocKey& other) const { return key == other.key; }
    bool operator!=(const LocKey& other) const { return key != other.key; }
    bool operator<(const LocKey& other) const { return key < other.key; }
};

// Resource - Resource path reference
struct Resource {
    String path;

    Resource() = default;
    explicit Resource(String p) : path(std::move(p)) {}

    bool operator==(const Resource& other) const { return path == other.path; }
    bool operator!=(const Resource& other) const { return path != other.path; }
    bool operator<(const Resource& other) const { return path < other.path; }
};

// Forward declarations
class TweakDBID;
class Value;
struct Flat;
struct Record;

} // namespace TweakXL

// Hash function for CName (for use in unordered containers)
namespace std {
    template<>
    struct hash<TweakXL::CName> {
        size_t operator()(const TweakXL::CName& cname) const {
            return std::hash<TweakXL::uint64>{}(cname.hash);
        }
    };
}
