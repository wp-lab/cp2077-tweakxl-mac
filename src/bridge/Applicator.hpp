#pragma once

#include "../model/TweakSource.hpp"
#include "../tweakdb/Database.hpp"
#include <vector>

namespace TweakXL {

/**
 * ModificationApplicator - Applies TweakSource modifications to TweakDB
 *
 * Handles:
 * - Flat modifications (assign/append/remove)
 * - Record creation and modification
 * - Record inheritance resolution
 * - Type coercion
 * - Validation
 */
class ModificationApplicator {
public:
    ModificationApplicator() = default;

    // Apply all modifications from a TweakSource
    bool Apply(TweakDB& db, const TweakSource& source);

    // Apply individual modifications
    void ApplyFlat(TweakDB& db, const FlatModification& flat);
    void ApplyRecord(TweakDB& db, const RecordModification& record);

    // Error handling
    const std::vector<String>& GetErrors() const { return errors_; }
    bool HasErrors() const { return !errors_.empty(); }
    void ClearErrors() { errors_.clear(); }

    // Statistics
    size_t GetAppliedFlatCount() const { return appliedFlats_; }
    size_t GetAppliedRecordCount() const { return appliedRecords_; }
    void ResetStatistics();

private:
    std::vector<String> errors_;
    size_t appliedFlats_ = 0;
    size_t appliedRecords_ = 0;

    // Array operations
    void ApplyArrayOperation(TweakDB& db, const FlatModification& flat);

    // Record helpers
    void ApplyRecordInheritance(TweakDB& db, const RecordModification& record);
    void ApplyRecordProperties(TweakDB& db, const TweakDBID& recordId,
                               const std::vector<RecordPropertyModification>& properties);

    // Error handling
    void AddError(const String& message);
};

} // namespace TweakXL
