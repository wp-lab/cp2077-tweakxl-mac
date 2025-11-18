#include "TweakSource.hpp"
#include <spdlog/spdlog.h>

namespace TweakXL {

TweakSource::TweakSource(const String& filepath)
    : filepath_(filepath)
{
}

void TweakSource::AddFlat(FlatModification flat) {
    flats_.push_back(std::move(flat));
}

void TweakSource::AddRecord(RecordModification record) {
    records_.push_back(std::move(record));
}

size_t TweakSource::GetTotalModificationCount() const {
    size_t count = flats_.size();
    for (const auto& record : records_) {
        count += record.properties.size();
    }
    return count;
}

bool TweakSource::IsValid() const {
    return Validate().empty();
}

std::vector<String> TweakSource::Validate() const {
    std::vector<String> errors;

    // Validate flats
    for (size_t i = 0; i < flats_.size(); ++i) {
        const auto& flat = flats_[i];

        if (!flat.id.IsValid()) {
            errors.push_back("Flat #" + std::to_string(i) + ": Invalid TweakDBID");
        }

        if (flat.value.IsEmpty()) {
            errors.push_back("Flat #" + std::to_string(i) + " (" + flat.id.ToString() + "): Empty value");
        }

        if (flat.type == TypeID::Unknown) {
            errors.push_back("Flat #" + std::to_string(i) + " (" + flat.id.ToString() + "): Unknown type");
        }
    }

    // Validate records
    for (size_t i = 0; i < records_.size(); ++i) {
        const auto& record = records_[i];

        if (!record.id.IsValid()) {
            errors.push_back("Record #" + std::to_string(i) + ": Invalid TweakDBID");
        }

        if (record.type == TypeID::Unknown) {
            errors.push_back("Record #" + std::to_string(i) + " (" + record.id.ToString() + "): Unknown type");
        }

        // Validate properties
        for (size_t j = 0; j < record.properties.size(); ++j) {
            const auto& prop = record.properties[j];

            if (prop.propertyName.empty()) {
                errors.push_back("Record #" + std::to_string(i) + " (" + record.id.ToString() +
                                 "), Property #" + std::to_string(j) + ": Empty property name");
            }

            if (prop.value.IsEmpty()) {
                errors.push_back("Record #" + std::to_string(i) + " (" + record.id.ToString() +
                                 "), Property '" + prop.propertyName + "': Empty value");
            }

            if (prop.type == TypeID::Unknown) {
                errors.push_back("Record #" + std::to_string(i) + " (" + record.id.ToString() +
                                 "), Property '" + prop.propertyName + "': Unknown type");
            }
        }

        // Check for circular inheritance (basic check)
        if (record.HasBase() && record.baseRecord.value() == record.id) {
            errors.push_back("Record #" + std::to_string(i) + " (" + record.id.ToString() +
                             "): Cannot inherit from itself");
        }
    }

    return errors;
}

void TweakSource::Clear() {
    flats_.clear();
    records_.clear();
}

void TweakSource::Merge(const TweakSource& other) {
    // Merge flats
    flats_.insert(flats_.end(), other.flats_.begin(), other.flats_.end());

    // Merge records
    records_.insert(records_.end(), other.records_.begin(), other.records_.end());

    spdlog::debug("Merged TweakSource: {} flats, {} records from '{}'",
                  other.flats_.size(), other.records_.size(), other.filepath_);
}

} // namespace TweakXL
