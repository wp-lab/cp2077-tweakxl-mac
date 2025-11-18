#pragma once

#include "../core/Types.hpp"
#include <fstream>
#include <vector>
#include <stdexcept>

namespace TweakXL {

/**
 * BinaryWriter - Utility for writing binary data with little-endian encoding
 *
 * Provides:
 * - Primitive type writing (uint8-64, int8-64, float, double)
 * - Variable Length Quantity (VLQ) encoding
 * - String writing
 * - Stream position management
 * - Little-endian byte order handling
 */
class BinaryWriter {
public:
    // Constructor
    explicit BinaryWriter(const String& filepath);
    ~BinaryWriter();

    // Disable copy, allow move
    BinaryWriter(const BinaryWriter&) = delete;
    BinaryWriter& operator=(const BinaryWriter&) = delete;
    BinaryWriter(BinaryWriter&&) = default;
    BinaryWriter& operator=(BinaryWriter&&) = default;

    // Stream state
    bool IsOpen() const;
    bool IsGood();
    size_t Tell();
    void Seek(size_t position);
    void Flush();

    // Primitive writes (little-endian)
    void WriteUInt8(uint8 value);
    void WriteUInt16(uint16 value);
    void WriteUInt32(uint32 value);
    void WriteUInt64(uint64 value);

    void WriteInt8(int8 value);
    void WriteInt16(int16 value);
    void WriteInt32(int32 value);
    void WriteInt64(int64 value);

    void WriteFloat(float value);
    void WriteDouble(double value);
    void WriteBool(bool value);

    // Variable Length Quantity (VLQ) encoding
    // Used for array counts and string lengths
    void WriteVLQInt32(int32 value);
    void WriteVLQUInt32(uint32 value);

    // String writing
    void WriteString(const String& str);         // Write raw string bytes
    void WriteVLQString(const String& str);      // Write VLQ-prefixed string
    void WriteNullTerminatedString(const String& str); // Write null-terminated string

    // Raw byte writing
    void WriteBytes(const void* buffer, size_t count);
    void WriteBytes(const std::vector<uint8>& data);

    // Template for writing POD structures
    template<typename T>
    void WriteStruct(const T& value) {
        static_assert(std::is_trivially_copyable<T>::value,
                      "Type must be trivially copyable");
        WriteBytes(&value, sizeof(T));
    }

private:
    std::ofstream stream_;

    // Helper to write value in little-endian format
    template<typename T>
    void WriteLittleEndian(T value);
};

} // namespace TweakXL
