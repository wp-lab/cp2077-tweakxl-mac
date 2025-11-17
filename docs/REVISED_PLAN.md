# TweakXL-macOS: Revised Implementation Plan

**Version:** 2.0 (Research-Based)
**Date:** November 2025
**Status:** Planning → Implementation

---

## Context

This plan is based on comprehensive research into:
- Cyberpunk 2077's native macOS port (released July 2025)
- TweakDB binary format and modding ecosystem
- TweakXL's Windows implementation (runtime hooking)
- Development environment constraints (Linux, no game access)

See [RESEARCH_FINDINGS.md](RESEARCH_FINDINGS.md) for detailed background.

---

## Mission Statement

> **Build an offline TweakDB patcher for macOS** that enables existing Windows TweakXL mods to work on macOS Cyberpunk 2077 with minimal changes, using a pre-game patching approach instead of runtime hooking.

### What We're Building

**TweakXL-macOS** (working name)
- **Type:** Command-line tool (CLI)
- **Approach:** Offline binary patching
- **Input:** TweakDB binary + mod files (YAML/.tweak)
- **Output:** Modified TweakDB binary
- **Target:** macOS Cyberpunk 2077: Ultimate Edition

### Why Offline Patching?

Windows TweakXL uses **runtime hooking** via RED4ext (DLL injection):
- ❌ RED4ext is Windows-only
- ❌ No macOS equivalent exists
- ❌ Game internals may differ in Metal 3 port

Offline patching:
- ✅ Platform-independent approach
- ✅ Works without game modifications
- ✅ Safer (patches applied before game launch)
- ✅ Easy to revert (backup/restore)

---

## Development Environment Reality Check

### What We Have

**Platform:** Linux x86_64 (containerized development environment)
- Compilers: Clang 18, GCC 13
- Build Tools: CMake 3.28.3, Git, Python 3.11
- Can write C++ cross-platform code

### What We Don't Have

- ❌ macOS machine (can't test on target platform)
- ❌ Cyberpunk 2077 game (can't access tweakdb.bin)
- ❌ Real mod files (for initial testing)

### Implications

**We must:**
1. Study existing open-source parsers (WolvenKit, Gibbed)
2. Work from documentation and schema files
3. Create synthetic test data
4. Rely on community for beta testing

**We cannot:**
- Directly reverse-engineer binary format
- Test against real game until late stages
- Verify in-game behavior ourselves

---

## Technical Architecture

### Component Overview

```
┌─────────────────────────────────────────────────────────────┐
│                      TweakXL-macOS CLI                       │
└─────────────────────────────────────────────────────────────┘
                              │
                              ↓
        ┌─────────────────────────────────────────────┐
        │           Mod File Parsers                  │
        │  ┌──────────────┐    ┌──────────────┐      │
        │  │ YAML Parser  │    │.tweak Parser │      │
        │  │  (yaml-cpp)  │    │   (PEGTL)    │      │
        │  └──────────────┘    └──────────────┘      │
        └─────────────────────────────────────────────┘
                              │
                              ↓
        ┌─────────────────────────────────────────────┐
        │         Modification Representation         │
        │      (TweakSource, Operations, Values)      │
        └─────────────────────────────────────────────┘
                              │
                              ↓
┌──────────────────┐          │          ┌──────────────────┐
│ TweakDB Reader   │◄─────────┼─────────►│ TweakDB Writer   │
│                  │          │          │                  │
│ - Parse header   │          │          │ - Rebuild binary │
│ - Extract flats  │          │          │ - Write sections │
│ - Extract records│          │          │ - Update offsets │
└──────────────────┘          │          └──────────────────┘
                              ↓
                  ┌─────────────────────────┐
                  │    Bridge / Applicator   │
                  │                         │
                  │ - Apply modifications   │
                  │ - Type coercion         │
                  │ - Validation            │
                  └─────────────────────────┘
                              │
                              ↓
              ┌───────────────────────────────┐
              │     In-Memory TweakDB         │
              │                               │
              │ - Flats (ID → Value)          │
              │ - Records (ID → Properties)   │
              │ - Type metadata               │
              └───────────────────────────────┘
```

### Core Components

#### 1. TweakDB Binary Parser

**Responsibility:** Read `tweakdb.bin` into memory

**Implementation:**
```cpp
class TweakDBReader {
public:
    TweakDB Load(const std::filesystem::path& path);

private:
    Header ReadHeader(std::istream& stream);
    std::vector<Flat> ReadFlats(std::istream& stream, const Header& header);
    std::vector<Record> ReadRecords(std::istream& stream, const Header& header);
};
```

**Key Structures:**
```cpp
struct Header {
    uint32_t magic;           // 0x0BB1DB47
    uint32_t version;
    uint32_t flatsOffset;
    uint32_t flatsCount;
    uint32_t recordsOffset;
    uint32_t recordsCount;
    // ... more fields
};

struct Flat {
    TweakDBID id;             // 64-bit hash
    TypeID type;              // Type identifier
    std::variant<...> value;  // Actual value
};

struct Record {
    TweakDBID id;
    TypeID type;
    std::map<std::string, Flat> properties;
};
```

#### 2. Mod File Parsers

**YAML Parser:**
```cpp
class YAMLTweakParser {
public:
    TweakSource Parse(const std::filesystem::path& path);

private:
    void ParseFlats(const YAML::Node& node, TweakSource& source);
    void ParseRecords(const YAML::Node& node, TweakSource& source);
    Value ParseValue(const YAML::Node& node);
};
```

**.tweak Parser (PEGTL):**
```cpp
class TweakFileParser {
public:
    TweakSource Parse(const std::filesystem::path& path);

private:
    // Use PEGTL grammar from TweakXL
    // Build AST, convert to TweakSource
};
```

#### 3. In-Memory Database Model

```cpp
class TweakDB {
public:
    // Query
    std::optional<Flat> GetFlat(TweakDBID id) const;
    std::optional<Record> GetRecord(TweakDBID id) const;

    // Modify
    void SetFlat(TweakDBID id, Value value);
    void SetRecordProperty(TweakDBID recordId, const std::string& prop, Value value);
    void AppendToArray(TweakDBID id, Value value);
    void RemoveFromArray(TweakDBID id, Value value);

    // Metadata
    TypeInfo GetTypeInfo(TypeID type) const;

private:
    std::unordered_map<TweakDBID, Flat> flats_;
    std::unordered_map<TweakDBID, Record> records_;
    TypeRegistry typeRegistry_;
};
```

#### 4. Bridge / Applicator

```cpp
class ModificationApplicator {
public:
    void Apply(TweakDB& db, const TweakSource& source);

private:
    void ApplyFlat(TweakDB& db, const TweakFlat& flat);
    void ApplyRecord(TweakDB& db, const TweakRecord& record);
    Value CoerceType(const Value& value, TypeID targetType);
};
```

#### 5. TweakDB Binary Writer

```cpp
class TweakDBWriter {
public:
    void Save(const TweakDB& db, const std::filesystem::path& path);

private:
    void WriteHeader(std::ostream& stream, const TweakDB& db);
    void WriteFlats(std::ostream& stream, const TweakDB& db);
    void WriteRecords(std::ostream& stream, const TweakDB& db);
    void UpdateOffsets(std::ostream& stream);
};
```

---

## Phase Breakdown

### Phase 1: Foundation (Weeks 1-2)

**Goal:** Set up project, understand formats, create test infrastructure

#### Tasks

1. **Project Setup**
   - [ ] Create CMakeLists.txt
   - [ ] Configure dependencies (yaml-cpp, PEGTL, spdlog, CLI11)
   - [ ] Set up directory structure
   - [ ] Create basic README

2. **Binary Format Research**
   - [ ] Study WolvenKit TweakDB parser (C# source)
   - [ ] Document header structure in detail
   - [ ] Document flat storage format
   - [ ] Document record storage format
   - [ ] Create `docs/tweakdb-binary-format.md`

3. **Test Data Creation**
   - [ ] Create minimal synthetic tweakdb.bin (hex editor)
   - [ ] Create test YAML tweak files
   - [ ] Create test .tweak files
   - [ ] Set up test fixtures

4. **Core Data Structures**
   - [ ] Implement `TweakDBID` class (64-bit hash)
   - [ ] Implement `Value` variant type
   - [ ] Implement `Flat` and `Record` structs
   - [ ] Write unit tests

#### Deliverables

- ✅ CMake project that builds
- ✅ Dependencies integrated
- ✅ `docs/tweakdb-binary-format.md` (draft)
- ✅ Synthetic test data (minimal tweakdb.bin)
- ✅ Core data structures with tests

#### Success Criteria

- Project builds without errors
- Tests pass (even if trivial)
- Have documented understanding of binary format
- Can create and parse test data

---

### Phase 2: TweakDB Reader (Weeks 3-4)

**Goal:** Parse real tweakdb.bin files into memory

#### Tasks

1. **Header Parser**
   - [ ] Implement `ReadHeader()` function
   - [ ] Validate magic number
   - [ ] Extract offsets and counts
   - [ ] Write tests

2. **Flats Parser**
   - [ ] Locate flats section
   - [ ] Parse TweakDBID
   - [ ] Parse type information
   - [ ] Parse values (start with simple types: int, float, bool, string)
   - [ ] Handle arrays
   - [ ] Write tests

3. **Records Parser**
   - [ ] Locate records section
   - [ ] Parse record headers
   - [ ] Parse properties
   - [ ] Link to flats
   - [ ] Write tests

4. **Integration**
   - [ ] Implement `TweakDBReader::Load()`
   - [ ] Build `TweakDB` in-memory model
   - [ ] Create dump tool (`tweakdb-dump` CLI)
   - [ ] Test with synthetic data
   - [ ] Test with real tweakdb.bin (if available)

#### Deliverables

- ✅ `TweakDBReader` class fully implemented
- ✅ `TweakDB` in-memory model
- ✅ `tweakdb-dump` tool (dump to JSON/text)
- ✅ Comprehensive tests
- ✅ Works with synthetic data

#### Success Criteria

- Can parse synthetic tweakdb.bin without errors
- Can dump flats and records to human-readable format
- Tests cover edge cases (empty arrays, missing types, etc.)
- (Stretch) Can parse real game tweakdb.bin

---

### Phase 3: Mod File Parsers (Weeks 5-6)

**Goal:** Parse YAML and .tweak mod files

#### Tasks

1. **YAML Parser - Basic**
   - [ ] Integrate yaml-cpp
   - [ ] Parse flat modifications (`key: value`)
   - [ ] Parse record modifications
   - [ ] Handle basic types (int, float, bool, string)
   - [ ] Write tests

2. **YAML Parser - Advanced**
   - [ ] Parse special types (CName, LocKey, Resource, etc.)
   - [ ] Parse arrays
   - [ ] Parse array operations (`!append`, `!remove`)
   - [ ] Parse inline records
   - [ ] Write tests

3. **.tweak Parser**
   - [ ] Integrate PEGTL library
   - [ ] Port grammar from TweakXL (`Grammar.hpp`)
   - [ ] Implement parser actions (build AST)
   - [ ] Convert AST to `TweakSource`
   - [ ] Write tests

4. **Mod Loader**
   - [ ] Scan mods directory
   - [ ] Load all .yaml/.yml files
   - [ ] Load all .tweak files
   - [ ] Handle subdirectories
   - [ ] Merge into single modification set
   - [ ] Write tests

#### Deliverables

- ✅ `YAMLTweakParser` class
- ✅ `TweakFileParser` class
- ✅ `TweakSource` representation
- ✅ `ModLoader` class
- ✅ Comprehensive tests with sample mod files

#### Success Criteria

- Can parse NIGHT CITY ALIVE .tweak files
- Can parse sample YAML tweaks
- All operations represented correctly (assign, append, remove)
- Tests cover complex cases (nested records, arrays, special types)

---

### Phase 4: Bridge & Application Logic (Weeks 7-8)

**Goal:** Apply parsed mods to TweakDB

#### Tasks

1. **Applicator Core**
   - [ ] Implement `ModificationApplicator::Apply()`
   - [ ] Apply flat assignments
   - [ ] Apply record property changes
   - [ ] Write tests

2. **Array Operations**
   - [ ] Implement append operations
   - [ ] Implement remove operations
   - [ ] Handle append-once (deduplication)
   - [ ] Write tests

3. **Type System**
   - [ ] Implement type coercion (string → CName, etc.)
   - [ ] Handle special types (LocKey, Resource, etc.)
   - [ ] Validate type compatibility
   - [ ] Write tests

4. **Validation & Error Handling**
   - [ ] Warn on missing flats/records
   - [ ] Warn on type mismatches
   - [ ] Provide helpful error messages
   - [ ] Write tests

#### Deliverables

- ✅ `ModificationApplicator` class
- ✅ Type coercion system
- ✅ Validation and error reporting
- ✅ Comprehensive tests

#### Success Criteria

- Can apply simple modifications (flat assignments)
- Can modify record properties
- Can append/remove from arrays
- Type coercion works correctly
- Errors are clear and helpful

---

### Phase 5: TweakDB Writer (Weeks 9-10)

**Goal:** Write modified TweakDB back to binary format

#### Tasks

1. **Binary Writer Core**
   - [ ] Implement `TweakDBWriter::Save()`
   - [ ] Write header with correct offsets
   - [ ] Calculate section sizes
   - [ ] Write tests (round-trip: read → write → read)

2. **Flats Writer**
   - [ ] Serialize TweakDBID
   - [ ] Serialize type information
   - [ ] Serialize values (all supported types)
   - [ ] Handle arrays
   - [ ] Write tests

3. **Records Writer**
   - [ ] Serialize record headers
   - [ ] Serialize properties
   - [ ] Maintain references to flats
   - [ ] Write tests

4. **Validation**
   - [ ] Verify offsets are correct
   - [ ] Verify counts match
   - [ ] Verify no data corruption
   - [ ] Round-trip tests (read → write → read → compare)

#### Deliverables

- ✅ `TweakDBWriter` class
- ✅ Round-trip tests pass
- ✅ Can write modified TweakDB
- ✅ Binary format validation

#### Success Criteria

- Read → Write → Read produces identical database
- Modified database has changes applied
- Binary format matches original structure
- Game accepts modified tweakdb.bin (test with community)

---

### Phase 6: CLI & Integration (Weeks 11-12)

**Goal:** Create user-friendly command-line tool

#### Tasks

1. **CLI Interface**
   - [ ] Integrate CLI11 or cxxopts
   - [ ] Implement argument parsing
   - [ ] Add help text
   - [ ] Write usage examples

2. **Main Workflow**
   - [ ] `--game-dir` - Locate game and TweakDB
   - [ ] `--mods-dir` - Find mod files
   - [ ] Load TweakDB
   - [ ] Load all mods
   - [ ] Apply modifications
   - [ ] Write output
   - [ ] Report summary

3. **Safety Features**
   - [ ] `--backup` - Auto-backup original TweakDB
   - [ ] `--restore` - Restore from backup
   - [ ] `--dry-run` - Preview without writing
   - [ ] Backup before overwriting

4. **User Experience**
   - [ ] Progress indicators (if slow)
   - [ ] Colorized output (if supported)
   - [ ] Clear error messages
   - [ ] Success/failure summary

5. **Configuration**
   - [ ] Optional config file (YAML/JSON)
   - [ ] Default paths (common game locations)
   - [ ] Mod load order (if needed)

#### Deliverables

- ✅ `tweakxl-mac` CLI binary
- ✅ User-friendly interface
- ✅ Safety features (backup/restore)
- ✅ Example usage in README

#### Success Criteria

- Tool runs end-to-end:
  1. Reads game TweakDB
  2. Loads mods from directory
  3. Applies modifications
  4. Writes modded TweakDB
- Backup/restore works
- Dry-run shows what would change
- Help text is clear

---

### Phase 7: Testing & Validation (Weeks 13-14)

**Goal:** Test with real mods and game

#### Tasks

1. **Obtain Test Data**
   - [ ] Get real tweakdb.bin (from community or game)
   - [ ] Get NIGHT CITY ALIVE mod files
   - [ ] Get 2-3 other popular mods

2. **Functional Testing**
   - [ ] Run tool on each test mod
   - [ ] Verify output TweakDB generated
   - [ ] Check for errors/warnings
   - [ ] Inspect output with `tweakdb-dump`

3. **In-Game Testing** (requires macOS + game)
   - [ ] Replace tweakdb.bin in game
   - [ ] Launch game
   - [ ] Verify no crashes
   - [ ] Check expected behavior (e.g., gang density in NIGHT CITY ALIVE)
   - [ ] Compare with Windows TweakXL results

4. **Compatibility Matrix**
   - [ ] Test variety of mod types
   - [ ] Document what works / what doesn't
   - [ ] Create `docs/compatibility-report.md`

5. **Bug Fixing**
   - [ ] Fix issues found during testing
   - [ ] Iterate until stable

#### Deliverables

- ✅ Tested with 3+ real mods
- ✅ `docs/compatibility-report.md`
- ✅ Bug fixes and improvements
- ✅ v1.0 release candidate

#### Success Criteria

- ✅ NIGHT CITY ALIVE works correctly
- ✅ At least 2 other mods work
- ✅ Game doesn't crash
- ✅ In-game behavior matches expectations
- ✅ Known issues documented

---

### Phase 8: Documentation & Release (Week 15)

**Goal:** Polish for public release

#### Tasks

1. **User Documentation**
   - [ ] Write `docs/user-guide.md`
   - [ ] Installation instructions
   - [ ] Usage examples
   - [ ] Troubleshooting guide
   - [ ] FAQ

2. **Developer Documentation**
   - [ ] Architecture overview
   - [ ] Building from source
   - [ ] Contributing guidelines
   - [ ] Code style guide

3. **Repository Polish**
   - [ ] Clean up README
   - [ ] Add badges (build status, license, etc.)
   - [ ] Create LICENSE file (choose MIT or similar)
   - [ ] Add CHANGELOG
   - [ ] Tag v1.0.0 release

4. **Distribution**
   - [ ] Build releases for macOS (if possible)
   - [ ] Create GitHub release with binaries
   - [ ] Write release notes
   - [ ] Announce to community (Reddit, Nexus Mods?)

#### Deliverables

- ✅ Comprehensive documentation
- ✅ v1.0.0 tagged release
- ✅ Binaries (if possible)
- ✅ Community announcement

#### Success Criteria

- Documentation is clear and complete
- New users can install and use without confusion
- Developers can build from source
- Release is publicly available

---

## Technology Stack (Finalized)

### Language

**C++17** (or C++20 if needed for specific features)

### Build System

**CMake 3.15+**
- Cross-platform (Linux, macOS, Windows)
- Standard in C++ community
- Good dependency management (FetchContent)

### Core Libraries

| Library | Purpose | License | Integration |
|---------|---------|---------|-------------|
| **yaml-cpp** | YAML parsing | MIT | CMake FetchContent |
| **PEGTL** | .tweak parser | MIT | Header-only |
| **spdlog** | Logging | MIT | CMake FetchContent |
| **CLI11** | Command-line parsing | BSD-3 | Header-only |
| **nlohmann/json** | JSON (metadata) | MIT | Header-only |

### Testing

**Catch2** - Unit testing framework

### Optional

- **{fmt}** - String formatting (if not using C++20 std::format)
- **backward-cpp** - Stack traces for debugging

---

## Project Structure

```
cp2077-tweakxl-mac/
├── CMakeLists.txt              # Root build config
├── README.md                   # Project overview
├── LICENSE                     # MIT license
├── CHANGELOG.md                # Version history
│
├── cmake/                      # CMake modules
│   └── Dependencies.cmake      # Fetch dependencies
│
├── docs/                       # Documentation
│   ├── RESEARCH_FINDINGS.md    # This document
│   ├── REVISED_PLAN.md         # Implementation plan
│   ├── tweakdb-binary-format.md # Binary format spec
│   ├── tweak-formats.md        # Mod file formats
│   ├── user-guide.md           # User documentation
│   ├── api-reference.md        # Developer docs
│   └── compatibility-report.md # Tested mods
│
├── src/                        # Source code
│   ├── CMakeLists.txt
│   │
│   ├── core/                   # Core data structures
│   │   ├── TweakDBID.hpp/cpp
│   │   ├── Value.hpp/cpp
│   │   ├── Flat.hpp/cpp
│   │   └── Record.hpp/cpp
│   │
│   ├── tweakdb/                # TweakDB binary I/O
│   │   ├── Reader.hpp/cpp
│   │   ├── Writer.hpp/cpp
│   │   ├── Database.hpp/cpp
│   │   └── Header.hpp
│   │
│   ├── parsers/                # Mod file parsers
│   │   ├── YAMLParser.hpp/cpp
│   │   ├── TweakParser.hpp/cpp
│   │   ├── TweakGrammar.hpp    # PEGTL grammar
│   │   └── ModLoader.hpp/cpp
│   │
│   ├── model/                  # Modification representation
│   │   ├── TweakSource.hpp/cpp
│   │   ├── Operation.hpp/cpp
│   │   └── TypeInfo.hpp/cpp
│   │
│   ├── bridge/                 # Apply modifications
│   │   ├── Applicator.hpp/cpp
│   │   ├── TypeCoercion.hpp/cpp
│   │   └── Validator.hpp/cpp
│   │
│   └── cli/                    # Command-line interface
│       ├── main.cpp
│       ├── Commands.hpp/cpp
│       └── Config.hpp/cpp
│
├── tests/                      # Unit tests
│   ├── CMakeLists.txt
│   ├── core/
│   ├── tweakdb/
│   ├── parsers/
│   ├── bridge/
│   └── integration/
│
├── data/                       # Metadata and test data
│   ├── schema/                 # TweakXL metadata (JSON)
│   │   ├── ExtraFlats.json
│   │   └── InheritanceMap.json
│   ├── test/                   # Test fixtures
│   │   ├── minimal.tweakdb.bin
│   │   ├── sample.yaml
│   │   └── sample.tweak
│   └── examples/               # Example mod files
│
├── scripts/                    # Build and utility scripts
│   ├── setup-dev.sh            # Set up dev environment
│   ├── build.sh                # Build script
│   └── test.sh                 # Run tests
│
└── third_party/                # Vendored dependencies (if needed)
    └── (empty - use CMake FetchContent)
```

---

## Compatibility Scope

### Supported Features (v1.0)

#### ✅ File Formats
- YAML tweaks (`.yaml`, `.yml`)
- .tweak flat files

#### ✅ Operations
- Assign (`=`) - Set flat or record property value
- Append (`+=`, `!append`) - Add to array
- Remove (`-=`, `!remove`) - Remove from array

#### ✅ Data Types
- Primitives: int, float, bool, string
- Special: CName, TweakDBID, LocKey, Resource
- Collections: Arrays

#### ✅ Mod Features
- Flat modifications
- Record property modifications
- Array operations (append, remove)
- Basic type coercion

### Limited Support (v1.0)

#### ⚠️ Partial Implementation
- Record cloning (`$base`) - If time permits
- Record creation (new records) - Simple cases only
- Templates (`$instances`) - Best effort

### Not Supported (v1.0)

#### ❌ Out of Scope
- RED tweak files (`.reds` format) - Different from .tweak
- Script extensions (redscript integration)
- Hot reloading (requires runtime)
- Dynamic queries (requires runtime)
- Record inheritance (complex cases)

---

## Testing Strategy

### Unit Tests

**Framework:** Catch2

**Coverage:**
- Core data structures (TweakDBID, Value, Flat, Record)
- Binary parser (header, flats, records)
- Mod parsers (YAML, .tweak)
- Type coercion
- Modification application

**Approach:**
- Test-driven development where possible
- Use synthetic test data
- Mock complex dependencies

### Integration Tests

**Scenarios:**
1. **Round-trip:** Read → Write → Read (should be identical)
2. **Apply mods:** Read → Apply → Write → Read (should have changes)
3. **Multiple mods:** Apply multiple mod files, verify merge
4. **Edge cases:** Empty arrays, missing flats, type mismatches

### Functional Tests

**Real mods:**
- NIGHT CITY ALIVE
- Simple weapon stat mod
- YAML-based mod

**Process:**
1. Load real tweakdb.bin
2. Apply mod
3. Inspect output
4. (Ideally) Test in game

### Community Beta Testing

**Once tool is functional:**
- Release alpha to volunteers
- Request testing on macOS with actual game
- Gather bug reports
- Iterate

---

## Risks & Mitigation

### Technical Risks

| Risk | Impact | Probability | Mitigation |
|------|--------|-------------|------------|
| Binary format too complex | High | Medium | Start simple, iterate; use WolvenKit as reference |
| No test data | High | Low | Community can provide; create synthetic |
| Type system issues | Medium | Medium | Support common types first; expand gradually |
| macOS differences | Medium | Low | Document assumptions; adapt when issues found |
| Performance problems | Low | Low | Profile and optimize as needed |

### Project Risks

| Risk | Impact | Probability | Mitigation |
|------|--------|-------------|------------|
| Scope creep | Medium | High | Stick to plan; defer features to v2 |
| Time overrun | Medium | Medium | Break into smaller milestones; adjust scope |
| No testers | Medium | Low | Engage community early; offer alpha access |
| Low adoption | Low | Medium | Good docs; easy install; show value |

---

## Success Metrics

### v1.0 Goals

**Functionality:**
- ✅ Parses vanilla tweakdb.bin without errors
- ✅ Supports YAML and .tweak mod files
- ✅ Applies modifications correctly
- ✅ Outputs valid modified tweakdb.bin
- ✅ Game accepts output (no crashes)

**Compatibility:**
- ✅ 2-3 real mods work end-to-end
- ✅ NIGHT CITY ALIVE produces expected in-game effects
- ✅ 80%+ of YAML/tweak-based mods "just work"

**Quality:**
- ✅ Clear error messages
- ✅ Backup/restore safety features
- ✅ Documentation is comprehensive
- ✅ 80%+ code coverage in tests

### Long-Term Goals

**Adoption:**
- 50+ mods confirmed compatible
- macOS modding community awareness
- Community contributions (bug reports, PRs)

**Features:**
- RED tweak support
- GUI wrapper
- Mod manager integration

---

## Timeline Estimate

**Total:** ~15 weeks (part-time development)

| Phase | Duration | Dependencies |
|-------|----------|--------------|
| 1. Foundation | 2 weeks | None |
| 2. TweakDB Reader | 2 weeks | Phase 1 |
| 3. Mod Parsers | 2 weeks | Phase 1 |
| 4. Bridge & Application | 2 weeks | Phases 2 & 3 |
| 5. TweakDB Writer | 2 weeks | Phase 2 |
| 6. CLI & Integration | 2 weeks | Phases 4 & 5 |
| 7. Testing & Validation | 2 weeks | Phase 6 |
| 8. Documentation & Release | 1 week | Phase 7 |

**Notes:**
- Assumes part-time development (~10-15 hours/week)
- Full-time development: ~8 weeks
- Some phases can overlap (parsers + reader)

---

## Immediate Next Steps

### Now (This Session)

1. ✅ Complete research and planning
2. ✅ Document findings
3. ✅ Create revised plan (this document)
4. [ ] Set up CMake project structure
5. [ ] Add dependencies
6. [ ] Create initial directory structure

### Next Session

1. [ ] Begin Phase 1: Foundation
2. [ ] Study WolvenKit TweakDB parser source
3. [ ] Document TweakDB binary format
4. [ ] Create synthetic test data
5. [ ] Implement core data structures

### Week 1 Goals

- Complete Phase 1 tasks
- Have buildable project
- Have initial binary format documentation
- Have test infrastructure ready

---

## Open Questions

1. **Binary Format Details:**
   - Exact encoding of arrays?
   - String pool structure?
   - Type ID mapping?
   → Research WolvenKit source, community wikis

2. **macOS Game Specifics:**
   - Exact path to tweakdb.bin on macOS?
   - Any differences from Windows version?
   → Test with community once tool is ready

3. **Mod Load Order:**
   - How to handle conflicts between mods?
   - Alphabetical? Config file? User-specified?
   → Decide in Phase 1, implement in Phase 6

4. **Distribution:**
   - Pre-built binaries for macOS?
   - Homebrew formula?
   - Nexus Mods listing?
   → Plan in Phase 8

---

## Conclusion

This revised plan is grounded in:
- Real research into the Cyberpunk 2077 modding ecosystem
- Understanding of our development constraints
- Realistic technical approach (offline patching)
- Clear scope and success criteria

**Key Difference from Original Plan:**
- Original: Assumed tools existed, vague about how runtime worked
- Revised: Acknowledges we're building from scratch, offline approach, no game access

**Confidence Level:** High
- We have clear examples to learn from (WolvenKit, Gibbed, TweakXL)
- Technology stack is proven
- Scope is realistic for v1
- Community can help with testing

**Let's build this! 🚀**

Next: Set up CMake project and begin Phase 1.
