#include "BinaryReader.hpp"
#include <cstring>

namespace TweakXL {

BinaryReader::BinaryReader(const String& filepath)
    : stream_(filepath, std::ios::binary | std::ios::in)
    , size_(0)
{
    if (!stream_) {
        throw std::runtime_error("Failed to open file: " + filepath);
    }

    // Get file size
    stream_.seekg(0, std::ios::end);
    size_ = stream_.tellg();
    stream_.seekg(0, std::ios::beg);
}

BinaryReader::BinaryReader(std::ifstream&& stream)
    : stream_(std::move(stream))
    , size_(0)
{
    if (!stream_) {
        throw std::runtime_error("Invalid stream provided");
    }

    // Get stream size
    auto current = stream_.tellg();
    stream_.seekg(0, std::ios::end);
    size_ = stream_.tellg();
    stream_.seekg(current);
}

BinaryReader::~BinaryReader() {
    if (stream_.is_open()) {
        stream_.close();
    }
}

bool BinaryReader::IsOpen() const {
    return stream_.is_open();
}

bool BinaryReader::IsEOF() {
    return stream_.eof();
}

bool BinaryReader::IsGood() {
    return stream_.good();
}

size_t BinaryReader::Tell() {
    return stream_.tellg();
}

void BinaryReader::Seek(size_t position) {
    stream_.seekg(position);
}

void BinaryReader::Skip(size_t bytes) {
    stream_.seekg(bytes, std::ios::cur);
}

size_t BinaryReader::Size() const {
    return size_;
}

void BinaryReader::ReadBytes(void* buffer, size_t count) {
    stream_.read(static_cast<char*>(buffer), count);
    if (!stream_.good() && !stream_.eof()) {
        throw std::runtime_error("Failed to read bytes from stream");
    }
}

std::vector<uint8> BinaryReader::ReadBytes(size_t count) {
    std::vector<uint8> buffer(count);
    ReadBytes(buffer.data(), count);
    return buffer;
}

// Primitive reads - little-endian
template<typename T>
T BinaryReader::ReadLittleEndian() {
    uint8 bytes[sizeof(T)];
    ReadBytes(bytes, sizeof(T));

    T value = 0;
    for (size_t i = 0; i < sizeof(T); ++i) {
        value |= static_cast<T>(bytes[i]) << (i * 8);
    }
    return value;
}

uint8 BinaryReader::ReadUInt8() {
    uint8 value;
    ReadBytes(&value, 1);
    return value;
}

uint16 BinaryReader::ReadUInt16() {
    return ReadLittleEndian<uint16>();
}

uint32 BinaryReader::ReadUInt32() {
    return ReadLittleEndian<uint32>();
}

uint64 BinaryReader::ReadUInt64() {
    return ReadLittleEndian<uint64>();
}

int8 BinaryReader::ReadInt8() {
    return static_cast<int8>(ReadUInt8());
}

int16 BinaryReader::ReadInt16() {
    return static_cast<int16>(ReadUInt16());
}

int32 BinaryReader::ReadInt32() {
    return static_cast<int32>(ReadUInt32());
}

int64 BinaryReader::ReadInt64() {
    return static_cast<int64>(ReadUInt64());
}

float BinaryReader::ReadFloat() {
    uint32 bits = ReadUInt32();
    float value;
    std::memcpy(&value, &bits, sizeof(float));
    return value;
}

double BinaryReader::ReadDouble() {
    uint64 bits = ReadUInt64();
    double value;
    std::memcpy(&value, &bits, sizeof(double));
    return value;
}

bool BinaryReader::ReadBool() {
    return ReadUInt8() != 0;
}

// Variable Length Quantity (VLQ) decoding
// Format: 7 bits of data per byte, MSB is continuation bit
int32 BinaryReader::ReadVLQInt32() {
    return static_cast<int32>(ReadVLQUInt32());
}

uint32 BinaryReader::ReadVLQUInt32() {
    uint32 value = 0;
    uint32 shift = 0;
    uint8 byte;

    do {
        byte = ReadUInt8();
        value |= static_cast<uint32>(byte & 0x7F) << shift;
        shift += 7;

        // Prevent infinite loop on malformed data
        if (shift > 32) {
            throw std::runtime_error("VLQ decoding exceeded 32 bits");
        }
    } while (byte & 0x80); // Continue if MSB is set

    return value;
}

// String reading
String BinaryReader::ReadString(size_t length) {
    if (length == 0) {
        return String();
    }

    std::vector<char> buffer(length);
    ReadBytes(buffer.data(), length);
    return String(buffer.data(), length);
}

String BinaryReader::ReadVLQString() {
    uint32 length = ReadVLQUInt32();
    return ReadString(length);
}

String BinaryReader::ReadNullTerminatedString() {
    String result;
    char c;

    while (true) {
        ReadBytes(&c, 1);
        if (c == '\0') {
            break;
        }
        result += c;

        // Prevent infinite loop on malformed data
        if (result.length() > 65536) {
            throw std::runtime_error("Null-terminated string exceeded maximum length");
        }
    }

    return result;
}

} // namespace TweakXL
