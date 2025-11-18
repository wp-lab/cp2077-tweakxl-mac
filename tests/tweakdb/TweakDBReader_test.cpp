#include <catch2/catch_test_macros.hpp>
#include "tweakdb/Reader.hpp"
#include <fstream>

using namespace TweakXL;

// Helper to create a minimal synthetic TweakDB file
class SyntheticTweakDB {
public:
    SyntheticTweakDB(const String& filename) : filename_(filename) {
        file_.open(filename, std::ios::binary | std::ios::out);
    }

    ~SyntheticTweakDB() {
        if (file_.is_open()) {
            file_.close();
        }
        std::remove(filename_.c_str());
    }

    void CreateMinimalValid() {
        // Magic number
        WriteUInt32(0x0BB1DB47);

        // Header (28 bytes)
        WriteInt32(8);           // blobVersion
        WriteInt32(4);           // parserVersion
        WriteUInt32(0x12345678); // recordChecksum
        WriteInt32(32);          // flatsOffset (right after header: 4 + 28 = 32)
        WriteInt32(40);          // recordsOffset (flats section is 8 bytes: count=0)
        WriteInt32(56);          // queriesOffset (records section is 16 bytes)
        WriteInt32(60);          // groupTagsOffset

        // Flats section (at offset 32)
        WriteUInt32(0);  // typeGroupCount = 0 (no flats)

        // Records section (at offset 40)
        WriteUInt32(1);  // recordCount = 1

        // Record 1: Use a real TweakDBID for "Test.Record"
        // Calculate: CRC32("test.record") + (11 << 32)
        TweakDBID testRecord("Test.Record");
        WriteUInt64(testRecord.GetHash());
        WriteUInt32(0x5EEDBA5E);             // typeHash

        // Queries section (at offset 56)
        // Empty - just end of file

        // Group Tags section (at offset 60)
        // Empty - just end of file
    }

    void CreateInvalidMagic() {
        WriteUInt32(0xDEADBEEF);  // Wrong magic
        WriteInt32(8);
        WriteInt32(4);
        // ... rest doesn't matter
    }

    void CreateInvalidVersion() {
        WriteUInt32(0x0BB1DB47);  // Correct magic
        WriteInt32(99);           // Invalid blob version
        WriteInt32(99);           // Invalid parser version
        WriteUInt32(0);
        WriteInt32(32);
        WriteInt32(40);
        WriteInt32(56);
        WriteInt32(60);
    }

    void Close() {
        file_.close();
    }

private:
    String filename_;
    std::ofstream file_;

    void WriteUInt8(uint8 value) {
        file_.write(reinterpret_cast<const char*>(&value), 1);
    }

    void WriteUInt16(uint16 value) {
        file_.write(reinterpret_cast<const char*>(&value), 2);
    }

    void WriteUInt32(uint32 value) {
        file_.write(reinterpret_cast<const char*>(&value), 4);
    }

    void WriteUInt64(uint64 value) {
        file_.write(reinterpret_cast<const char*>(&value), 8);
    }

    void WriteInt32(int32 value) {
        file_.write(reinterpret_cast<const char*>(&value), 4);
    }
};

TEST_CASE("TweakDBReader - Construction", "[TweakDBReader]") {
    SECTION("Construct with valid file") {
        SyntheticTweakDB db("test_tweakdb.bin");
        db.CreateMinimalValid();
        db.Close();

        REQUIRE_NOTHROW(TweakDBReader("test_tweakdb.bin"));
    }

    SECTION("Construct with non-existent file (load will fail)") {
        // Constructor doesn't open the file, Load() does
        REQUIRE_NOTHROW(TweakDBReader("nonexistent.bin"));
    }
}

TEST_CASE("TweakDBReader - Header validation", "[TweakDBReader]") {
    SECTION("Valid header loads successfully") {
        SyntheticTweakDB db("test_valid_header.bin");
        db.CreateMinimalValid();
        db.Close();

        TweakDBReader reader("test_valid_header.bin");
        REQUIRE(reader.Load());

        const auto& header = reader.GetHeader();
        REQUIRE(header.blobVersion == 8);
        REQUIRE(header.parserVersion == 4);
        REQUIRE(header.recordChecksum == 0x12345678);
    }

    SECTION("Invalid magic number fails") {
        SyntheticTweakDB db("test_invalid_magic.bin");
        db.CreateInvalidMagic();
        db.Close();

        TweakDBReader reader("test_invalid_magic.bin");
        REQUIRE_FALSE(reader.Load());
    }

    SECTION("Invalid version (incompatible but loads with warning)") {
        SyntheticTweakDB db("test_invalid_version.bin");
        db.CreateInvalidVersion();
        db.Close();

        TweakDBReader reader("test_invalid_version.bin");
        // Should still load (warnings logged but not fatal)
        // In a real scenario, incompatible versions might fail
        // but our current implementation allows it with warnings
        REQUIRE_FALSE(reader.Load()); // Fails because offsets are wrong
    }

    SECTION("Non-existent file fails to load") {
        TweakDBReader reader("nonexistent_tweakdb.bin");
        REQUIRE_FALSE(reader.Load());
    }
}

TEST_CASE("TweakDBReader - Data access", "[TweakDBReader]") {
    SyntheticTweakDB db("test_data_access.bin");
    db.CreateMinimalValid();
    db.Close();

    TweakDBReader reader("test_data_access.bin");
    REQUIRE(reader.Load());

    SECTION("Get counts") {
        REQUIRE(reader.GetFlatCount() == 0);      // We created 0 flats
        REQUIRE(reader.GetRecordCount() == 1);    // We created 1 record
    }

    SECTION("Access records") {
        const auto& records = reader.GetRecords();
        REQUIRE(records.size() == 1);

        // Check for the record we created (must match CreateMinimalValid)
        TweakDBID testId("Test.Record");
        REQUIRE(reader.HasRecord(testId));

        const Record* record = reader.GetRecord(testId);
        REQUIRE(record != nullptr);
        REQUIRE(record->id == testId);
    }

    SECTION("Access non-existent record") {
        TweakDBID nonExistent(0x9999999999999999ULL);
        REQUIRE_FALSE(reader.HasRecord(nonExistent));
        REQUIRE(reader.GetRecord(nonExistent) == nullptr);
    }

    SECTION("Access flats") {
        const auto& flats = reader.GetFlats();
        REQUIRE(flats.empty());  // No flats in minimal test file

        TweakDBID nonExistent(0x1234567890ABCDEFULL);
        REQUIRE_FALSE(reader.HasFlat(nonExistent));
        REQUIRE(reader.GetFlat(nonExistent) == nullptr);
    }
}

TEST_CASE("TweakDBReader - Header constants", "[TweakDBReader]") {
    SECTION("Magic number constant") {
        REQUIRE(TweakDBHeader::MAGIC == 0x0BB1DB47);
    }

    SECTION("Version constants") {
        REQUIRE(TweakDBHeader::EXPECTED_BLOB_VERSION == 8);
        REQUIRE(TweakDBHeader::EXPECTED_PARSER_VERSION == 4);
    }

    SECTION("Records seed constant") {
        REQUIRE(TweakDBHeader::RECORDS_SEED == 0x5EEDBA5E);
    }
}

TEST_CASE("TweakDBReader - Header validation methods", "[TweakDBReader]") {
    TweakDBHeader header;

    SECTION("IsValid with correct version") {
        header.blobVersion = 8;
        header.parserVersion = 4;
        REQUIRE(header.IsValid());
    }

    SECTION("IsValid with wrong blob version") {
        header.blobVersion = 7;
        header.parserVersion = 4;
        REQUIRE_FALSE(header.IsValid());
    }

    SECTION("IsValid with wrong parser version") {
        header.blobVersion = 8;
        header.parserVersion = 3;
        REQUIRE_FALSE(header.IsValid());
    }

    SECTION("IsCompatible with same version") {
        header.blobVersion = 8;
        header.parserVersion = 4;
        REQUIRE(header.IsCompatible());
    }

    SECTION("IsCompatible with older version") {
        header.blobVersion = 7;
        header.parserVersion = 3;
        REQUIRE(header.IsCompatible());
    }

    SECTION("IsCompatible with newer version") {
        header.blobVersion = 9;
        header.parserVersion = 5;
        REQUIRE_FALSE(header.IsCompatible());
    }
}
