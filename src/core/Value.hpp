#pragma once

#include "Types.hpp"
#include "TweakDBID.hpp"
#include <variant>
#include <vector>

namespace TweakXL {

/**
 * Value - Variant type that can hold any TweakDB value type
 *
 * Supports:
 * - Primitive types: bool, int32, float, string
 * - Special types: CName, TweakDBID, LocKey, Resource
 * - Arrays of the above types
 */
class Value {
public:
    // Value variant - holds actual data
    using Variant = std::variant<
        std::monostate,           // Empty/unset
        bool,                     // Bool
        int32,                    // Int32
        float,                    // Float
        String,                   // String
        CName,                    // CName
        TweakDBID,                // TweakDBID
        LocKey,                   // LocKey
        Resource,                 // Resource
        std::vector<Value>        // Array
    >;

    // Constructors
    Value() = default;

    // Primitive types
    Value(bool v) : data_(v), type_(TypeID::Bool) {}
    Value(int32 v) : data_(v), type_(TypeID::Int32) {}
    Value(float v) : data_(v), type_(TypeID::Float) {}
    Value(const String& v) : data_(v), type_(TypeID::String) {}
    Value(String&& v) : data_(std::move(v)), type_(TypeID::String) {}
    Value(const char* v) : data_(String(v)), type_(TypeID::String) {}

    // Special types
    Value(const CName& v) : data_(v), type_(TypeID::CName) {}
    Value(const TweakDBID& v) : data_(v), type_(TypeID::TweakDBID) {}
    Value(const LocKey& v) : data_(v), type_(TypeID::LocKey) {}
    Value(const Resource& v) : data_(v), type_(TypeID::Resource) {}

    // Array
    Value(const std::vector<Value>& v) : data_(v), type_(TypeID::Array) {}
    Value(std::vector<Value>&& v) : data_(std::move(v)), type_(TypeID::Array) {}

    // Type queries
    TypeID GetType() const { return type_; }
    bool IsEmpty() const { return std::holds_alternative<std::monostate>(data_); }
    bool IsBool() const { return std::holds_alternative<bool>(data_); }
    bool IsInt32() const { return std::holds_alternative<int32>(data_); }
    bool IsFloat() const { return std::holds_alternative<float>(data_); }
    bool IsString() const { return std::holds_alternative<String>(data_); }
    bool IsCName() const { return std::holds_alternative<CName>(data_); }
    bool IsTweakDBID() const { return std::holds_alternative<TweakDBID>(data_); }
    bool IsLocKey() const { return std::holds_alternative<LocKey>(data_); }
    bool IsResource() const { return std::holds_alternative<Resource>(data_); }
    bool IsArray() const { return std::holds_alternative<std::vector<Value>>(data_); }

    // Value getters (with type checking)
    bool AsBool() const;
    int32 AsInt32() const;
    float AsFloat() const;
    const String& AsString() const;
    const CName& AsCName() const;
    const TweakDBID& AsTweakDBID() const;
    const LocKey& AsLocKey() const;
    const Resource& AsResource() const;
    const std::vector<Value>& AsArray() const;

    // Value getters (unsafe - no type checking)
    template<typename T>
    const T& Get() const {
        return std::get<T>(data_);
    }

    template<typename T>
    T& Get() {
        return std::get<T>(data_);
    }

    // String conversion
    String ToString() const;

    // Comparison
    bool operator==(const Value& other) const {
        return data_ == other.data_;
    }
    bool operator!=(const Value& other) const {
        return data_ != other.data_;
    }

private:
    Variant data_;
    TypeID type_ = TypeID::Unknown;
};

} // namespace TweakXL
