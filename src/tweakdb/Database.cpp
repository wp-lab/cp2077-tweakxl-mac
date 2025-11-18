#include "Database.hpp"
#include <spdlog/spdlog.h>
#include <algorithm>

namespace TweakXL {

// Query methods
bool TweakDB::HasFlat(const TweakDBID& id) const {
    return flats_.find(id) != flats_.end();
}

bool TweakDB::HasRecord(const TweakDBID& id) const {
    return records_.find(id) != records_.end();
}

const Flat* TweakDB::GetFlat(const TweakDBID& id) const {
    auto it = flats_.find(id);
    return (it != flats_.end()) ? &it->second : nullptr;
}

const Record* TweakDB::GetRecord(const TweakDBID& id) const {
    auto it = records_.find(id);
    return (it != records_.end()) ? &it->second : nullptr;
}

// Modification methods
void TweakDB::SetFlat(const TweakDBID& id, const Value& value, TypeID type) {
    Flat flat;
    flat.id = id;
    flat.value = value;
    flat.type = type;
    flats_[id] = std::move(flat);
}

void TweakDB::SetFlat(const Flat& flat) {
    if (!flat.id.IsValid()) {
        spdlog::warn("Attempting to set flat with invalid ID");
        return;
    }
    flats_[flat.id] = flat;
}

void TweakDB::SetRecord(const Record& record) {
    if (!record.id.IsValid()) {
        spdlog::warn("Attempting to set record with invalid ID");
        return;
    }
    records_[record.id] = record;
}

void TweakDB::SetRecordProperty(const TweakDBID& recordId, const String& propertyName,
                                 const Value& value, TypeID type) {
    auto* record = GetRecordMutable(recordId);
    if (!record) {
        spdlog::warn("Record {} not found, creating new record", recordId.ToString());
        Record newRecord;
        newRecord.id = recordId;
        newRecord.type = TypeID::Unknown;
        records_[recordId] = newRecord;
        record = &records_[recordId];
    }

    record->SetProperty(propertyName, value);
}

// Array operations
void TweakDB::AppendToArray(const TweakDBID& id, const Value& value) {
    auto* flat = GetFlatMutable(id);
    if (!flat) {
        spdlog::warn("Flat {} not found for append operation", id.ToString());
        return;
    }

    if (!flat->value.IsArray()) {
        spdlog::error("Cannot append to non-array flat {}", id.ToString());
        return;
    }

    // Get current array
    auto currentArray = flat->value.AsArray();
    currentArray.push_back(value);

    // Update flat with new array
    flat->value = Value(std::move(currentArray));
}

void TweakDB::RemoveFromArray(const TweakDBID& id, const Value& value) {
    auto* flat = GetFlatMutable(id);
    if (!flat) {
        spdlog::warn("Flat {} not found for remove operation", id.ToString());
        return;
    }

    if (!flat->value.IsArray()) {
        spdlog::error("Cannot remove from non-array flat {}", id.ToString());
        return;
    }

    // Get current array
    auto currentArray = flat->value.AsArray();

    // Remove matching values
    auto it = std::remove_if(currentArray.begin(), currentArray.end(),
                             [&value](const Value& v) {
                                 return v.ToString() == value.ToString();
                             });
    currentArray.erase(it, currentArray.end());

    // Update flat with modified array
    flat->value = Value(std::move(currentArray));
}

// Clear
void TweakDB::Clear() {
    flats_.clear();
    records_.clear();
}

// Validation
bool TweakDB::IsValid() const {
    return Validate().empty();
}

std::vector<String> TweakDB::Validate() const {
    std::vector<String> errors;

    // Validate flats
    for (const auto& [id, flat] : flats_) {
        if (!flat.IsValid()) {
            errors.push_back("Invalid flat: " + id.ToString());
        }
    }

    // Validate records
    for (const auto& [id, record] : records_) {
        if (!record.id.IsValid()) {
            errors.push_back("Invalid record ID: " + id.ToString());
        }

        // Check for circular inheritance
        if (record.base.has_value()) {
            TweakDBID baseId = record.base.value();
            if (baseId == id) {
                errors.push_back("Record " + id.ToString() + " inherits from itself");
            }
        }
    }

    return errors;
}

// Helper methods
Flat* TweakDB::GetFlatMutable(const TweakDBID& id) {
    auto it = flats_.find(id);
    return (it != flats_.end()) ? &it->second : nullptr;
}

Record* TweakDB::GetRecordMutable(const TweakDBID& id) {
    auto it = records_.find(id);
    return (it != records_.end()) ? &it->second : nullptr;
}

} // namespace TweakXL
