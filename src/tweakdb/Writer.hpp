#pragma once

#include "../core/Types.hpp"
#include "../core/TweakDBID.hpp"
#include "../core/Value.hpp"
#include "Database.hpp"
#include "Header.hpp"
#include "BinaryWriter.hpp"
#include <memory>
#include <unordered_map>
#include <vector>

namespace TweakXL {

/**
 * TweakDBWriter - Writes TweakDB binary files (tweakdb.bin)
 *
 * Serializes a TweakDB database to binary format with:
 * - Flats (key-value pairs)
 * - Records (typed objects)
 * - Queries (result sets) - optional
 * - Group Tags (categorizations) - optional
 */
class TweakDBWriter {
public:
    // Constructor
    explicit TweakDBWriter(const String& filepath);
    ~TweakDBWriter();

    // Disable copy, allow move
    TweakDBWriter(const TweakDBWriter&) = delete;
    TweakDBWriter& operator=(const TweakDBWriter&) = delete;
    TweakDBWriter(TweakDBWriter&&) = default;
    TweakDBWriter& operator=(TweakDBWriter&&) = default;

    // Write the entire database
    bool Write(const TweakDB& db);

    // Set header values (optional, uses defaults if not set)
    void SetBlobVersion(int32 version) { header_.blobVersion = version; }
    void SetParserVersion(int32 version) { header_.parserVersion = version; }
    void SetRecordChecksum(uint32 checksum) { header_.recordChecksum = checksum; }

private:
    // File writing
    std::unique_ptr<BinaryWriter> writer_;
    String filepath_;

    // Header to write
    TweakDBHeader header_;

    // Section writing methods
    bool WriteHeader();
    bool WriteFlats(const TweakDB& db);
    bool WriteRecords(const TweakDB& db);
    bool WriteQueries();    // Empty section for now
    bool WriteGroupTags();  // Empty section for now

    // Value serialization
    void WriteValue(const Value& value, TypeID type);
    void WriteArray(const Value& value, TypeID elementType);

    // Helper to group flats by type for efficient serialization
    struct TypeGroup {
        TypeID type;
        std::vector<TweakDBID> keys;
        std::vector<Value> values;
    };

    std::vector<TypeGroup> GroupFlatsByType(const TweakDB& db);

    // Calculate checksum (simplified version for now)
    uint32 CalculateChecksum(const TweakDB& db);
};

} // namespace TweakXL
