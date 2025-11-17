#include <catch2/catch_test_macros.hpp>
#include "core/Record.hpp"

using namespace TweakXL;

TEST_CASE("Record basic construction", "[Record]") {
    SECTION("Default constructor") {
        Record record;
        REQUIRE_FALSE(record.IsValid());
    }

    SECTION("Constructor with ID") {
        TweakDBID id("Items.Preset_Katana_Default");
        Record record(id);

        REQUIRE(record.IsValid());
        REQUIRE(record.id == id);
        REQUIRE(record.properties.empty());
    }
}

TEST_CASE("Record property management", "[Record]") {
    Record record(TweakDBID("Items.Preset_Katana_Default"));

    SECTION("Add property") {
        record.SetProperty("quality", Value("Legendary"));
        REQUIRE(record.HasProperty("quality"));
        REQUIRE(record.GetProperty("quality").AsString() == "Legendary");
    }

    SECTION("Get non-existent property") {
        REQUIRE_FALSE(record.HasProperty("nonexistent"));
        REQUIRE(record.GetProperty("nonexistent").IsEmpty());
    }

    SECTION("Remove property") {
        record.SetProperty("mass", Value(20.0f));
        REQUIRE(record.HasProperty("mass"));

        record.RemoveProperty("mass");
        REQUIRE_FALSE(record.HasProperty("mass"));
    }

    SECTION("Multiple properties") {
        record.SetProperty("quality", Value("Legendary"));
        record.SetProperty("mass", Value(20.0f));
        record.SetProperty("damage", Value(100));

        REQUIRE(record.properties.size() == 3);
        REQUIRE(record.HasProperty("quality"));
        REQUIRE(record.HasProperty("mass"));
        REQUIRE(record.HasProperty("damage"));
    }
}

TEST_CASE("Record inheritance", "[Record]") {
    SECTION("Record with base") {
        Record record(TweakDBID("Items.MyKatana"));
        record.base = TweakDBID("Items.Preset_Katana_Default");

        REQUIRE(record.base.has_value());
        REQUIRE(*record.base == TweakDBID("Items.Preset_Katana_Default"));
    }
}
