#include "BinaryWriter.hpp"
#include <spdlog/spdlog.h>
#include <cstring>

namespace TweakXL {

BinaryWriter::BinaryWriter(const String& filepath)
    : stream_(filepath, std::ios::binary | std::ios::out)
{
    if (!stream_.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + filepath);
    }
}

BinaryWriter::~BinaryWriter() {
    if (stream_.is_open()) {
        stream_.close();
    }
}

// Stream state
bool BinaryWriter::IsOpen() const {
    return stream_.is_open();
}

bool BinaryWriter::IsGood() {
    return stream_.good();
}

size_t BinaryWriter::Tell() {
    return stream_.tellp();
}

void BinaryWriter::Seek(size_t position) {
    stream_.seekp(position);
}

void BinaryWriter::Flush() {
    stream_.flush();
}

// Primitive writes - little-endian
template<typename T>
void BinaryWriter::WriteLittleEndian(T value) {
    uint8 bytes[sizeof(T)];

    for (size_t i = 0; i < sizeof(T); ++i) {
        bytes[i] = static_cast<uint8>((value >> (i * 8)) & 0xFF);
    }

    WriteBytes(bytes, sizeof(T));
}

void BinaryWriter::WriteUInt8(uint8 value) {
    WriteBytes(&value, 1);
}

void BinaryWriter::WriteUInt16(uint16 value) {
    WriteLittleEndian<uint16>(value);
}

void BinaryWriter::WriteUInt32(uint32 value) {
    WriteLittleEndian<uint32>(value);
}

void BinaryWriter::WriteUInt64(uint64 value) {
    WriteLittleEndian<uint64>(value);
}

void BinaryWriter::WriteInt8(int8 value) {
    WriteUInt8(static_cast<uint8>(value));
}

void BinaryWriter::WriteInt16(int16 value) {
    WriteUInt16(static_cast<uint16>(value));
}

void BinaryWriter::WriteInt32(int32 value) {
    WriteUInt32(static_cast<uint32>(value));
}

void BinaryWriter::WriteInt64(int64 value) {
    WriteUInt64(static_cast<uint64>(value));
}

void BinaryWriter::WriteFloat(float value) {
    uint32 bits;
    std::memcpy(&bits, &value, sizeof(float));
    WriteUInt32(bits);
}

void BinaryWriter::WriteDouble(double value) {
    uint64 bits;
    std::memcpy(&bits, &value, sizeof(double));
    WriteUInt64(bits);
}

void BinaryWriter::WriteBool(bool value) {
    WriteUInt8(value ? 1 : 0);
}

// Variable Length Quantity (VLQ) encoding
// Format: 7 bits of data per byte, MSB is continuation bit
void BinaryWriter::WriteVLQInt32(int32 value) {
    WriteVLQUInt32(static_cast<uint32>(value));
}

void BinaryWriter::WriteVLQUInt32(uint32 value) {
    do {
        uint8 byte = static_cast<uint8>(value & 0x7F);
        value >>= 7;

        // Set MSB if more bytes to follow
        if (value != 0) {
            byte |= 0x80;
        }

        WriteUInt8(byte);
    } while (value != 0);
}

// String writing
void BinaryWriter::WriteString(const String& str) {
    if (!str.empty()) {
        WriteBytes(str.data(), str.size());
    }
}

void BinaryWriter::WriteVLQString(const String& str) {
    WriteVLQUInt32(static_cast<uint32>(str.size()));
    WriteString(str);
}

void BinaryWriter::WriteNullTerminatedString(const String& str) {
    WriteString(str);
    WriteUInt8(0); // Null terminator
}

// Raw byte writing
void BinaryWriter::WriteBytes(const void* buffer, size_t count) {
    if (count == 0) {
        return;
    }

    stream_.write(static_cast<const char*>(buffer), count);

    if (!stream_.good()) {
        throw std::runtime_error("Failed to write bytes to stream");
    }
}

void BinaryWriter::WriteBytes(const std::vector<uint8>& data) {
    if (!data.empty()) {
        WriteBytes(data.data(), data.size());
    }
}

} // namespace TweakXL
