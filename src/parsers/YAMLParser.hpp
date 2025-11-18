#pragma once

#include "../model/TweakSource.hpp"
#include <yaml-cpp/yaml.h>
#include <filesystem>

namespace TweakXL {

/**
 * YAMLTweakParser - Parses YAML mod files into TweakSource
 *
 * Supported YAML format:
 *
 * Flats (simple key-value):
 *   Items.Katana.damage: 100
 *   Player.health: 500.5
 *
 * Arrays:
 *   Player.inventory:
 *     - item1
 *     - item2
 *
 * Array operations:
 *   Player.inventory:
 *     $append:
 *       - item3
 *     $remove:
 *       - item1
 *
 * Records:
 *   Items.MyKatana:
 *     $type: Weapon_Record
 *     $base: Items.Preset_Katana_Default
 *     damage: 150
 *     range: 2.5
 */
class YAMLTweakParser {
public:
    YAMLTweakParser() = default;

    // Parse a YAML file
    TweakSource Parse(const std::filesystem::path& filepath);

    // Parse YAML from string (for testing)
    TweakSource ParseString(const String& yamlContent);

    // Get last error message
    const String& GetLastError() const { return lastError_; }

private:
    String lastError_;

    // Parsing methods
    void ParseNode(const YAML::Node& node, TweakSource& source);
    void ParseFlat(const String& key, const YAML::Node& value, TweakSource& source);
    void ParseRecord(const String& key, const YAML::Node& value, TweakSource& source);

    // Value parsing
    Value ParseValue(const YAML::Node& node);
    Value ParseArray(const YAML::Node& node);
    Value ParseSpecialType(const YAML::Node& node);

    // Type detection
    TypeID DetectType(const YAML::Node& node);
    TypeID ParseTypeString(const String& typeStr);

    // Array operations
    ArrayOperation ParseArrayOperation(const YAML::Node& node);
    bool IsArrayOperation(const YAML::Node& node);

    // Record operations
    bool IsRecord(const YAML::Node& node);
    std::optional<TypeID> GetRecordType(const YAML::Node& node);
    std::optional<TweakDBID> GetRecordBase(const YAML::Node& node);

    // Helpers
    bool IsSpecialKey(const String& key) const;
    String NormalizeKey(const String& key) const;
    void SetError(const String& message);
};

} // namespace TweakXL
