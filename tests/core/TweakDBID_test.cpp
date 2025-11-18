#include <catch2/catch_test_macros.hpp>
#include "core/TweakDBID.hpp"

using namespace TweakXL;

TEST_CASE("TweakDBID basic construction", "[TweakDBID]") {
    SECTION("Default constructor creates invalid ID") {
        TweakDBID id;
        REQUIRE_FALSE(id.IsValid());
        REQUIRE(id.GetHash() == 0);
    }

    SECTION("Hash constructor creates valid ID") {
        TweakDBID id(0x123456789ABCDEF0ULL);
        REQUIRE(id.IsValid());
        REQUIRE(id.GetHash() == 0x123456789ABCDEF0ULL);
    }

    SECTION("String constructor creates valid ID") {
        TweakDBID id("Items.Preset_Katana_Default");
        REQUIRE(id.IsValid());
        REQUIRE(id.GetHash() != 0);
        REQUIRE(id.GetName().has_value());
        REQUIRE(*id.GetName() == "Items.Preset_Katana_Default");
    }
}

TEST_CASE("TweakDBID hashing", "[TweakDBID]") {
    SECTION("Empty string produces zero hash") {
        auto hash = TweakDBID::Hash("");
        REQUIRE(hash == 0);
    }

    SECTION("Same string produces same hash") {
        auto hash1 = TweakDBID::Hash("test");
        auto hash2 = TweakDBID::Hash("test");
        REQUIRE(hash1 == hash2);
        REQUIRE(hash1 != 0);
    }

    SECTION("Different strings produce different hashes") {
        auto hash1 = TweakDBID::Hash("test1");
        auto hash2 = TweakDBID::Hash("test2");
        REQUIRE(hash1 != hash2);
    }

    SECTION("Hashing is case-insensitive") {
        auto hash1 = TweakDBID::Hash("TEST");
        auto hash2 = TweakDBID::Hash("test");
        auto hash3 = TweakDBID::Hash("TeSt");
        REQUIRE(hash1 == hash2);
        REQUIRE(hash2 == hash3);
    }
}

TEST_CASE("TweakDBID comparison", "[TweakDBID]") {
    TweakDBID id1("test");
    TweakDBID id2("test");
    TweakDBID id3("other");

    SECTION("Equality comparison") {
        REQUIRE(id1 == id2);
        REQUIRE_FALSE(id1 == id3);
    }

    SECTION("Inequality comparison") {
        REQUIRE_FALSE(id1 != id2);
        REQUIRE(id1 != id3);
    }
}

TEST_CASE("TweakDBID string conversion", "[TweakDBID]") {
    SECTION("ID with name converts to string") {
        TweakDBID id("Items.Preset_Katana_Default");
        REQUIRE(id.ToString() == "Items.Preset_Katana_Default");
    }

    SECTION("ID without name converts to hex") {
        TweakDBID id(0x123456789ABCDEF0ULL);
        auto str = id.ToString();
        REQUIRE(str.find("0x") == 0);  // Starts with "0x"
    }
}

TEST_CASE("TweakDBID realistic examples", "[TweakDBID]") {
    SECTION("Item flat ID") {
        TweakDBID id("Items.Preset_Katana_Default:quality");
        REQUIRE(id.IsValid());
        REQUIRE(id.GetName().has_value());
    }

    SECTION("Prevention system ID") {
        TweakDBID id("PreventionSystem.setup.totalEntitiesLimit");
        REQUIRE(id.IsValid());
        REQUIRE(id.GetName().has_value());
    }
}
