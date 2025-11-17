#pragma once

#include "Types.hpp"
#include "TweakDBID.hpp"
#include "Value.hpp"
#include <map>
#include <unordered_map>

namespace TweakXL {

/**
 * Record - Collection of properties in TweakDB
 *
 * A record is a structured object with named properties.
 * Example:
 *   Items.Preset_Katana_Default:
 *     quality: Quality.Legendary
 *     mass: 20.0
 *     entityName: my_katana
 */
struct Record {
    TweakDBID id;
    TypeID type;
    std::unordered_map<String, Value> properties;

    // Base record (for inheritance)
    std::optional<TweakDBID> base;

    Record() : type(TypeID::Unknown) {}

    explicit Record(const TweakDBID& i, TypeID t = TypeID::Unknown)
        : id(i), type(t) {}

    // Check if record has a property
    bool HasProperty(const String& name) const {
        return properties.find(name) != properties.end();
    }

    // Get property value
    const Value& GetProperty(const String& name) const {
        auto it = properties.find(name);
        if (it == properties.end()) {
            static Value empty;
            return empty;
        }
        return it->second;
    }

    // Set property value
    void SetProperty(const String& name, const Value& value) {
        properties[name] = value;
    }

    // Remove property
    void RemoveProperty(const String& name) {
        properties.erase(name);
    }

    bool IsValid() const {
        return id.IsValid();
    }
};

} // namespace TweakXL
