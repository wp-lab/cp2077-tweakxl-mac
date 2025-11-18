#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "../../src/parsers/YAMLParser.hpp"
#include "../../src/bridge/Applicator.hpp"
#include "../../src/tweakdb/Database.hpp"
#include <sstream>

using namespace TweakXL;

TEST_CASE("Integration - Simple flat modification", "[Integration]") {
    // YAML with simple flat values
    String yaml = R"(
Player.BaseHealth: 100
Player.MaxSpeed: 5.5
Player.Name: "V"
)";

    // Parse YAML
    YAMLTweakParser parser;
    TweakSource source = parser.ParseString(yaml);

    REQUIRE(source.GetFlatCount() == 3);
    REQUIRE(source.GetRecordCount() == 0);

    // Apply to database
    TweakDB db;
    ModificationApplicator applicator;

    bool result = applicator.Apply(db, source);
    REQUIRE(result == true);
    REQUIRE(applicator.GetAppliedFlatCount() == 3);
    REQUIRE(applicator.GetAppliedRecordCount() == 0);
    REQUIRE(applicator.HasErrors() == false);

    // Verify flats were applied
    REQUIRE(db.HasFlat(TweakDBID("Player.BaseHealth")));
    REQUIRE(db.HasFlat(TweakDBID("Player.MaxSpeed")));
    REQUIRE(db.HasFlat(TweakDBID("Player.Name")));

    const Flat* health = db.GetFlat(TweakDBID("Player.BaseHealth"));
    REQUIRE(health != nullptr);
    REQUIRE(health->value.AsInt32() == 100);

    const Flat* speed = db.GetFlat(TweakDBID("Player.MaxSpeed"));
    REQUIRE(speed != nullptr);
    REQUIRE_THAT(speed->value.AsFloat(), Catch::Matchers::WithinAbs(5.5f, 0.001f));

    const Flat* name = db.GetFlat(TweakDBID("Player.Name"));
    REQUIRE(name != nullptr);
    REQUIRE(name->value.AsString() == "V");
}

TEST_CASE("Integration - Array append operation", "[Integration]") {
    String yaml = R"(
Player.StartingInventory:
  $append:
    - Items.FirstAidKit
    - Items.MaxDoc
    - Items.BounceBack
)";

    YAMLTweakParser parser;
    TweakSource source = parser.ParseString(yaml);

    REQUIRE(source.GetFlatCount() == 1);
    const auto& flats = source.GetFlats();
    REQUIRE(flats[0].operation == ArrayOperation::Append);

    // Create database with existing array
    TweakDB db;
    std::vector<Value> existingItems = {
        Value("Items.Money"),
        Value("Items.Pistol")
    };
    db.SetFlat(TweakDBID("Player.StartingInventory"),
               Value(existingItems),
               TypeID::Array);

    // Apply append operation
    ModificationApplicator applicator;
    bool result = applicator.Apply(db, source);

    REQUIRE(result == true);
    REQUIRE(applicator.HasErrors() == false);

    // Verify array was appended
    const Flat* inventory = db.GetFlat(TweakDBID("Player.StartingInventory"));
    REQUIRE(inventory != nullptr);
    REQUIRE(inventory->value.IsArray());

    auto items = inventory->value.AsArray();
    REQUIRE(items.size() == 5);  // 2 original + 3 appended
    REQUIRE(items[0].AsString() == "Items.Money");
    REQUIRE(items[1].AsString() == "Items.Pistol");
    REQUIRE(items[2].AsString() == "Items.FirstAidKit");
    REQUIRE(items[3].AsString() == "Items.MaxDoc");
    REQUIRE(items[4].AsString() == "Items.BounceBack");
}

TEST_CASE("Integration - Array remove operation", "[Integration]") {
    String yaml = R"(
Loot.CommonItems:
  $remove:
    - Items.Junk_Shard_Damaged
    - Items.Junk_ElectronicScrap
)";

    YAMLTweakParser parser;
    TweakSource source = parser.ParseString(yaml);

    // Create database with array containing items to remove
    TweakDB db;
    std::vector<Value> lootItems = {
        Value("Items.Junk_Shard_Damaged"),
        Value("Items.Money"),
        Value("Items.Junk_ElectronicScrap"),
        Value("Items.Weapon")
    };
    db.SetFlat(TweakDBID("Loot.CommonItems"),
               Value(lootItems),
               TypeID::Array);

    // Apply remove operation
    ModificationApplicator applicator;
    bool result = applicator.Apply(db, source);

    REQUIRE(result == true);

    // Verify items were removed
    const Flat* loot = db.GetFlat(TweakDBID("Loot.CommonItems"));
    REQUIRE(loot != nullptr);

    auto items = loot->value.AsArray();
    REQUIRE(items.size() == 2);  // 4 original - 2 removed
    REQUIRE(items[0].AsString() == "Items.Money");
    REQUIRE(items[1].AsString() == "Items.Weapon");
}

TEST_CASE("Integration - Array assign operation", "[Integration]") {
    String yaml = R"(
NPC.VendorInventory.Weapons:
  $assign:
    - Items.Preset_Overture
    - Items.Preset_Katana
    - Items.Preset_Mantis_Blades
)";

    YAMLTweakParser parser;
    TweakSource source = parser.ParseString(yaml);

    // Create database with existing array
    TweakDB db;
    std::vector<Value> oldWeapons = {
        Value("Items.OldWeapon1"),
        Value("Items.OldWeapon2")
    };
    db.SetFlat(TweakDBID("NPC.VendorInventory.Weapons"),
               Value(oldWeapons),
               TypeID::Array);

    // Apply assign operation
    ModificationApplicator applicator;
    bool result = applicator.Apply(db, source);

    REQUIRE(result == true);

    // Verify array was completely replaced
    const Flat* weapons = db.GetFlat(TweakDBID("NPC.VendorInventory.Weapons"));
    REQUIRE(weapons != nullptr);

    auto items = weapons->value.AsArray();
    REQUIRE(items.size() == 3);  // Completely replaced
    REQUIRE(items[0].AsString() == "Items.Preset_Overture");
    REQUIRE(items[1].AsString() == "Items.Preset_Katana");
    REQUIRE(items[2].AsString() == "Items.Preset_Mantis_Blades");
}

TEST_CASE("Integration - Record with inheritance", "[Integration]") {
    String yaml = R"(
Items.MyCustomKatana:
  $type: Weapon_Record
  $base: Items.Preset_Katana_Default
  damage: 250
  range: 3.0
  critChance: 0.45
)";

    YAMLTweakParser parser;
    TweakSource source = parser.ParseString(yaml);

    REQUIRE(source.GetRecordCount() == 1);
    const auto& records = source.GetRecords();
    REQUIRE(records[0].HasBase());
    REQUIRE(records[0].baseRecord.value().ToString() == "Items.Preset_Katana_Default");
    REQUIRE(records[0].properties.size() == 3);

    // Apply to database
    TweakDB db;

    // Create base record that our custom one inherits from
    Record baseRecord;
    baseRecord.id = TweakDBID("Items.Preset_Katana_Default");
    baseRecord.type = TypeID::Unknown;  // Would be Weapon_Record in real scenario
    db.SetRecord(baseRecord);

    ModificationApplicator applicator;
    bool result = applicator.Apply(db, source);

    REQUIRE(result == true);
    REQUIRE(applicator.GetAppliedRecordCount() == 1);

    // Verify record was created
    REQUIRE(db.HasRecord(TweakDBID("Items.MyCustomKatana")));

    const Record* katana = db.GetRecord(TweakDBID("Items.MyCustomKatana"));
    REQUIRE(katana != nullptr);
    REQUIRE(katana->id.ToString() == "Items.MyCustomKatana");
}

TEST_CASE("Integration - Multiple modifications", "[Integration]") {
    String yaml = R"(
# Mix of flats and records
Player.BaseHealth: 150
Player.BaseStamina: 100

Items.MyWeapon:
  $type: Weapon_Record
  damage: 100
  fireRate: 2.5

Player.Skills:
  $append:
    - Skills.Hacking_5
    - Skills.Combat_5
)";

    YAMLTweakParser parser;
    TweakSource source = parser.ParseString(yaml);

    REQUIRE(source.GetFlatCount() == 3);  // 2 simple flats + 1 array operation
    REQUIRE(source.GetRecordCount() == 1);

    // Apply to database
    TweakDB db;

    // Pre-populate with existing array
    std::vector<Value> existingSkills = {Value("Skills.Default")};
    db.SetFlat(TweakDBID("Player.Skills"), Value(existingSkills), TypeID::Array);

    ModificationApplicator applicator;
    bool result = applicator.Apply(db, source);

    REQUIRE(result == true);
    REQUIRE(applicator.GetAppliedFlatCount() == 3);
    REQUIRE(applicator.GetAppliedRecordCount() == 1);
    REQUIRE(applicator.HasErrors() == false);

    // Verify all modifications
    REQUIRE(db.HasFlat(TweakDBID("Player.BaseHealth")));
    REQUIRE(db.HasFlat(TweakDBID("Player.BaseStamina")));
    REQUIRE(db.HasFlat(TweakDBID("Player.Skills")));
    REQUIRE(db.HasRecord(TweakDBID("Items.MyWeapon")));

    // Verify array append worked
    const Flat* skills = db.GetFlat(TweakDBID("Player.Skills"));
    REQUIRE(skills != nullptr);
    auto skillList = skills->value.AsArray();
    REQUIRE(skillList.size() == 3);  // 1 existing + 2 appended
}

TEST_CASE("Integration - Error handling for invalid modifications", "[Integration]") {
    String yaml = R"(
# Try to append to non-existent array (will fail in applicator)
NonExistent.Array:
  $append:
    - Item1
)";

    YAMLTweakParser parser;
    TweakSource source = parser.ParseString(yaml);

    // Apply to empty database (no existing array to append to)
    TweakDB db;
    ModificationApplicator applicator;

    applicator.Apply(db, source);

    // Should complete but log warnings
    // (In current implementation, warnings are logged but not treated as errors)
    REQUIRE(applicator.GetAppliedFlatCount() == 1);
}

TEST_CASE("Integration - Load from example YAML file", "[Integration]") {
    // Test loading one of our example mod files
    YAMLTweakParser parser;

    // This tests the file I/O path
    TweakSource source = parser.Parse("/home/user/cp2077-tweakxl-mac/examples/mods/inventory_tweaks.yaml");

    REQUIRE(source.GetTotalModificationCount() > 0);
    REQUIRE(source.GetFlatCount() > 0);

    // Apply to database
    TweakDB db;

    // Pre-populate arrays that will be modified
    db.SetFlat(TweakDBID("Player.StartingInventory"),
               Value(std::vector<Value>{}), TypeID::Array);
    db.SetFlat(TweakDBID("Loot.CommonItems"),
               Value(std::vector<Value>{
                   Value("Items.Junk_Shard_Damaged"),
                   Value("Items.KeepThis"),
                   Value("Items.Junk_ElectronicScrap")
               }), TypeID::Array);
    db.SetFlat(TweakDBID("NPC.VendorInventory.Weapons"),
               Value(std::vector<Value>{}), TypeID::Array);
    db.SetFlat(TweakDBID("Player.AvailableSkills"),
               Value(std::vector<Value>{}), TypeID::Array);

    ModificationApplicator applicator;
    bool result = applicator.Apply(db, source);

    REQUIRE(result == true);
    REQUIRE(applicator.HasErrors() == false);

    // Verify some modifications were applied
    REQUIRE(db.HasFlat(TweakDBID("Player.StartingInventory")));

    const Flat* inventory = db.GetFlat(TweakDBID("Player.StartingInventory"));
    REQUIRE(inventory != nullptr);
    REQUIRE(inventory->value.IsArray());
    REQUIRE(inventory->value.AsArray().size() > 0);
}

TEST_CASE("Integration - Database validation", "[Integration]") {
    TweakDB db;

    // Add valid entries
    db.SetFlat(TweakDBID("Test.Value"), Value(42), TypeID::Int32);

    Record record;
    record.id = TweakDBID("Test.Record");
    record.type = TypeID::Unknown;
    db.SetRecord(record);

    // Database should be valid
    REQUIRE(db.IsValid() == true);

    // Create a record with circular inheritance
    Record badRecord;
    badRecord.id = TweakDBID("Bad.Record");
    badRecord.type = TypeID::Unknown;
    badRecord.base = TweakDBID("Bad.Record");  // Points to itself
    db.SetRecord(badRecord);

    // Now database should be invalid
    REQUIRE(db.IsValid() == false);

    auto errors = db.Validate();
    REQUIRE(errors.size() > 0);
    REQUIRE(errors[0].find("inherits from itself") != String::npos);
}
