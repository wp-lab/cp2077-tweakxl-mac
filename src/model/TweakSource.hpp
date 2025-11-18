#pragma once

#include "../core/Types.hpp"
#include "../core/TweakDBID.hpp"
#include "../core/Value.hpp"
#include <vector>
#include <unordered_map>
#include <optional>

namespace TweakXL {

/**
 * Operation types for array modifications
 */
enum class ArrayOperation {
    Assign,   // Replace entire array
    Append,   // Add values to end
    Remove    // Remove matching values
};

/**
 * Represents a modification to a flat (key-value pair)
 */
struct FlatModification {
    TweakDBID id;
    Value value;
    TypeID type;

    // For array operations
    ArrayOperation operation = ArrayOperation::Assign;

    bool IsArrayOperation() const {
        return operation != ArrayOperation::Assign;
    }
};

/**
 * Represents a modification to a record property
 */
struct RecordPropertyModification {
    String propertyName;
    Value value;
    TypeID type;
    ArrayOperation operation = ArrayOperation::Assign;
};

/**
 * Represents a record modification (create or update)
 */
struct RecordModification {
    TweakDBID id;
    TypeID type;
    std::optional<TweakDBID> baseRecord;  // Inheritance
    std::vector<RecordPropertyModification> properties;

    bool HasBase() const { return baseRecord.has_value(); }
};

/**
 * TweakSource - Represents all modifications from a single mod file
 *
 * Can be loaded from:
 * - YAML files (.yaml, .yml)
 * - Tweak files (.tweak)
 * - Other custom formats
 */
class TweakSource {
public:
    TweakSource() = default;
    explicit TweakSource(const String& filepath);

    // Add modifications
    void AddFlat(FlatModification flat);
    void AddRecord(RecordModification record);

    // Access modifications
    const std::vector<FlatModification>& GetFlats() const { return flats_; }
    const std::vector<RecordModification>& GetRecords() const { return records_; }

    // Metadata
    const String& GetFilePath() const { return filepath_; }
    void SetFilePath(const String& path) { filepath_ = path; }

    const String& GetModName() const { return modName_; }
    void SetModName(const String& name) { modName_ = name; }

    // Statistics
    size_t GetFlatCount() const { return flats_.size(); }
    size_t GetRecordCount() const { return records_.size(); }
    size_t GetTotalModificationCount() const;

    // Validation
    bool IsValid() const;
    std::vector<String> Validate() const;  // Returns error messages

    // Clear all modifications
    void Clear();

    // Merge another source into this one
    void Merge(const TweakSource& other);

private:
    String filepath_;
    String modName_;
    std::vector<FlatModification> flats_;
    std::vector<RecordModification> records_;
};

} // namespace TweakXL
