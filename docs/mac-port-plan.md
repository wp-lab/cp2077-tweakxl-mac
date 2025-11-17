# TweakXL-macOS Port Plan

## Mission Goal

Build a macOS version of TweakXL ("TweakXL-macOS") that is as compatible as possible with existing Windows TweakXL mods.

### Specifically

- **Accept the same formats and layout:**
  - YAML tweaks
  - RED tweaks
  - `.tweak` flat files
- **Apply those tweaks** to the macOS TweakDB files (`tweakdb.bin`, `tweakdb_ep1.bin`, etc.)
- **Use an offline patcher model** (no RED4ext plugin / DLL injection on macOS)
- **Preserve TweakXL semantics** for TweakDB operations where possible (Assign, Append, Remove, inheritance)

### Original TweakXL Repository

https://github.com/psiberx/cp2077-tweak-xl

---

## Phase 0 – Bootstrap

**Status:** To Do

**Goal:** Set up the basic development environment and verify we can modify TweakDB files on macOS.

### Tasks

- [ ] Clone this repository structure
- [ ] Set up build system (CMake or xmake)
- [ ] Create basic `tweakdb-patcher` CLI tool that can:
  - Read `tweakdb.bin` file
  - Report basic stats (file size, header info)
  - Copy byte-for-byte to another path
  - `--patch-byte <offset> <value>`: change a single byte in a copy
  - `--dump-hex <offset> <length>`: inspect any region in hex
- [ ] **Verify:** macOS Cyberpunk 2077 accepts a modified `tweakdb.bin`

### Success Criteria

- `tweakdb-patcher` builds successfully on macOS
- Game boots with a patched `tweakdb.bin` copy
- No crashes from basic byte modifications

---

## Phase 1 – Scope & Compatibility Definition

**Status:** Not Started

**Goal:** Define what "compatible with existing TweakXL mods" actually means, so we don't chase everything at once.

### Tasks

#### 1. Feature Surface Definition

Document which parts of TweakXL you want in v1:

**Formats:**
- [ ] YAML tweaks (TweakXL's YAML format)
- [ ] RED tweaks (TweakXL's alternative format)
- [ ] `.tweak` files (like NIGHT CITY ALIVE's)

**Operations:**
- [ ] `=` (Assign)
- [ ] `+=` (Append)
- [ ] `-=` (Remove)
- [ ] Record creation, cloning, inheritance (at least where real mods use them)

**Out-of-scope initially:**
- Script extensions that rely on in-process RED4ext hooking and game RTTI
- Dynamic runtime manipulation

#### 2. Runtime Model

Confirm we are targeting:
- [ ] An offline CLI that patches TweakDB on disk
- [ ] No RED4ext DLL on macOS (at least for v1)
- [ ] Apply patches before game launch

#### 3. Pick Test Mods

Choose 2–3 real TweakXL mods to serve as compatibility test cases:

- [ ] **NIGHT CITY ALIVE** (for `.tweak` and AI/traffic adjustments)
- [ ] Another mod that uses YAML (and/or RED) tweaks
- [ ] Possibly a smaller "example" mod from TweakXL docs

### Deliverables

- `docs/feature-scope.md` - Document of supported features
- `docs/test-mods.md` - List of target mods with download links and expected behaviors

---

## Phase 2 – Mac Tool Baseline (tweakdb-patcher CLI)

**Status:** Not Started

**Goal:** Have a reliable offline TweakDB manipulator that we trust, before layering TweakXL's higher-level logic.

### Core Features (from Phase 0)

- [x] Read and report `tweakdb.bin` size
- [x] Copy byte-for-byte to another path
- [x] `--patch-byte <offset> <value>`: change a single byte
- [x] `--dump-hex <offset> <length>`: inspect any region in hex
- [x] Verify game boots with patched file

### Additional Features

- [ ] `--patch-text <offset> <ascii-string>`: Overwrite N bytes starting at an offset with ASCII text (no length change)
- [ ] `--backup <path>`: Create timestamped backups
- [ ] `--restore <backup>`: Restore from backup
- [ ] Better error handling and validation
- [ ] Document the TweakDB file format structure

### Success Criteria

- Can make reproducible changes to TweakDB
- Can verify changes via hex dump
- Game accepts all modifications
- Clear documentation of tool capabilities

---

## Phase 3 – TweakXL Code Survey & Mac Stubs

**Status:** Not Started

**Goal:** Figure out which TweakXL pieces we can reuse and which we need to stub or replace.

### Tasks

#### 1. Classify Components

**Reusable in offline mode:**
- [ ] `src/Red/TweakDB/Source/*`:
  - `Grammar.hpp` – PEGTL grammar for `.tweak` syntax
  - `Parser.hpp/cpp` – builds TweakSource, TweakGroup, TweakFlat, etc.
  - `Source.hpp` – the in-memory tweak representation
- [ ] Metadata from `data/` (e.g., ExtraFlats, InheritanceMap)

**Windows-only / runtime (need to stub or replace):**
- [ ] `src/Red/TweakDB/{Buffer,Reflection,Manager}` – manipulate live Red::TweakDB
- [ ] `lib/Red/*`, `lib/Core/*` – parts that rely on RED4ext SDK and TiltedCore
- [ ] `src/pch.hpp` and plugin entrypoints

#### 2. Mac Offline Compatibility Layer

- [ ] Introduce `TWEAKXL_MAC_OFFLINE` preprocessor flag
- [ ] Create `src/mac/CoreStubs.hpp`:
  - Minimal `Core::Vector`, `Core::SharedPtr`, `Core::MakeShared`, etc.
- [ ] Stub/alias basic RED types as needed (e.g., `CName` equivalent)

#### 3. Compile Parser in Offline Mode

- [ ] Under `TWEAKXL_MAC_OFFLINE`:
  - Ensure `Red::TweakParser::Parse(path)` works without RED4ext/TiltedCore
- [ ] Create test program that parses a simple `.tweak` file
- [ ] Test with `data/mac-test/example.tweak`

### Deliverables

- `src/mac/CoreStubs.hpp` - Stub implementations
- `src/mac/OfflineParser.cpp` - Standalone parser test
- `docs/component-classification.md` - What's reusable vs. what needs stubs

---

## Phase 4 – Offline TweakDB Model & Bridge

**Status:** Not Started

**Goal:** Teach TweakXL-macOS how to apply parsed tweaks to the actual `tweakdb.bin` file.

### Tasks

#### 1. Model the On-Disk TweakDB

- [ ] Document TweakDB file format:
  - Header fields (magic `47 db b1 0b`, version, offsets, counts)
  - Key sections (flat value buffers, lists of record IDs)
- [ ] Focus on sections touched by target mods:
  - Gang density, AI flags, vehicle lists

#### 2. Offline TweakDBFile Abstraction

- [ ] Define C++ types to represent:
  - Map of TweakDBIDs → flat entries
  - Records and their properties
- [ ] Implement:
  - `TweakDBFile::Load(path)`
  - `TweakDBFile::Save(path)`
  - `TweakDBFile::GetFlat(id)`
  - `TweakDBFile::SetFlat(id, value)`

#### 3. Bridge Layer

- [ ] Map `TweakSource` / `TweakFlat` / `TweakValue` into actions on `TweakDBFile`:
  - For each parsed flat:
    - Compute target TweakDBID (mod + group + flat name)
    - Apply Assign/Append/Remove on appropriate flat's value
  - Handle simple record operations

#### 4. First End-to-End Tweak

- [ ] Hard-code one simple tweak (e.g., change a numeric flat value)
- [ ] Run: Parse → Bridge → Write patched `tweakdb.bin`
- [ ] Verify:
  - Game still boots
  - Behavior changes as expected (e.g., stat value visible in-game)

### Deliverables

- `src/mac/TweakDBFile.hpp/cpp` - On-disk TweakDB model
- `src/mac/TweakBridge.hpp/cpp` - Bridge layer
- `docs/tweakdb-format.md` - File format documentation
- Working demo of single tweak modification

---

## Phase 5 – Mod Compatibility Layer (TweakXL Formats)

**Status:** Not Started

**Goal:** Have TweakXL-macOS read and apply tweaks from real TweakXL mods.

### Tasks

#### 1. YAML Tweaks

- [ ] Integrate yaml-cpp library
- [ ] Implement YAML-to-TweakDB mapping (same as TweakXL on Windows):
  - Parse YAML structure
  - Build internal representation (TweakSource-like objects)
  - Feed into bridge
- [ ] Test with sample YAML tweak files

#### 2. RED Tweaks

- [ ] Support RED-based syntax that describes TweakDB changes
- [ ] Focus on subset that doesn't require runtime game scripting
- [ ] Translate to operations on TweakDBFile

#### 3. .tweak Files via Red::TweakParser

- [ ] Enable parser in mac offline mode (from Phase 3)
- [ ] Confirm parsing of real mod `.tweak` files
- [ ] Translate them into operations on TweakDBFile

#### 4. Mod Folder Conventions

- [ ] Define mods directory structure mirroring TweakXL's expectations
- [ ] CLI arguments:
  - `--mods-dir /path/to/tweakxl-mods`
  - `--config /path/to/tweakxl-config.yml` (if needed)
- [ ] Auto-discover mod files in directory

### Deliverables

- `src/mac/YAMLTweakLoader.cpp` - YAML tweak support
- `src/mac/REDTweakLoader.cpp` - RED tweak support
- `src/mac/TweakFileLoader.cpp` - .tweak file support
- `src/mac/ModLoader.cpp` - Mod discovery and loading
- CLI with mod directory support

---

## Phase 6 – Real Mod Validation (NIGHT CITY ALIVE & Friends)

**Status:** Not Started

**Goal:** Prove TweakXL-macOS is actually compatible, not just theoretically.

### Tasks

#### 1. NIGHT CITY ALIVE

- [ ] Obtain mod files (`.tweak` / `.tweakdb` data)
- [ ] Run TweakXL-macOS to patch TweakDB on mac
- [ ] Test in-game:
  - Higher gang density
  - Behavior changes (gang reactions, etc.)
- [ ] Compare behavior with documented Windows + TweakXL results
- [ ] Document any differences

#### 2. Other Mods

- [ ] Test additional TweakXL mods using different features (YAML-heavy, etc.)
- [ ] For each mod:
  - Parse, apply, patch
  - Test in-game
  - Document results

#### 3. Document Gaps

- [ ] Create `docs/compatibility-report.md`:
  - What works perfectly
  - What has minor differences
  - Known incompatibilities
  - Features not yet implemented

### Success Criteria

- At least 2 real mods work correctly
- Differences are documented
- Clear picture of v1 compatibility level

---

## Phase 7 – Packaging & UX

**Status:** Not Started

**Goal:** Make this usable for you and others without re-running CMake manually every time.

### Tasks

#### 1. CLI Wrapper

- [ ] Create `tweakxl-mac` script/binary that wraps the tooling:
  - `--game-dir` to locate the game
  - `--mods-dir` to pick up TweakXL mods
  - `--backup` / `--restore` options for TweakDB
  - `--list-mods` to show discovered mods
  - `--apply` to apply all mods
  - `--dry-run` to preview changes

#### 2. Documentation

- [ ] Write clear README for macOS:
  - How to build/install TweakXL-macOS
  - How to install mods made for Windows TweakXL
  - How to apply/unapply patches safely
  - Troubleshooting guide
- [ ] Create `docs/user-guide.md`
- [ ] Add examples and screenshots

#### 3. Build & Distribution

- [ ] Create build scripts for easy compilation
- [ ] Consider pre-built binaries for releases
- [ ] Installation instructions (Homebrew formula?)

#### 4. Optional: Nicer UX

- [ ] Small `.app` wrapper for macOS
- [ ] Simple GUI (Qt or native macOS)
- [ ] Not required for v1 but quality-of-life improvement

### Deliverables

- User-friendly CLI tool
- Comprehensive documentation
- Easy installation process

---

## Current Status

**Current Phase:** Phase 0 (Bootstrap)
**Branch:** `claude/tweakxl-macos-port-plan-01T9jS4k4J5gsWNHShochisQ`

### Next Immediate Steps

1. **Set up basic project structure** ✓
2. **Create this plan document** ✓
3. **Review original TweakXL repository** to understand architecture
4. **Begin Phase 0:** Build basic `tweakdb-patcher` tool
5. **Complete Phase 1:** Define exact scope and choose test mods

### Key Resources

- Original TweakXL: https://github.com/psiberx/cp2077-tweak-xl
- NIGHT CITY ALIVE mod: [TBD - add link]
- Cyberpunk 2077 macOS installation path: [TBD - document]

---

## Notes & Decisions

### Architecture Decisions

- **Offline vs Runtime:** We're building an offline patcher, not a runtime plugin
- **No RED4ext on macOS:** At least for v1, avoiding the complexity of DLL injection
- **Compatibility focus:** Prioritize working with existing Windows mods over new features

### Technical Constraints

- macOS Cyberpunk 2077 uses same TweakDB file format
- No access to game's internal RTTI at runtime
- Must work with on-disk files only

### Future Possibilities (Post-v1)

- Runtime hooking if RED4ext becomes viable on macOS
- Additional tweak formats
- Mod manager integration
- Automated mod compatibility testing
