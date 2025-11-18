#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "tweakdb/BinaryReader.hpp"
#include <fstream>
#include <cstring>

using namespace TweakXL;

// Helper to create a temporary test file
class TempBinaryFile {
public:
    TempBinaryFile(const String& filename) : filename_(filename) {
        file_.open(filename, std::ios::binary | std::ios::out);
    }

    ~TempBinaryFile() {
        if (file_.is_open()) {
            file_.close();
        }
        std::remove(filename_.c_str());
    }

    template<typename T>
    void Write(T value) {
        file_.write(reinterpret_cast<const char*>(&value), sizeof(T));
    }

    void WriteBytes(const void* data, size_t size) {
        file_.write(reinterpret_cast<const char*>(data), size);
    }

    void WriteString(const String& str) {
        file_.write(str.c_str(), str.length());
    }

    void Close() {
        file_.close();
    }

private:
    String filename_;
    std::ofstream file_;
};

TEST_CASE("BinaryReader - Construction and state", "[BinaryReader]") {
    SECTION("Open valid file") {
        TempBinaryFile temp("test_binary.bin");
        temp.Write<uint32>(0x12345678);
        temp.Close();

        BinaryReader reader("test_binary.bin");
        REQUIRE(reader.IsOpen());
        REQUIRE(reader.Size() == 4);
    }

    SECTION("Fail to open non-existent file") {
        REQUIRE_THROWS(BinaryReader("nonexistent_file.bin"));
    }
}

TEST_CASE("BinaryReader - Primitive reads", "[BinaryReader]") {
    TempBinaryFile temp("test_primitives.bin");

    // Write test data (little-endian)
    temp.Write<uint8>(0x42);
    temp.Write<uint16>(0x1234);
    temp.Write<uint32>(0xDEADBEEF);
    temp.Write<uint64>(0x0123456789ABCDEFULL);

    temp.Write<int8>(-42);
    temp.Write<int16>(-1234);
    temp.Write<int32>(-987654);
    temp.Write<int64>(-123456789LL);

    float f = 3.14159f;
    temp.WriteBytes(&f, sizeof(float));

    double d = 2.71828;
    temp.WriteBytes(&d, sizeof(double));

    temp.Write<uint8>(1);  // true
    temp.Write<uint8>(0);  // false

    temp.Close();

    BinaryReader reader("test_primitives.bin");

    SECTION("Read unsigned integers") {
        REQUIRE(reader.ReadUInt8() == 0x42);
        REQUIRE(reader.ReadUInt16() == 0x1234);
        REQUIRE(reader.ReadUInt32() == 0xDEADBEEF);
        REQUIRE(reader.ReadUInt64() == 0x0123456789ABCDEFULL);
    }

    SECTION("Read signed integers") {
        reader.Skip(1 + 2 + 4 + 8); // Skip unsigned ints
        REQUIRE(reader.ReadInt8() == -42);
        REQUIRE(reader.ReadInt16() == -1234);
        REQUIRE(reader.ReadInt32() == -987654);
        REQUIRE(reader.ReadInt64() == -123456789LL);
    }

    SECTION("Read floating point") {
        reader.Skip(1 + 2 + 4 + 8 + 1 + 2 + 4 + 8); // Skip integers
        float read_f = reader.ReadFloat();
        REQUIRE_THAT(read_f, Catch::Matchers::WithinAbs(3.14159f, 0.00001f));

        double read_d = reader.ReadDouble();
        REQUIRE_THAT(read_d, Catch::Matchers::WithinAbs(2.71828, 0.00001));
    }

    SECTION("Read boolean") {
        reader.Skip(1 + 2 + 4 + 8 + 1 + 2 + 4 + 8 + 4 + 8); // Skip to bools
        REQUIRE(reader.ReadBool() == true);
        REQUIRE(reader.ReadBool() == false);
    }
}

TEST_CASE("BinaryReader - VLQ decoding", "[BinaryReader]") {
    TempBinaryFile temp("test_vlq.bin");

    // VLQ encoding:
    // 0 = 0x00
    // 127 = 0x7F
    // 128 = 0x80 0x01
    // 300 = 0xAC 0x02  (300 = 0x12C = 0b100101100 = 0b0101100 0b0000010)

    temp.Write<uint8>(0x00);        // 0
    temp.Write<uint8>(0x7F);        // 127
    temp.Write<uint8>(0x80);        // 128 (part 1)
    temp.Write<uint8>(0x01);        // 128 (part 2)
    temp.Write<uint8>(0xAC);        // 300 (part 1: 0b10101100)
    temp.Write<uint8>(0x02);        // 300 (part 2: 0b00000010)

    temp.Close();

    BinaryReader reader("test_vlq.bin");

    SECTION("VLQ single byte values") {
        REQUIRE(reader.ReadVLQUInt32() == 0);
        REQUIRE(reader.ReadVLQUInt32() == 127);
    }

    SECTION("VLQ multi-byte values") {
        reader.Skip(2); // Skip first two values
        REQUIRE(reader.ReadVLQUInt32() == 128);
        REQUIRE(reader.ReadVLQUInt32() == 300);
    }
}

TEST_CASE("BinaryReader - String reading", "[BinaryReader]") {
    TempBinaryFile temp("test_strings.bin");

    // Fixed-length string
    temp.WriteString("Hello");

    // VLQ string: length=5, then "World"
    temp.Write<uint8>(5);           // VLQ: 5
    temp.WriteString("World");

    // Null-terminated string
    temp.WriteString("Test");
    temp.Write<uint8>(0);

    temp.Close();

    BinaryReader reader("test_strings.bin");

    SECTION("Read fixed-length string") {
        String str = reader.ReadString(5);
        REQUIRE(str == "Hello");
    }

    SECTION("Read VLQ-prefixed string") {
        reader.Skip(5); // Skip "Hello"
        String str = reader.ReadVLQString();
        REQUIRE(str == "World");
    }

    SECTION("Read null-terminated string") {
        reader.Skip(5 + 1 + 5); // Skip to null-terminated
        String str = reader.ReadNullTerminatedString();
        REQUIRE(str == "Test");
    }
}

TEST_CASE("BinaryReader - Stream position", "[BinaryReader]") {
    TempBinaryFile temp("test_position.bin");
    for (int i = 0; i < 10; ++i) {
        temp.Write<uint8>(i);
    }
    temp.Close();

    BinaryReader reader("test_position.bin");

    SECTION("Tell and Seek") {
        REQUIRE(reader.Tell() == 0);

        reader.ReadUInt8();
        REQUIRE(reader.Tell() == 1);

        reader.Seek(5);
        REQUIRE(reader.Tell() == 5);
        REQUIRE(reader.ReadUInt8() == 5);
    }

    SECTION("Skip") {
        reader.Skip(3);
        REQUIRE(reader.Tell() == 3);
        REQUIRE(reader.ReadUInt8() == 3);
    }

    SECTION("Size") {
        REQUIRE(reader.Size() == 10);
    }
}

TEST_CASE("BinaryReader - Raw byte reading", "[BinaryReader]") {
    TempBinaryFile temp("test_bytes.bin");
    uint8 data[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    temp.WriteBytes(data, 5);
    temp.Close();

    BinaryReader reader("test_bytes.bin");

    SECTION("Read bytes to buffer") {
        uint8 buffer[5];
        reader.ReadBytes(buffer, 5);
        for (int i = 0; i < 5; ++i) {
            REQUIRE(buffer[i] == data[i]);
        }
    }

    SECTION("Read bytes to vector") {
        auto bytes = reader.ReadBytes(5);
        REQUIRE(bytes.size() == 5);
        for (int i = 0; i < 5; ++i) {
            REQUIRE(bytes[i] == data[i]);
        }
    }
}
