#include <catch2/catch_test_macros.hpp>
#include "core/Flat.hpp"

using namespace TweakXL;

TEST_CASE("Flat basic construction", "[Flat]") {
    SECTION("Default constructor") {
        Flat flat;
        REQUIRE_FALSE(flat.IsValid());
    }

    SECTION("Constructor with ID and value") {
        TweakDBID id("Items.Preset_Katana_Default:quality");
        Value val("Legendary");
        Flat flat(id, val, TypeID::String);

        REQUIRE(flat.IsValid());
        REQUIRE(flat.id == id);
        REQUIRE(flat.value == val);
        REQUIRE(flat.type == TypeID::String);
    }
}

TEST_CASE("Flat validation", "[Flat]") {
    SECTION("Valid flat") {
        Flat flat(TweakDBID("test"), Value(42), TypeID::Int32);
        REQUIRE(flat.IsValid());
    }

    SECTION("Invalid flat - no ID") {
        Flat flat(TweakDBID(), Value(42), TypeID::Int32);
        REQUIRE_FALSE(flat.IsValid());
    }

    SECTION("Invalid flat - empty value") {
        Flat flat(TweakDBID("test"), Value(), TypeID::Unknown);
        REQUIRE_FALSE(flat.IsValid());
    }
}
