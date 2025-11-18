#pragma once
#include "../core/Types.hpp"

namespace TweakXL {

/**
 * TweakDB file header structure (28 bytes)
 *
 * Magic number: 0x0BB1DB47
 * Blob version: 8
 * Parser version: 4
 */
struct TweakDBHeader {
    int32 blobVersion;      // Binary format version (expect: 8)
    int32 parserVersion;    // Parser compatibility version (expect: 4)
    uint32 recordChecksum;  // CRC32 checksum for validation
    int32 flatsOffset;      // File offset to Flats section
    int32 recordsOffset;    // File offset to Records section
    int32 queriesOffset;    // File offset to Queries section
    int32 groupTagsOffset;  // File offset to Group Tags section

    // Constants
    static constexpr uint32 MAGIC = 0x0BB1DB47;
    static constexpr int32 EXPECTED_BLOB_VERSION = 8;
    static constexpr int32 EXPECTED_PARSER_VERSION = 4;
    static constexpr uint32 RECORDS_SEED = 0x5EEDBA5E;

    // Validation
    bool IsValid() const {
        return blobVersion == EXPECTED_BLOB_VERSION &&
               parserVersion == EXPECTED_PARSER_VERSION;
    }

    bool IsCompatible() const {
        // May want to support older versions in the future
        return blobVersion <= EXPECTED_BLOB_VERSION &&
               parserVersion <= EXPECTED_PARSER_VERSION;
    }
};

} // namespace TweakXL
