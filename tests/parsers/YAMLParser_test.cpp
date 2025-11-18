#include <catch2/catch_test_macros.hpp>
#include "parsers/YAMLParser.hpp"

using namespace TweakXL;

TEST_CASE("YAMLParser - Simple flats", "[YAMLParser]") {
    SECTION("Parse simple scalar values") {
        String yaml = R"(
Items.Katana.damage: 100
Player.health: 500.5
Player.name: "Johnny"
Player.isAlive: true
)";

        YAMLTweakParser parser;
        TweakSource source = parser.ParseString(yaml);

        REQUIRE(source.GetFlatCount() == 4);
        REQUIRE(source.GetRecordCount() == 0);

        const auto& flats = source.GetFlats();

        // Check IDs are valid
        REQUIRE(flats[0].id.IsValid());
        REQUIRE(flats[1].id.IsValid());
        REQUIRE(flats[2].id.IsValid());
        REQUIRE(flats[3].id.IsValid());

        // Check types
        REQUIRE(flats[0].type == TypeID::Int32);
        REQUIRE(flats[1].type == TypeID::Float);
        REQUIRE(flats[2].type == TypeID::String);
        REQUIRE(flats[3].type == TypeID::Bool);
    }
}

TEST_CASE("YAMLParser - Arrays", "[YAMLParser]") {
    SECTION("Parse simple array") {
        String yaml = R"(
Player.inventory:
  - item1
  - item2
  - item3
)";

        YAMLTweakParser parser;
        TweakSource source = parser.ParseString(yaml);

        REQUIRE(source.GetFlatCount() == 1);

        const auto& flat = source.GetFlats()[0];
        REQUIRE(flat.id.IsValid());
        REQUIRE(flat.type == TypeID::Array);
        REQUIRE(flat.operation == ArrayOperation::Assign);
    }

    SECTION("Parse array append operation") {
        String yaml = R"(
Player.inventory:
  $append:
    - newItem1
    - newItem2
)";

        YAMLTweakParser parser;
        TweakSource source = parser.ParseString(yaml);

        REQUIRE(source.GetFlatCount() == 1);

        const auto& flat = source.GetFlats()[0];
        REQUIRE(flat.operation == ArrayOperation::Append);
    }

    SECTION("Parse array remove operation") {
        String yaml = R"(
Player.inventory:
  $remove:
    - unwantedItem
)";

        YAMLTweakParser parser;
        TweakSource source = parser.ParseString(yaml);

        REQUIRE(source.GetFlatCount() == 1);

        const auto& flat = source.GetFlats()[0];
        REQUIRE(flat.operation == ArrayOperation::Remove);
    }
}

TEST_CASE("YAMLParser - Records", "[YAMLParser]") {
    SECTION("Parse basic record") {
        String yaml = R"(
Items.MyKatana:
  $type: Weapon_Record
  damage: 150
  range: 2.5
)";

        YAMLTweakParser parser;
        TweakSource source = parser.ParseString(yaml);

        REQUIRE(source.GetFlatCount() == 0);
        REQUIRE(source.GetRecordCount() == 1);

        const auto& record = source.GetRecords()[0];
        REQUIRE(record.id.IsValid());
        REQUIRE(record.properties.size() == 2);
        REQUIRE_FALSE(record.HasBase());
    }

    SECTION("Parse record with inheritance") {
        String yaml = R"(
Items.MyKatana:
  $type: Weapon_Record
  $base: Items.Preset_Katana_Default
  damage: 200
)";

        YAMLTweakParser parser;
        TweakSource source = parser.ParseString(yaml);

        REQUIRE(source.GetRecordCount() == 1);

        const auto& record = source.GetRecords()[0];
        REQUIRE(record.HasBase());
        REQUIRE(record.baseRecord->IsValid());
        REQUIRE(record.properties.size() == 1);
    }

    SECTION("Parse record with array property") {
        String yaml = R"(
NPC.Character:
  name: "V"
  items:
    - sword
    - gun
)";

        YAMLTweakParser parser;
        TweakSource source = parser.ParseString(yaml);

        REQUIRE(source.GetRecordCount() == 1);

        const auto& record = source.GetRecords()[0];
        REQUIRE(record.properties.size() == 2);
    }
}

TEST_CASE("YAMLParser - Mixed content", "[YAMLParser]") {
    SECTION("Parse flats and records together") {
        String yaml = R"(
# Simple flats
Items.Katana.damage: 100
Player.health: 500

# A record
Items.MyWeapon:
  $type: Weapon_Record
  damage: 150
  range: 2.5

# Another flat
Player.level: 50
)";

        YAMLTweakParser parser;
        TweakSource source = parser.ParseString(yaml);

        REQUIRE(source.GetFlatCount() == 3);
        REQUIRE(source.GetRecordCount() == 1);
    }
}

TEST_CASE("YAMLParser - Validation", "[YAMLParser]") {
    SECTION("Valid source passes validation") {
        String yaml = R"(
Player.health: 100
Items.Weapon:
  damage: 50
)";

        YAMLTweakParser parser;
        TweakSource source = parser.ParseString(yaml);

        REQUIRE(source.IsValid());
        REQUIRE(source.Validate().empty());
    }
}

TEST_CASE("YAMLParser - Error handling", "[YAMLParser]") {
    SECTION("Invalid YAML syntax") {
        String yaml = R"(
This is: [not, valid: yaml
)";

        YAMLTweakParser parser;
        TweakSource source = parser.ParseString(yaml);

        REQUIRE_FALSE(parser.GetLastError().empty());
    }

    SECTION("Empty YAML") {
        String yaml = "";

        YAMLTweakParser parser;
        TweakSource source = parser.ParseString(yaml);

        // Empty is valid, just produces no modifications
        REQUIRE(source.GetFlatCount() == 0);
        REQUIRE(source.GetRecordCount() == 0);
    }
}

TEST_CASE("YAMLParser - TweakSource merge", "[YAMLParser]") {
    SECTION("Merge two sources") {
        String yaml1 = "Player.health: 100";
        String yaml2 = "Player.mana: 50";

        YAMLTweakParser parser;
        TweakSource source1 = parser.ParseString(yaml1);
        TweakSource source2 = parser.ParseString(yaml2);

        REQUIRE(source1.GetFlatCount() == 1);
        REQUIRE(source2.GetFlatCount() == 1);

        source1.Merge(source2);

        REQUIRE(source1.GetFlatCount() == 2);
    }
}

TEST_CASE("YAMLParser - Statistics", "[YAMLParser]") {
    SECTION("Get total modification count") {
        String yaml = R"(
Flat1: 100
Flat2: 200

Record1:
  prop1: value1
  prop2: value2
  prop3: value3
)";

        YAMLTweakParser parser;
        TweakSource source = parser.ParseString(yaml);

        REQUIRE(source.GetFlatCount() == 2);
        REQUIRE(source.GetRecordCount() == 1);
        REQUIRE(source.GetTotalModificationCount() == 5); // 2 flats + 3 record properties
    }
}
