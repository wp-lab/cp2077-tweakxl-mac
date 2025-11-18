#pragma once

#include "../core/Types.hpp"
#include "../core/TweakDBID.hpp"
#include "../core/Value.hpp"
#include "../core/Flat.hpp"
#include "../core/Record.hpp"
#include "Header.hpp"
#include "BinaryReader.hpp"
#include <memory>
#include <unordered_map>
#include <vector>

namespace TweakXL {

/**
 * TweakDBReader - Parses TweakDB binary files (tweakdb.bin)
 *
 * Reads the binary format and loads:
 * - Flats (key-value pairs)
 * - Records (typed objects)
 * - Queries (result sets)
 * - Group Tags (categorizations)
 */
class TweakDBReader {
public:
    // Constructor
    explicit TweakDBReader(const String& filepath);
    ~TweakDBReader();

    // Disable copy, allow move
    TweakDBReader(const TweakDBReader&) = delete;
    TweakDBReader& operator=(const TweakDBReader&) = delete;
    TweakDBReader(TweakDBReader&&) = default;
    TweakDBReader& operator=(TweakDBReader&&) = default;

    // Load the entire database
    bool Load();

    // Get header information
    const TweakDBHeader& GetHeader() const { return header_; }

    // Access loaded data
    const std::unordered_map<TweakDBID, Flat>& GetFlats() const { return flats_; }
    const std::unordered_map<TweakDBID, Record>& GetRecords() const { return records_; }

    // Lookup methods
    bool HasFlat(const TweakDBID& id) const;
    bool HasRecord(const TweakDBID& id) const;

    const Flat* GetFlat(const TweakDBID& id) const;
    const Record* GetRecord(const TweakDBID& id) const;

    // Statistics
    size_t GetFlatCount() const { return flats_.size(); }
    size_t GetRecordCount() const { return records_.size(); }

private:
    // File reading
    std::unique_ptr<BinaryReader> reader_;
    String filepath_;

    // Parsed data
    TweakDBHeader header_;
    std::unordered_map<TweakDBID, Flat> flats_;
    std::unordered_map<TweakDBID, Record> records_;

    // Parsing methods
    bool ReadHeader();
    bool ReadFlats();
    bool ReadRecords();
    bool ReadQueries();
    bool ReadGroupTags();

    // Type-specific value readers
    Value ReadValue(TypeID type);
    Value ReadArray(TypeID elementType);
    Value ReadVector2();
    Value ReadVector3();
    Value ReadQuaternion();
    Value ReadEulerAngles();
    Value ReadColor();
};

} // namespace TweakXL
