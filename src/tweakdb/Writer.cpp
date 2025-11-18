#include "Writer.hpp"
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <algorithm>

namespace TweakXL {

TweakDBWriter::TweakDBWriter(const String& filepath)
    : filepath_(filepath)
    , header_{}
{
    // Set default header values
    header_.blobVersion = TweakDBHeader::EXPECTED_BLOB_VERSION;
    header_.parserVersion = TweakDBHeader::EXPECTED_PARSER_VERSION;
    header_.recordChecksum = 0; // Will be calculated during write
    header_.flatsOffset = 0;
    header_.recordsOffset = 0;
    header_.queriesOffset = 0;
    header_.groupTagsOffset = 0;
}

TweakDBWriter::~TweakDBWriter() = default;

bool TweakDBWriter::Write(const TweakDB& db) {
    try {
        // Validate database before writing
        if (!db.IsValid()) {
            spdlog::error("Cannot write invalid TweakDB");
            auto errors = db.Validate();
            for (const auto& error : errors) {
                spdlog::error("  - {}", error);
            }
            return false;
        }

        // Open file for writing
        writer_ = std::make_unique<BinaryWriter>(filepath_);

        if (!writer_->IsOpen()) {
            spdlog::error("Failed to open file for writing: {}", filepath_);
            return false;
        }

        spdlog::info("Writing TweakDB to: {}", filepath_);

        // Calculate checksum
        header_.recordChecksum = CalculateChecksum(db);

        // Write magic number first
        writer_->WriteUInt32(TweakDBHeader::MAGIC);

        // Reserve space for header (we'll come back to write offsets)
        size_t headerStart = writer_->Tell();
        writer_->WriteInt32(header_.blobVersion);
        writer_->WriteInt32(header_.parserVersion);
        writer_->WriteUInt32(header_.recordChecksum);
        writer_->WriteInt32(0); // flatsOffset placeholder
        writer_->WriteInt32(0); // recordsOffset placeholder
        writer_->WriteInt32(0); // queriesOffset placeholder
        writer_->WriteInt32(0); // groupTagsOffset placeholder

        // Write flats section
        header_.flatsOffset = static_cast<int32>(writer_->Tell());
        if (!WriteFlats(db)) {
            spdlog::error("Failed to write Flats section");
            return false;
        }

        // Write records section
        header_.recordsOffset = static_cast<int32>(writer_->Tell());
        if (!WriteRecords(db)) {
            spdlog::error("Failed to write Records section");
            return false;
        }

        // Write queries section (empty for now)
        header_.queriesOffset = static_cast<int32>(writer_->Tell());
        if (!WriteQueries()) {
            spdlog::error("Failed to write Queries section");
            return false;
        }

        // Write group tags section (empty for now)
        header_.groupTagsOffset = static_cast<int32>(writer_->Tell());
        if (!WriteGroupTags()) {
            spdlog::error("Failed to write Group Tags section");
            return false;
        }

        // Go back and write the header with correct offsets
        size_t endPosition = writer_->Tell();
        writer_->Seek(headerStart);
        writer_->WriteInt32(header_.blobVersion);
        writer_->WriteInt32(header_.parserVersion);
        writer_->WriteUInt32(header_.recordChecksum);
        writer_->WriteInt32(header_.flatsOffset);
        writer_->WriteInt32(header_.recordsOffset);
        writer_->WriteInt32(header_.queriesOffset);
        writer_->WriteInt32(header_.groupTagsOffset);

        // Return to end of file
        writer_->Seek(endPosition);

        writer_->Flush();

        spdlog::info("TweakDB written successfully: {} flats, {} records",
                     db.GetFlatCount(), db.GetRecordCount());

        return true;

    } catch (const std::exception& e) {
        spdlog::error("Exception while writing TweakDB: {}", e.what());
        return false;
    }
}

bool TweakDBWriter::WriteFlats(const TweakDB& db) {
    spdlog::debug("Writing Flats section at offset {}", writer_->Tell());

    // Group flats by type for efficient serialization
    auto typeGroups = GroupFlatsByType(db);

    // Write number of type groups
    writer_->WriteUInt32(static_cast<uint32>(typeGroups.size()));

    for (const auto& group : typeGroups) {
        // Write type ID (simplified - using enum value)
        writer_->WriteUInt32(static_cast<uint32>(group.type));

        // Write value count
        writer_->WriteUInt32(static_cast<uint32>(group.values.size()));

        // Write all values for this type
        for (const auto& value : group.values) {
            WriteValue(value, group.type);
        }

        // Write key count (must match value count)
        writer_->WriteUInt32(static_cast<uint32>(group.keys.size()));

        // Write all keys (TweakDBIDs)
        for (const auto& key : group.keys) {
            writer_->WriteUInt64(key.GetHash());
        }
    }

    spdlog::debug("Wrote {} type groups", typeGroups.size());
    return true;
}

bool TweakDBWriter::WriteRecords(const TweakDB& db) {
    spdlog::debug("Writing Records section at offset {}", writer_->Tell());

    const auto& records = db.GetAllRecords();

    // Write number of records
    writer_->WriteUInt32(static_cast<uint32>(records.size()));

    for (const auto& [id, record] : records) {
        // Write record ID
        writer_->WriteUInt64(record.id.GetHash());

        // Write type hash (simplified - using a basic hash for now)
        // TODO: Implement proper Murmur3-32 hash with RECORDS_SEED
        uint32 typeHash = static_cast<uint32>(record.type);
        writer_->WriteUInt32(typeHash);
    }

    spdlog::debug("Wrote {} records", records.size());
    return true;
}

bool TweakDBWriter::WriteQueries() {
    spdlog::debug("Writing Queries section at offset {} (empty)", writer_->Tell());
    // Write empty queries section - 0 count
    writer_->WriteUInt32(0);
    return true;
}

bool TweakDBWriter::WriteGroupTags() {
    spdlog::debug("Writing Group Tags section at offset {} (empty)", writer_->Tell());
    // Write empty group tags section - 0 count
    writer_->WriteUInt32(0);
    return true;
}

void TweakDBWriter::WriteValue(const Value& value, TypeID type) {
    switch (type) {
        case TypeID::Bool:
            writer_->WriteBool(value.AsBool());
            break;

        case TypeID::Int32:
            writer_->WriteInt32(value.AsInt32());
            break;

        case TypeID::Float:
            writer_->WriteFloat(value.AsFloat());
            break;

        case TypeID::String:
            writer_->WriteVLQString(value.AsString());
            break;

        case TypeID::CName:
            writer_->WriteUInt64(value.AsCName().hash);
            break;

        case TypeID::TweakDBID:
            writer_->WriteUInt64(value.AsTweakDBID().GetHash());
            break;

        case TypeID::LocKey:
            writer_->WriteUInt64(value.AsLocKey().key);
            break;

        case TypeID::Resource:
            writer_->WriteVLQString(value.AsResource().path);
            break;

        case TypeID::Array:
            // Arrays need element type - this should not be called directly
            // Arrays are handled in GroupFlatsByType with specific element types
            throw std::runtime_error("Array type requires element type for serialization");

        default:
            throw std::runtime_error("Unsupported type for serialization: " +
                                     std::to_string(static_cast<uint32>(type)));
    }
}

void TweakDBWriter::WriteArray(const Value& value, TypeID elementType) {
    if (!value.IsArray()) {
        throw std::runtime_error("Expected array value");
    }

    const auto& elements = value.AsArray();

    // Write array count using VLQ
    writer_->WriteVLQUInt32(static_cast<uint32>(elements.size()));

    // Write each element
    for (const auto& element : elements) {
        WriteValue(element, elementType);
    }
}

std::vector<TweakDBWriter::TypeGroup> TweakDBWriter::GroupFlatsByType(const TweakDB& db) {
    const auto& flats = db.GetAllFlats();

    // Group flats by their type
    std::unordered_map<TypeID, TypeGroup> groups;

    for (const auto& [id, flat] : flats) {
        TypeID type = flat.type;

        // Arrays are special - group by array element type
        // For now, treat all arrays as generic arrays
        if (flat.value.IsArray()) {
            type = TypeID::Array;
        }

        auto& group = groups[type];
        if (group.type == TypeID::Unknown) {
            group.type = type;
        }

        group.keys.push_back(flat.id);
        group.values.push_back(flat.value);
    }

    // Convert map to vector and sort by type for consistent output
    std::vector<TypeGroup> result;
    result.reserve(groups.size());

    for (auto& [type, group] : groups) {
        result.push_back(std::move(group));
    }

    // Sort by type ID for deterministic output
    std::sort(result.begin(), result.end(),
              [](const TypeGroup& a, const TypeGroup& b) {
                  return static_cast<uint32>(a.type) < static_cast<uint32>(b.type);
              });

    return result;
}

uint32 TweakDBWriter::CalculateChecksum(const TweakDB& db) {
    // Simplified checksum - just XOR all record ID hashes
    // TODO: Implement proper CRC32 checksum matching game's format
    uint32 checksum = 0;

    for (const auto& [id, record] : db.GetAllRecords()) {
        checksum ^= static_cast<uint32>(record.id.GetHash() & 0xFFFFFFFF);
    }

    return checksum;
}

} // namespace TweakXL
