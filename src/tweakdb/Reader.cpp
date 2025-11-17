#include "Reader.hpp"
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace TweakXL {

TweakDBReader::TweakDBReader(const String& filepath)
    : filepath_(filepath)
    , header_{}
{
}

TweakDBReader::~TweakDBReader() = default;

bool TweakDBReader::Load() {
    try {
        // Open file
        reader_ = std::make_unique<BinaryReader>(filepath_);

        if (!reader_->IsOpen()) {
            spdlog::error("Failed to open TweakDB file: {}", filepath_);
            return false;
        }

        spdlog::info("Loading TweakDB from: {}", filepath_);

        // Read and validate header
        if (!ReadHeader()) {
            spdlog::error("Failed to read TweakDB header");
            return false;
        }

        spdlog::info("TweakDB Header: blob v{}, parser v{}",
                     header_.blobVersion, header_.parserVersion);

        // Read all sections
        if (!ReadFlats()) {
            spdlog::error("Failed to read Flats section");
            return false;
        }

        if (!ReadRecords()) {
            spdlog::error("Failed to read Records section");
            return false;
        }

        if (!ReadQueries()) {
            spdlog::warn("Failed to read Queries section (non-fatal)");
        }

        if (!ReadGroupTags()) {
            spdlog::warn("Failed to read Group Tags section (non-fatal)");
        }

        spdlog::info("TweakDB loaded successfully: {} flats, {} records",
                     flats_.size(), records_.size());

        return true;

    } catch (const std::exception& e) {
        spdlog::error("Exception while loading TweakDB: {}", e.what());
        return false;
    }
}

bool TweakDBReader::ReadHeader() {
    // Read magic number
    uint32 magic = reader_->ReadUInt32();
    if (magic != TweakDBHeader::MAGIC) {
        spdlog::error("Invalid magic number: 0x{:08X} (expected 0x{:08X})",
                      magic, TweakDBHeader::MAGIC);
        return false;
    }

    // Read header structure (28 bytes)
    header_.blobVersion = reader_->ReadInt32();
    header_.parserVersion = reader_->ReadInt32();
    header_.recordChecksum = reader_->ReadUInt32();
    header_.flatsOffset = reader_->ReadInt32();
    header_.recordsOffset = reader_->ReadInt32();
    header_.queriesOffset = reader_->ReadInt32();
    header_.groupTagsOffset = reader_->ReadInt32();

    // Validate versions
    if (!header_.IsCompatible()) {
        spdlog::warn("TweakDB version may be incompatible: blob v{}, parser v{} "
                     "(expected: blob v{}, parser v{})",
                     header_.blobVersion, header_.parserVersion,
                     TweakDBHeader::EXPECTED_BLOB_VERSION,
                     TweakDBHeader::EXPECTED_PARSER_VERSION);
    }

    // Validate offsets
    size_t fileSize = reader_->Size();
    if (header_.flatsOffset < 0 || static_cast<size_t>(header_.flatsOffset) > fileSize ||
        header_.recordsOffset < 0 || static_cast<size_t>(header_.recordsOffset) > fileSize) {
        spdlog::error("Invalid section offsets in header");
        return false;
    }

    return true;
}

bool TweakDBReader::ReadFlats() {
    // Seek to flats section
    reader_->Seek(header_.flatsOffset);

    spdlog::debug("Reading Flats section at offset {}", header_.flatsOffset);

    // Read number of type groups
    uint32 typeGroupCount = reader_->ReadUInt32();
    spdlog::debug("Flats: {} type groups", typeGroupCount);

    for (uint32 i = 0; i < typeGroupCount; ++i) {
        // Read type information
        // TODO: Implement type reading based on actual format
        // For now, this is a placeholder

        // Read value count
        uint32 valueCount = reader_->ReadUInt32();

        // Read all values of this type
        std::vector<Value> values;
        values.reserve(valueCount);

        for (uint32 j = 0; j < valueCount; ++j) {
            // TODO: Read value based on type
            // values.push_back(ReadValue(type));
        }

        // Read key count (should match value count)
        uint32 keyCount = reader_->ReadUInt32();
        if (keyCount != valueCount) {
            spdlog::error("Flats: key count ({}) doesn't match value count ({})",
                          keyCount, valueCount);
            return false;
        }

        // Read keys and associate with values
        for (uint32 j = 0; j < keyCount; ++j) {
            TweakDBID id(reader_->ReadUInt64());

            // TODO: Create flat with correct value
            // Flat flat;
            // flat.id = id;
            // flat.value = values[j];
            // flat.type = type;
            // flats_[id] = std::move(flat);
        }
    }

    spdlog::info("Read {} flats", flats_.size());
    return true;
}

bool TweakDBReader::ReadRecords() {
    // Seek to records section
    reader_->Seek(header_.recordsOffset);

    spdlog::debug("Reading Records section at offset {}", header_.recordsOffset);

    // Read number of records
    uint32 recordCount = reader_->ReadUInt32();
    spdlog::debug("Records: {} entries", recordCount);

    records_.reserve(recordCount);

    for (uint32 i = 0; i < recordCount; ++i) {
        // Read record ID
        TweakDBID recordId(reader_->ReadUInt64());

        // Read type hash (Murmur3-32 with seed 0x5EEDBA5E)
        uint32 typeHash = reader_->ReadUInt32();

        // Create record
        Record record;
        record.id = recordId;
        record.type = TypeID::Unknown; // TODO: Map type hash to TypeID

        // Record properties come from flats with matching prefix
        // This will be resolved after all data is loaded

        records_[recordId] = std::move(record);
    }

    spdlog::info("Read {} records", records_.size());
    return true;
}

bool TweakDBReader::ReadQueries() {
    // Queries are optional for our use case
    // We'll implement this if needed later
    spdlog::debug("Skipping Queries section (not implemented yet)");
    return true;
}

bool TweakDBReader::ReadGroupTags() {
    // Group tags are optional for our use case
    // We'll implement this if needed later
    spdlog::debug("Skipping Group Tags section (not implemented yet)");
    return true;
}

// Value reading methods (to be fully implemented)
Value TweakDBReader::ReadValue(TypeID type) {
    switch (type) {
        case TypeID::Bool:
            return Value(reader_->ReadBool());

        case TypeID::Int32:
            return Value(reader_->ReadInt32());

        case TypeID::Float:
            return Value(reader_->ReadFloat());

        case TypeID::String: {
            String str = reader_->ReadVLQString();
            return Value(str);
        }

        case TypeID::CName: {
            uint64 hash = reader_->ReadUInt64();
            return Value(CName(hash));
        }

        case TypeID::TweakDBID: {
            uint64 hash = reader_->ReadUInt64();
            return Value(TweakDBID(hash));
        }

        case TypeID::LocKey: {
            uint64 key = reader_->ReadUInt64();
            return Value(LocKey(key));
        }

        case TypeID::Resource: {
            String path = reader_->ReadVLQString();
            return Value(Resource(path));
        }

        case TypeID::Array: {
            // Need to know element type
            // This is handled separately in ReadArray()
            throw std::runtime_error("Array type requires element type");
        }

        // TODO: Implement geometric types
        case TypeID::Vector2:
            return ReadVector2();

        case TypeID::Vector3:
            return ReadVector3();

        case TypeID::Quaternion:
            return ReadQuaternion();

        case TypeID::EulerAngles:
            return ReadEulerAngles();

        case TypeID::Color:
            return ReadColor();

        default:
            throw std::runtime_error("Unsupported type: " +
                                     std::to_string(static_cast<uint32>(type)));
    }
}

Value TweakDBReader::ReadArray(TypeID elementType) {
    uint32 count = reader_->ReadVLQUInt32();

    std::vector<Value> elements;
    elements.reserve(count);

    for (uint32 i = 0; i < count; ++i) {
        elements.push_back(ReadValue(elementType));
    }

    return Value(std::move(elements));
}

Value TweakDBReader::ReadVector2() {
    // Vector2: 2 floats (X, Y)
    // TODO: Implement when Vector2 is added to Value variant
    reader_->Skip(2 * sizeof(float));
    return Value(); // Placeholder
}

Value TweakDBReader::ReadVector3() {
    // Vector3: 3 floats (X, Y, Z)
    // TODO: Implement when Vector3 is added to Value variant
    reader_->Skip(3 * sizeof(float));
    return Value(); // Placeholder
}

Value TweakDBReader::ReadQuaternion() {
    // Quaternion: 4 floats (I, J, K, R)
    // TODO: Implement when Quaternion is added to Value variant
    reader_->Skip(4 * sizeof(float));
    return Value(); // Placeholder
}

Value TweakDBReader::ReadEulerAngles() {
    // EulerAngles: 3 floats (Pitch, Yaw, Roll)
    // TODO: Implement when EulerAngles is added to Value variant
    reader_->Skip(3 * sizeof(float));
    return Value(); // Placeholder
}

Value TweakDBReader::ReadColor() {
    // Color: uint32 RGBA
    // TODO: Implement when Color is added to Value variant
    reader_->Skip(sizeof(uint32));
    return Value(); // Placeholder
}

// Lookup methods
bool TweakDBReader::HasFlat(const TweakDBID& id) const {
    return flats_.find(id) != flats_.end();
}

bool TweakDBReader::HasRecord(const TweakDBID& id) const {
    return records_.find(id) != records_.end();
}

const Flat* TweakDBReader::GetFlat(const TweakDBID& id) const {
    auto it = flats_.find(id);
    return (it != flats_.end()) ? &it->second : nullptr;
}

const Record* TweakDBReader::GetRecord(const TweakDBID& id) const {
    auto it = records_.find(id);
    return (it != records_.end()) ? &it->second : nullptr;
}

} // namespace TweakXL
