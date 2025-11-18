#include "YAMLParser.hpp"
#include <spdlog/spdlog.h>
#include <fstream>
#include <algorithm>

namespace TweakXL {

TweakSource YAMLTweakParser::Parse(const std::filesystem::path& filepath) {
    TweakSource source(filepath.string());

    try {
        YAML::Node root = YAML::LoadFile(filepath.string());

        if (!root || !root.IsMap()) {
            SetError("Invalid YAML: root must be a map");
            return source;
        }

        ParseNode(root, source);

        spdlog::info("Parsed YAML file: {} ({} flats, {} records)",
                     filepath.filename().string(),
                     source.GetFlatCount(),
                     source.GetRecordCount());

    } catch (const YAML::Exception& e) {
        SetError(String("YAML parsing error: ") + e.what());
        spdlog::error("Failed to parse {}: {}", filepath.string(), e.what());
    }

    return source;
}

TweakSource YAMLTweakParser::ParseString(const String& yamlContent) {
    TweakSource source("<string>");

    try {
        YAML::Node root = YAML::Load(yamlContent);

        if (!root || !root.IsMap()) {
            SetError("Invalid YAML: root must be a map");
            return source;
        }

        ParseNode(root, source);

    } catch (const YAML::Exception& e) {
        SetError(String("YAML parsing error: ") + e.what());
    }

    return source;
}

void YAMLTweakParser::ParseNode(const YAML::Node& node, TweakSource& source) {
    for (auto it = node.begin(); it != node.end(); ++it) {
        String key = it->first.as<String>();
        const YAML::Node& value = it->second;

        // Skip special keys at root level
        if (IsSpecialKey(key)) {
            continue;
        }

        // Determine if this is a record or a flat
        if (IsRecord(value)) {
            ParseRecord(key, value, source);
        } else {
            ParseFlat(key, value, source);
        }
    }
}

void YAMLTweakParser::ParseFlat(const String& key, const YAML::Node& value, TweakSource& source) {
    FlatModification flat;
    flat.id = TweakDBID(key);

    // Check for array operations
    if (value.IsMap() && IsArrayOperation(value)) {
        flat.operation = ParseArrayOperation(value);

        // Get the operation node
        if (value["$append"]) {
            flat.value = ParseValue(value["$append"]);
        } else if (value["$remove"]) {
            flat.value = ParseValue(value["$remove"]);
        } else {
            flat.value = ParseValue(value["$assign"] ? value["$assign"] : value);
        }
    } else {
        flat.value = ParseValue(value);
        flat.operation = ArrayOperation::Assign;
    }

    flat.type = DetectType(value);

    source.AddFlat(std::move(flat));
}

void YAMLTweakParser::ParseRecord(const String& key, const YAML::Node& value, TweakSource& source) {
    RecordModification record;
    record.id = TweakDBID(key);

    // Get record type
    auto recordType = GetRecordType(value);
    if (recordType) {
        record.type = *recordType;
    } else {
        record.type = TypeID::Unknown;
    }

    // Get base record
    record.baseRecord = GetRecordBase(value);

    // Parse properties
    for (auto it = value.begin(); it != value.end(); ++it) {
        String propKey = it->first.as<String>();

        // Skip special keys
        if (IsSpecialKey(propKey)) {
            continue;
        }

        RecordPropertyModification prop;
        prop.propertyName = propKey;

        const YAML::Node& propValue = it->second;

        // Check for array operations
        if (propValue.IsMap() && IsArrayOperation(propValue)) {
            prop.operation = ParseArrayOperation(propValue);

            if (propValue["$append"]) {
                prop.value = ParseValue(propValue["$append"]);
            } else if (propValue["$remove"]) {
                prop.value = ParseValue(propValue["$remove"]);
            } else {
                prop.value = ParseValue(propValue["$assign"] ? propValue["$assign"] : propValue);
            }
        } else {
            prop.value = ParseValue(propValue);
            prop.operation = ArrayOperation::Assign;
        }

        prop.type = DetectType(propValue);

        record.properties.push_back(std::move(prop));
    }

    source.AddRecord(std::move(record));
}

Value YAMLTweakParser::ParseValue(const YAML::Node& node) {
    if (!node.IsDefined() || node.IsNull()) {
        return Value();
    }

    // Scalar values
    if (node.IsScalar()) {
        String str = node.as<String>();

        // Try to detect special types
        return ParseSpecialType(node);
    }

    // Arrays
    if (node.IsSequence()) {
        return ParseArray(node);
    }

    // Maps (inline records or special types)
    if (node.IsMap()) {
        // For now, treat as unknown
        // TODO: Handle inline records
        return Value();
    }

    return Value();
}

Value YAMLTweakParser::ParseArray(const YAML::Node& node) {
    std::vector<Value> elements;

    for (size_t i = 0; i < node.size(); ++i) {
        elements.push_back(ParseValue(node[i]));
    }

    return Value(std::move(elements));
}

Value YAMLTweakParser::ParseSpecialType(const YAML::Node& node) {
    String str = node.as<String>();

    // Try bool
    if (str == "true" || str == "false") {
        return Value(str == "true");
    }

    // Try int
    try {
        size_t pos;
        int32 intVal = std::stoi(str, &pos);
        if (pos == str.length()) {
            return Value(intVal);
        }
    } catch (...) {}

    // Try float
    try {
        size_t pos;
        float floatVal = std::stof(str, &pos);
        if (pos == str.length()) {
            return Value(floatVal);
        }
    } catch (...) {}

    // Default to string
    return Value(str);
}

TypeID YAMLTweakParser::DetectType(const YAML::Node& node) {
    if (!node.IsDefined() || node.IsNull()) {
        return TypeID::Unknown;
    }

    if (node.IsScalar()) {
        String str = node.as<String>();

        // Bool
        if (str == "true" || str == "false") {
            return TypeID::Bool;
        }

        // Try int
        try {
            size_t pos;
            std::stoi(str, &pos);
            if (pos == str.length()) {
                return TypeID::Int32;
            }
        } catch (...) {}

        // Try float
        try {
            size_t pos;
            std::stof(str, &pos);
            if (pos == str.length()) {
                return TypeID::Float;
            }
        } catch (...) {}

        // Default to string
        return TypeID::String;
    }

    if (node.IsSequence()) {
        return TypeID::Array;
    }

    return TypeID::Unknown;
}

TypeID YAMLTweakParser::ParseTypeString(const String& typeStr) {
    // Simple mapping - expand as needed
    if (typeStr.find("Int") != String::npos) return TypeID::Int32;
    if (typeStr.find("Float") != String::npos) return TypeID::Float;
    if (typeStr.find("Bool") != String::npos) return TypeID::Bool;
    if (typeStr.find("String") != String::npos) return TypeID::String;
    if (typeStr.find("CName") != String::npos) return TypeID::CName;
    if (typeStr.find("TweakDBID") != String::npos) return TypeID::TweakDBID;
    if (typeStr.find("LocKey") != String::npos) return TypeID::LocKey;
    if (typeStr.find("Resource") != String::npos) return TypeID::Resource;

    return TypeID::Unknown;
}

ArrayOperation YAMLTweakParser::ParseArrayOperation(const YAML::Node& node) {
    if (node["$append"]) {
        return ArrayOperation::Append;
    }
    if (node["$remove"]) {
        return ArrayOperation::Remove;
    }
    return ArrayOperation::Assign;
}

bool YAMLTweakParser::IsArrayOperation(const YAML::Node& node) {
    if (!node.IsMap()) {
        return false;
    }

    return node["$append"] || node["$remove"] || node["$assign"];
}

bool YAMLTweakParser::IsRecord(const YAML::Node& node) {
    if (!node.IsMap()) {
        return false;
    }

    // A node is a record if it has $type or has multiple properties
    return node["$type"] || node.size() > 1;
}

std::optional<TypeID> YAMLTweakParser::GetRecordType(const YAML::Node& node) {
    if (!node["$type"]) {
        return std::nullopt;
    }

    String typeStr = node["$type"].as<String>();
    TypeID type = ParseTypeString(typeStr);

    if (type == TypeID::Unknown) {
        return std::nullopt;
    }

    return type;
}

std::optional<TweakDBID> YAMLTweakParser::GetRecordBase(const YAML::Node& node) {
    if (!node["$base"]) {
        return std::nullopt;
    }

    String baseName = node["$base"].as<String>();
    return TweakDBID(baseName);
}

bool YAMLTweakParser::IsSpecialKey(const String& key) const {
    return key[0] == '$';
}

String YAMLTweakParser::NormalizeKey(const String& key) const {
    // Convert to lowercase for case-insensitive matching
    String normalized = key;
    std::transform(normalized.begin(), normalized.end(), normalized.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return normalized;
}

void YAMLTweakParser::SetError(const String& message) {
    lastError_ = message;
}

} // namespace TweakXL
