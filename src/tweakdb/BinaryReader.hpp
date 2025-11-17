#pragma once

#include "../core/Types.hpp"
#include <fstream>
#include <vector>
#include <stdexcept>

namespace TweakXL {

/**
 * BinaryReader - Utility for reading binary data with little-endian encoding
 *
 * Provides:
 * - Primitive type reading (uint8-64, int8-64, float, double)
 * - Variable Length Quantity (VLQ) decoding
 * - String reading
 * - Stream position management
 * - Little-endian byte order handling
 */
class BinaryReader {
public:
    // Constructor
    explicit BinaryReader(const String& filepath);
    explicit BinaryReader(std::ifstream&& stream);
    ~BinaryReader();

    // Disable copy, allow move
    BinaryReader(const BinaryReader&) = delete;
    BinaryReader& operator=(const BinaryReader&) = delete;
    BinaryReader(BinaryReader&&) = default;
    BinaryReader& operator=(BinaryReader&&) = default;

    // Stream state
    bool IsOpen() const;
    bool IsEOF();
    bool IsGood();
    size_t Tell();
    void Seek(size_t position);
    void Skip(size_t bytes);
    size_t Size() const;

    // Primitive reads (little-endian)
    uint8 ReadUInt8();
    uint16 ReadUInt16();
    uint32 ReadUInt32();
    uint64 ReadUInt64();

    int8 ReadInt8();
    int16 ReadInt16();
    int32 ReadInt32();
    int64 ReadInt64();

    float ReadFloat();
    double ReadDouble();
    bool ReadBool();

    // Variable Length Quantity (VLQ) decoding
    // Used for array counts and string lengths
    int32 ReadVLQInt32();
    uint32 ReadVLQUInt32();

    // String reading
    String ReadString(size_t length);      // Read fixed-length string
    String ReadVLQString();                 // Read VLQ-prefixed string
    String ReadNullTerminatedString();      // Read null-terminated string

    // Raw byte reading
    void ReadBytes(void* buffer, size_t count);
    std::vector<uint8> ReadBytes(size_t count);

    // Template for reading POD structures
    template<typename T>
    T ReadStruct() {
        static_assert(std::is_trivially_copyable<T>::value,
                      "Type must be trivially copyable");
        T value;
        ReadBytes(&value, sizeof(T));
        return value;
    }

private:
    std::ifstream stream_;
    size_t size_;

    // Helper to convert bytes to little-endian value
    template<typename T>
    T ReadLittleEndian();
};

} // namespace TweakXL
