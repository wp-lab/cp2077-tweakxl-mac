#include "Applicator.hpp"
#include <spdlog/spdlog.h>

namespace TweakXL {

bool ModificationApplicator::Apply(TweakDB& db, const TweakSource& source) {
    ClearErrors();
    ResetStatistics();

    spdlog::info("Applying modifications from: {}", source.GetFilePath());

    // Apply all flats
    for (const auto& flat : source.GetFlats()) {
        try {
            ApplyFlat(db, flat);
            appliedFlats_++;
        } catch (const std::exception& e) {
            AddError("Failed to apply flat " + flat.id.ToString() + ": " + e.what());
        }
    }

    // Apply all records
    for (const auto& record : source.GetRecords()) {
        try {
            ApplyRecord(db, record);
            appliedRecords_++;
        } catch (const std::exception& e) {
            AddError("Failed to apply record " + record.id.ToString() + ": " + e.what());
        }
    }

    spdlog::info("Applied {} flats and {} records", appliedFlats_, appliedRecords_);

    if (HasErrors()) {
        spdlog::warn("Encountered {} errors during application", errors_.size());
        for (const auto& error : errors_) {
            spdlog::error("  - {}", error);
        }
        return false;
    }

    return true;
}

void ModificationApplicator::ApplyFlat(TweakDB& db, const FlatModification& flat) {
    if (!flat.id.IsValid()) {
        AddError("Invalid flat ID");
        return;
    }

    // Handle array operations
    if (flat.IsArrayOperation()) {
        ApplyArrayOperation(db, flat);
        return;
    }

    // Normal flat assignment
    db.SetFlat(flat.id, flat.value, flat.type);
}

void ModificationApplicator::ApplyRecord(TweakDB& db, const RecordModification& record) {
    if (!record.id.IsValid()) {
        AddError("Invalid record ID");
        return;
    }

    // Create or get existing record
    Record dbRecord;
    const Record* existing = db.GetRecord(record.id);
    if (existing) {
        dbRecord = *existing;
    } else {
        dbRecord.id = record.id;
        dbRecord.type = record.type;
    }

    // Apply inheritance if specified
    if (record.HasBase()) {
        dbRecord.base = record.baseRecord;
        ApplyRecordInheritance(db, record);
    }

    // Apply properties
    ApplyRecordProperties(db, record.id, record.properties);

    // Save record
    db.SetRecord(dbRecord);
}

void ModificationApplicator::ApplyArrayOperation(TweakDB& db, const FlatModification& flat) {
    switch (flat.operation) {
        case ArrayOperation::Append:
            // For append, we need to append each element
            if (flat.value.IsArray()) {
                for (const auto& element : flat.value.AsArray()) {
                    db.AppendToArray(flat.id, element);
                }
            } else {
                db.AppendToArray(flat.id, flat.value);
            }
            break;

        case ArrayOperation::Remove:
            // For remove, we remove each element
            if (flat.value.IsArray()) {
                for (const auto& element : flat.value.AsArray()) {
                    db.RemoveFromArray(flat.id, element);
                }
            } else {
                db.RemoveFromArray(flat.id, flat.value);
            }
            break;

        case ArrayOperation::Assign:
        default:
            // Normal assignment
            db.SetFlat(flat.id, flat.value, flat.type);
            break;
    }
}

void ModificationApplicator::ApplyRecordInheritance(TweakDB& db,
                                                     const RecordModification& record) {
    if (!record.HasBase()) {
        return;
    }

    const Record* baseRecord = db.GetRecord(record.baseRecord.value());
    if (!baseRecord) {
        spdlog::warn("Base record {} not found for record {}",
                     record.baseRecord->ToString(),
                     record.id.ToString());
        return;
    }

    // Copy properties from base record
    // (In a full implementation, this would recursively resolve inheritance)
    spdlog::debug("Record {} inherits from {}",
                  record.id.ToString(),
                  record.baseRecord->ToString());
}

void ModificationApplicator::ApplyRecordProperties(
    TweakDB& db,
    const TweakDBID& recordId,
    const std::vector<RecordPropertyModification>& properties) {

    for (const auto& prop : properties) {
        try {
            // Handle array operations on properties
            if (prop.operation == ArrayOperation::Append) {
                // TODO: Implement property-level array append
                spdlog::warn("Array append on record properties not yet implemented");
            } else if (prop.operation == ArrayOperation::Remove) {
                // TODO: Implement property-level array remove
                spdlog::warn("Array remove on record properties not yet implemented");
            } else {
                // Normal property assignment
                db.SetRecordProperty(recordId, prop.propertyName, prop.value, prop.type);
            }
        } catch (const std::exception& e) {
            AddError("Failed to apply property " + prop.propertyName +
                     " to record " + recordId.ToString() + ": " + e.what());
        }
    }
}

void ModificationApplicator::ResetStatistics() {
    appliedFlats_ = 0;
    appliedRecords_ = 0;
}

void ModificationApplicator::AddError(const String& message) {
    errors_.push_back(message);
}

} // namespace TweakXL
