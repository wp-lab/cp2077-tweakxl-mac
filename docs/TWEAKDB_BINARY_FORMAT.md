# TweakDB Binary Format Specification

**Version**: Blob 8, Parser 4
**Game**: Cyberpunk 2077 (REDengine 4)
**File Location**: `Cyberpunk 2077/r6/cache/tweakdb.bin`

## Overview

TweakDB is a proprietary binary database format used by REDengine 4 to store game configuration data. The file contains four main sections: Flats (key-value pairs), Records (typed objects), Queries (result sets), and Group Tags (categorizations).

## File Structure

```
[Magic Number]      4 bytes
[Header]           28 bytes
[Flats Section]    Variable
[Records Section]  Variable
[Queries Section]  Variable
[Group Tags]       Variable
```

## Magic Number

**Value**: `0x0BB1DB47`
**Purpose**: File format identification and validation

## Header Structure (28 bytes)

| Offset | Size | Type   | Field           | Description                    |
|--------|------|--------|-----------------|--------------------------------|
| 0      | 4    | int32  | blobVersion     | Binary format version (8)      |
| 4      | 4    | int32  | parserVersion   | Parser compatibility (4)       |
| 8      | 4    | uint32 | recordChecksum  | CRC32 checksum for validation  |
| 12     | 4    | int32  | flatsOffset     | File offset to Flats section   |
| 16     | 4    | int32  | recordsOffset   | File offset to Records section |
| 20     | 4    | int32  | queriesOffset   | File offset to Queries section |
| 24     | 4    | int32  | groupTagsOffset | File offset to Group Tags      |

**Endianness**: Little-endian
**Alignment**: Fields are explicitly positioned (not padded)

## TweakDBID

TweakDBID is the primary identifier type used throughout TweakDB. It's a 64-bit composite value.

### Hash Calculation

```
TweakDBID = CRC32(lowercase_name) + (name.length << 32)
```

**Components**:
- **Lower 32 bits**: CRC32 hash of the lowercase name string
- **Upper 32 bits**: Length of the original name string (must be ≤ 255)

### CRC32 Algorithm

- **Algorithm**: CRC32 (standard polynomial 0x04C11DB7)
- **Input**: UTF-8 encoded string, converted to lowercase
- **Output**: 32-bit unsigned integer

### Examples

```
Name: "BaseStats.Health"
Length: 18 (0x12)
CRC32("basestats.health"): 0xABCD1234 (example)
TweakDBID: 0x0000001200ABCD1234 (64-bit)
```

### Constraints

- Maximum name length: 255 characters
- Names are case-insensitive (converted to lowercase before hashing)
- Zero hash (0x0) represents an invalid/null ID

## Flats Section

Flats are individual key-value pairs. They can exist independently ("free flats") or as properties of records.

### Section Structure

```
[uint32: count]           // Number of flat type groups
For each type group:
  [type metadata]         // Type information
  [uint32: value_count]   // Number of values
  [values array]          // Actual values
  [uint32: key_count]     // Number of keys
  [TweakDBID array]       // Keys for each value
```

### Data Types

Flats support the following types (matching Phase 1 TypeID enum):

#### Primitive Types
- **Bool**: 1 byte (0 = false, 1 = true)
- **Int8, Int16, Int32, Int64**: Signed integers
- **Uint8, Uint16, Uint32, Uint64**: Unsigned integers
- **Float**: 32-bit IEEE 754
- **Double**: 64-bit IEEE 754
- **String**: VLQ length + UTF-8 bytes

#### Special Types
- **CName**: uint64 hash + optional string table reference
- **TweakDBID**: uint64 composite hash (as described above)
- **LocKey**: uint64 localization key
- **CResource**: String path to game resource

#### Geometric Types
- **Vector2**: 2× float (X, Y)
- **Vector3**: 3× float (X, Y, Z)
- **Vector4**: 4× float (X, Y, Z, W)
- **EulerAngles**: 3× float (Pitch, Yaw, Roll)
- **Quaternion**: 4× float (I, J, K, R)
- **Color**: uint32 RGBA

#### Container Types
- **Array**: VLQ count + elements
  - Format: `[element_type][count][element0][element1]...`
  - Arrays can contain any supported type

### Variable Length Quantity (VLQ) Encoding

Used for counts and lengths to save space.

**Encoding**:
- 7 bits of data per byte
- MSB (bit 7) = continuation bit (1 = more bytes follow)
- LSB 7 bits = data

**Example**:
- Value 127: `0x7F` (1 byte)
- Value 128: `0x80 0x01` (2 bytes)
- Value 300: `0xAC 0x02` (2 bytes)

### Key-Value Organization

The flats section uses a dual-array approach:
- **Values array**: Stores actual values sequentially
- **Keys array**: Stores TweakDBID for each value (same index)

This allows efficient lookup by ID and iteration over all values.

## Records Section

Records are typed collections of properties (flats). They represent game entities like weapons, characters, stats, etc.

### Section Structure

```
[uint32: count]              // Number of records
For each record:
  [TweakDBID: record_id]     // Unique record identifier
  [uint32: type_hash]        // Record type (Murmur3 hash)
```

### Type Hashing

Record types use **Murmur3-32** hash algorithm with seed `0x5EEDBA5E`.

**Example Type Names**:
- `Items.Weapon_Base`
- `Character.Johnny_Record`
- `BaseStats.Health_Record`

### Record-Flat Relationship

Records reference their properties through naming convention:
```
Record: "Items.Katana"
Flats:  "Items.Katana.damage"
        "Items.Katana.range"
        "Items.Katana.price"
```

The parser can reconstruct a record by finding all flats that start with the record's name followed by a period.

### Inheritance

Records can inherit from other records:
- Base record properties are merged with derived record
- Derived properties override base properties
- Inheritance chain resolved at load time

## Queries Section

Queries are named collections of TweakDBID references (result sets).

### Section Structure

```
[uint32: count]              // Number of queries
For each query:
  [CName: query_name]        // Query identifier (hashed name)
  [uint32: result_count]     // Number of results
  [TweakDBID array]          // Array of record IDs
```

### Purpose

Queries provide pre-computed collections for efficient game lookups:
- All weapons of a certain type
- NPCs in a specific faction
- Available cyberware mods

## Group Tags Section

Group tags categorize records with boolean flags.

### Section Structure

```
[uint32: count]              // Number of group tags
For each tag:
  [CName: tag_name]          // Tag identifier (hashed name)
  [uint32: record_count]     // Number of tagged records
  [TweakDBID array]          // Array of tagged record IDs
```

### Purpose

Efficient filtering and categorization:
- "Legendary" items
- "Hostile" NPCs
- "Craftable" recipes

## Type System Reference

### Type Hash Table (FNV1A64)

WolvenKit uses FNV1A64 for some internal type lookups:

```
FNV_OFFSET_BASIS = 0xcbf29ce484222325
FNV_PRIME = 0x100000001b3

hash = FNV_OFFSET_BASIS
for each byte in string:
    hash ^= byte
    hash *= FNV_PRIME
```

This is used for type name → TypeID lookups internally, but NOT for TweakDBID calculation.

## Implementation Notes

### Parsing Order

1. Read and validate magic number
2. Read header structure
3. Validate version compatibility
4. Seek to flatsOffset and parse Flats
5. Seek to recordsOffset and parse Records
6. Seek to queriesOffset and parse Queries
7. Seek to groupTagsOffset and parse Group Tags

### Error Handling

- Invalid magic number → reject file
- Version mismatch → warn or reject
- Checksum mismatch → warn (may still parse)
- Offset beyond EOF → corruption error
- Invalid VLQ encoding → parsing error

### Performance Considerations

- Use hash tables for O(1) lookups
- Pre-allocate arrays based on counts
- Stream reading for large files
- Lazy loading for unused sections

## Compatibility Notes

### macOS Considerations

- File paths use forward slashes on macOS
- Endianness is same (little-endian on x86_64/ARM64)
- No platform-specific binary format differences

### Version Evolution

Current documented version: Blob 8, Parser 4

Future versions may:
- Add new data types
- Extend header with additional offsets
- Introduce compression
- Add signature/encryption

## References

- **WolvenKit**: https://github.com/WolvenKit/WolvenKit
  - `WolvenKit.RED4/TweakDB/Header.cs` - Header structure
  - `WolvenKit.RED4/TweakDB/TweakDBReader.cs` - Binary parser
  - `WolvenKit.RED4/TweakDB/FlatsPool.cs` - Flats management

- **010 Editor Template**: https://github.com/flibdev/010_CP77_TweakDB
  - Detailed binary structure documentation

- **TweakDB-Edit**: https://github.com/AlpyneDreams/TweakDB-Edit
  - JavaScript parser implementation

- **Gibbed TweakDB Schema**: https://github.com/gibbed/Cyberpunk-TweakDB-Schema
  - Type definitions and schemas

## Revision History

- **2025-11-17**: Initial documentation based on WolvenKit analysis
- **Version**: Cyberpunk 2077 v2.1+ (current format)
