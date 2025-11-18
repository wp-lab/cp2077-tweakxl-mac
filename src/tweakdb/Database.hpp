#pragma once

#include "../core/Types.hpp"
#include "../core/TweakDBID.hpp"
#include "../core/Value.hpp"
#include "../core/Flat.hpp"
#include "../core/Record.hpp"
#include <unordered_map>
#include <vector>
#include <optional>

namespace TweakXL {

/**
 * TweakDB - In-memory database for TweakDB entries
 *
 * Stores:
 * - Flats (individual key-value pairs)
 * - Records (typed collections of properties)
 *
 * Provides:
 * - Query methods
 * - Modification methods
 * - Validation
 */
class TweakDB {
public:
    TweakDB() = default;

    // Query methods
    bool HasFlat(const TweakDBID& id) const;
    bool HasRecord(const TweakDBID& id) const;

    const Flat* GetFlat(const TweakDBID& id) const;
    const Record* GetRecord(const TweakDBID& id) const;

    // Modification methods
    void SetFlat(const TweakDBID& id, const Value& value, TypeID type);
    void SetFlat(const Flat& flat);

    void SetRecord(const Record& record);
    void SetRecordProperty(const TweakDBID& recordId, const String& propertyName,
                           const Value& value, TypeID type);

    // Array operations
    void AppendToArray(const TweakDBID& id, const Value& value);
    void RemoveFromArray(const TweakDBID& id, const Value& value);

    // Bulk access
    const std::unordered_map<TweakDBID, Flat>& GetAllFlats() const { return flats_; }
    const std::unordered_map<TweakDBID, Record>& GetAllRecords() const { return records_; }

    // Statistics
    size_t GetFlatCount() const { return flats_.size(); }
    size_t GetRecordCount() const { return records_.size(); }

    // Clear
    void Clear();

    // Validation
    bool IsValid() const;
    std::vector<String> Validate() const;

private:
    std::unordered_map<TweakDBID, Flat> flats_;
    std::unordered_map<TweakDBID, Record> records_;

    // Helper methods
    Flat* GetFlatMutable(const TweakDBID& id);
    Record* GetRecordMutable(const TweakDBID& id);
};

} // namespace TweakXL
