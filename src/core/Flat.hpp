#pragma once

#include "Types.hpp"
#include "TweakDBID.hpp"
#include "Value.hpp"

namespace TweakXL {

/**
 * Flat - Individual data value in TweakDB
 *
 * A flat is a single piece of data identified by a TweakDBID.
 * Example: Items.Preset_Katana_Default:quality = Quality.Legendary
 */
struct Flat {
    TweakDBID id;
    Value value;
    TypeID type;

    Flat() : type(TypeID::Unknown) {}

    Flat(const TweakDBID& i, const Value& v, TypeID t = TypeID::Unknown)
        : id(i), value(v), type(t) {}

    bool IsValid() const {
        return id.IsValid() && !value.IsEmpty();
    }
};

} // namespace TweakXL
