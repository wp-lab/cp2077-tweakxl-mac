# Research Findings: Cyberpunk 2077 Modding & TweakXL

**Date:** November 2025
**Purpose:** Document research to inform realistic TweakXL-macOS implementation plan

---

## Executive Summary

After comprehensive research into Cyberpunk 2077 modding ecosystem, TweakDB format, and TweakXL implementation, we've identified the actual requirements and constraints for building a macOS-compatible TweakXL alternative.

### Key Finding: Cyberpunk 2077 IS on macOS!

**Cyberpunk 2077: Ultimate Edition** was released natively for macOS on **July 17, 2025**:
- Native Apple Silicon support (M1+)
- Uses Metal 3 rendering
- Full cross-platform save support (Steam/GOG/Epic)
- Requires 16GB+ unified memory
- ~100GB installation size
- Includes all patches and Phantom Liberty expansion

This is critical: **we're targeting a real, native macOS game**, not an emulation layer.

---

## TweakDB: Technical Overview

### What is TweakDB?

TweakDB is Cyberpunk 2077's proprietary configuration database containing game entity data and behavior settings.

**File Location:**
- Windows: `Cyberpunk 2077\r6\cache\tweakdb.bin`
- macOS: `Cyberpunk 2077.app/Contents/Resources/r6/cache/tweakdb.bin` (likely)
- Modded: `r6/cache/modded/tweakdb.bin`

### TweakDB Structure

**Binary Format:**
- Custom REDengine 4 binary format
- Not human-readable
- Contains type information, values, and relationships

**Data Model:**

1. **Flats** - Individual data values:
   - Numbers (integers, floats)
   - Strings
   - Booleans
   - CNames (hashed string identifiers)
   - Localization keys
   - Resource references
   - Arrays

2. **Records** - Collections of flats:
   - Typed structures (e.g., `Clothing`, `Vehicle`, `Weapon`)
   - Properties map to flats
   - Support inheritance

**TweakDBID System:**
- Unique identifiers for every flat and record
- Based on FNV1A64 hash of the string name
- Format: `Package.Group.Item:Property`
- Example: `Items.Preset_Katana_Default:quality`

### Known Tools

| Tool | Type | Language | Purpose |
|------|------|----------|---------|
| **TweakDump** | Official | C++? | Dump TweakDB to text format |
| **WolvenKit** | Community | C# | Full modding suite with TweakDB parser |
| **Gibbed.RED4** | Community | C# | RED4 engine tools collection |
| **TweakXL** | Community | C++ | Runtime TweakDB modification framework |
| **TweakDB-Edit** | Community | ? | Convert tweakdump to JSON (outdated) |

---

## TweakXL: How It Works (Windows)

### Architecture

TweakXL is a **RED4ext plugin** that runs **at runtime** inside the game process:

1. **Load Phase:**
   - Game starts, loads RED4ext.dll
   - RED4ext loads TweakXL.dll plugin
   - TweakXL hooks into game's TweakDB initialization

2. **Parse Phase:**
   - Scans `r6/tweaks/` directory
   - Parses YAML, RED, and .tweak files
   - Builds in-memory modification list

3. **Apply Phase:**
   - During TweakDB initialization, hooks execute
   - TweakXL modifies TweakDB in memory
   - Operations: Assign, Append, Remove, Clone, Create

4. **Runtime Phase:**
   - Provides scripting API (via redscript)
   - Supports hot-reloading during development

### File Formats Supported

#### 1. YAML Tweaks (`.yaml`, `.yml`)

TweakXL's proprietary format:

```yaml
# Modify flat
PreventionSystem.setup.totalEntitiesLimit: 40

# Modify record properties
Items.MyClothingItem:
  $type: Clothing
  entityName: my_item
  quality: Quality.Legendary

# Array operations
Vehicle.v_sport2_quadra_type66_nomad:
  equipment:
    !append:
      - Vehicle.v_sport2_Equipment_Part_Extra
```

**Operations:**
- Direct assignment: `property: value`
- Append: `!append`, `!append-once`, `!append-from`
- Prepend: `!prepend`, `!prepend-once`, `!prepend-from`
- Remove: `!remove`

**Advanced Features:**
- Templates with `$instances` for bulk generation
- YAML anchors/aliases for reuse
- Inline record definitions
- Type coercion (auto-convert strings to CName, etc.)

#### 2. RED Tweaks

REDengine/REDmod format (original game format):

```swift
# Similar to redscript syntax
PreventionSystem.setup.totalEntitiesLimit = 40
```

#### 3. .tweak Files

Flat file format (used by many mods like NIGHT CITY ALIVE):

```
# Grammar defined by PEGTL
package MyMod.Config

gang.density = 2.5
ai.aggression = 1.8
```

### Dependencies

**Required:**
- **RED4ext** 1.28.0+ - Plugin loader and runtime hooks
- **redscript** 0.5.27+ - Script compiler/runtime (for script extensions)
- **Cyberpunk 2077** 2.3+ - The game

**Libraries (bundled):**
- **PEGTL** - Parser for .tweak grammar
- **yaml-cpp** - YAML parsing
- **TiltedCore** - Utility library (from vendor/)
- **spdlog** - Logging

**Build System:**
- **XMake** - Build configuration
- C++20 compiler
- Windows-specific: MSVC or Clang-CL

### Source Code Structure

```
cp2077-tweak-xl/
├── src/
│   ├── Red/
│   │   └── TweakDB/
│   │       ├── Source/       # Parsers for .tweak format
│   │       │   ├── Grammar.hpp  # PEGTL grammar
│   │       │   ├── Parser.cpp   # Build TweakSource objects
│   │       │   └── Source.hpp   # In-memory representation
│   │       ├── Buffer.hpp    # TweakDB memory access
│   │       ├── Manager.hpp   # Apply tweaks to live DB
│   │       └── Reflection.hpp # Type introspection
│   ├── App/              # Plugin lifecycle
│   └── Support/          # Utility code
├── lib/                  # Core libraries (TiltedCore, etc.)
├── vendor/               # Third-party dependencies
├── data/                 # Metadata files
│   ├── ExtraFlats        # Additional flat definitions
│   └── InheritanceMap    # Record inheritance tree
├── scripts/              # Redscript integration
└── xmake.lua             # Build config
```

---

## Why macOS is Different

### RED4ext is Windows-Only

RED4ext uses **DLL injection** and **Windows-specific hooking**:
- Injects code into game process
- Hooks game functions via detours
- Accesses game's internal RTTI (Run-Time Type Information)
- Requires Windows API and PE (Portable Executable) format

**This won't work on macOS:**
- macOS uses dylib, not DLL
- Different process memory model
- Metal 3 port may have different internals
- No community-developed hooking framework yet

### Implication: Offline Patching

Since we can't hook the game at runtime, we must:
1. **Read** the TweakDB binary from disk
2. **Parse** mod files (YAML/RED/.tweak)
3. **Apply** modifications to the binary data structure
4. **Write** a modified TweakDB back to disk
5. **Game loads** the pre-modified TweakDB

This is fundamentally different from TweakXL's runtime approach.

---

## Development Environment Constraints

### What We Have

**Environment:** Linux x86_64 (containerized)
- **OS:** Linux 4.4.0
- **Compilers:**
  - Clang 18 (clang++)
  - GCC 13 (g++)
- **Build Tools:**
  - CMake 3.28.3
  - Git
  - Python 3.11
- **NOT macOS:** Can't test on real target platform
- **NOT Windows:** Can't run original TweakXL

### What We Don't Have

**Critical Limitations:**
- ❌ **No Cyberpunk 2077 installation** - Can't access game files
- ❌ **No tweakdb.bin samples** - Can't test parsing
- ❌ **No macOS** - Can't verify compatibility
- ❌ **No RED4ext** - Can't use runtime approach
- ❌ **No game RTTI** - Can't introspect types dynamically

### What This Means

**We must develop based on:**
1. **Documentation** from existing tools (WolvenKit, Gibbed, TweakDump output)
2. **Source code analysis** of open-source parsers
3. **Schema files** (like Gibbed's TweakDB-Schema repository)
4. **Community knowledge** from wikis and forums
5. **Synthetic test data** we create ourselves

**We cannot:**
- Directly reverse-engineer the binary format from game files
- Test against real mod files until late in development
- Verify behavior matches game expectations
- Hot-reload or debug in a live game

---

## Existing Parser Implementations

### WolvenKit TweakDB Parser (C#)

**Repository:** https://github.com/WolvenKit/WolvenKit

**Capabilities:**
- Full TweakDB binary parser
- Converts tweakdb.bin to JSON
- Supports browsing database
- Can generate .tweak files from records

**Relevance:**
- Open source (GPL-3.0)
- C# code (not directly portable to C++)
- Good reference for binary format
- Shows what's possible

### Gibbed.RED4 (C#)

**Repository:** https://github.com/gibbed/Gibbed.RED4

**Capabilities:**
- Tools for RED4 engine files
- Includes TweakDB utilities (presumed)
- Research-focused

**Relevance:**
- Early reverse engineering work
- May have format documentation
- C# implementation

### TweakXL Parsers (C++)

**What's Reusable:**

1. **PEGTL Grammar** (`src/Red/TweakDB/Source/Grammar.hpp`):
   - Defines .tweak file syntax
   - Can be used standalone with PEGTL library
   - No runtime dependencies

2. **Parser Logic** (`src/Red/TweakDB/Source/Parser.cpp`):
   - Builds `TweakSource` objects from parsed .tweak files
   - Has some RED4ext/TiltedCore dependencies
   - Would need adaptation

3. **Data Structures** (`src/Red/TweakDB/Source/Source.hpp`):
   - `TweakSource`, `TweakGroup`, `TweakFlat`, `TweakValue`
   - In-memory representation of tweaks
   - Useful model to follow

4. **Metadata** (`data/ExtraFlats`, `data/InheritanceMap`):
   - JSON files with TweakDB schema information
   - No code dependencies
   - Directly usable

**What's NOT Reusable:**

1. **Runtime Components** (`src/Red/TweakDB/{Buffer,Manager,Reflection}.hpp`):
   - Operate on live game memory
   - Use RED4ext SDK types
   - Windows/runtime specific

2. **Core Library** (`lib/Red/*`, `lib/Core/*`):
   - RED4ext SDK wrappers
   - TiltedCore utilities
   - Would need full reimplementation

3. **Plugin Infrastructure** (`src/App/*`, `support/red4ext/*`):
   - DLL entry points
   - RED4ext callbacks
   - Not applicable to offline tool

---

## Real-World Mod Examples

### NIGHT CITY ALIVE

**What it does:**
- Increases gang density in districts
- Modifies AI behavior (aggression, reactions)
- Changes traffic patterns
- Adjusts police response

**Format:**
- Uses `.tweak` files
- Modifies existing records
- Primarily numeric tweaks (densities, thresholds)

**Why it's a good test:**
- Popular mod (proven useful)
- Simple operations (mostly assignments)
- Clear in-game effects (visually verifiable)
- Doesn't use complex TweakXL features

### Other Potential Test Mods

**Equipment/Item Mods:**
- Change weapon stats
- Modify clothing properties
- Adjust crafting costs

**Gameplay Tweaks:**
- Skill XP rates
- Difficulty modifiers
- Economy adjustments

**Ideal Test Suite:**
1. Simple flat modification (single value change)
2. Record property change (multiple values in one record)
3. Array append operation (add to list)
4. YAML-based mod (test YAML parser)
5. .tweak-based mod (test PEGTL parser)

---

## TweakDB Binary Format (Partial Knowledge)

### Header Structure (Inferred)

```
Offset | Size | Type   | Description
-------|------|--------|-------------
0x00   | 4    | uint32 | Magic: 0x0BB1DB47 (or "47 db b1 0b" little-endian)
0x04   | 4    | uint32 | Version
0x08   | 4    | uint32 | Offset to flats section
0x0C   | 4    | uint32 | Number of flats
0x10   | 4    | uint32 | Offset to records section
0x14   | 4    | uint32 | Number of records
...    | ...  | ...    | (More fields, exact structure unknown)
```

### Data Sections (Approximate)

1. **Flats Table:**
   - Array of flat entries
   - Each entry: TweakDBID (8 bytes) + Type (?) + Value (variable)

2. **Records Table:**
   - Array of record definitions
   - Record ID + Type + Properties list

3. **String Pool:**
   - Null-terminated strings
   - Referenced by offset from records/flats

4. **Type Information:**
   - Class/struct definitions
   - Property metadata

**Note:** Exact format requires reverse engineering from actual files or studying WolvenKit source.

---

## Recommended Technology Stack

### Core Language

**C++17 or C++20:**
- Performance (binary parsing)
- Native on all platforms
- Matches TweakXL (easier to adapt code)
- Strong typing for binary structures

### Build System

**CMake:**
- Standard for C++ cross-platform
- Better macOS support than XMake
- Easier dependency management
- Widely known

**Alternative: XMake**
- Matches TweakXL
- Simpler syntax
- Good Lua-based config
- Smaller community

**Recommendation:** **CMake** for this project (macOS focus, broader compatibility)

### Libraries

**Must Have:**

1. **yaml-cpp** (YAML parsing)
   - Used by TweakXL
   - Mature, stable
   - Easy integration

2. **PEGTL** (Parser for .tweak files)
   - Header-only library
   - Used by TweakXL (can reuse grammar)
   - Modern C++ approach

**Nice to Have:**

3. **spdlog** (Logging)
   - Fast, feature-rich
   - Used by TweakXL
   - Good diagnostics

4. **CLI11** or **cxxopts** (Command-line parsing)
   - Modern C++ CLI frameworks
   - Better than hand-rolled parsing

5. **nlohmann/json** (JSON for metadata)
   - Use TweakXL's data/ JSON files
   - Schema validation

**Not Needed:**

- ❌ TiltedCore (Windows-specific, RED4ext dependency)
- ❌ RED4ext SDK (runtime hooking)
- ❌ Qt/GUI libraries (CLI tool for v1)

---

## Revised Project Scope

### What We're Actually Building

**Name:** TweakXL-macOS (or "TweakDB Patcher for macOS")

**Type:** Offline TweakDB patcher

**Goal:** Enable existing Windows TweakXL mods to work on macOS Cyberpunk 2077 with minimal to no changes.

### Core Functionality (v1.0)

**Must Have:**

1. ✅ **Read TweakDB binary** (`tweakdb.bin`)
   - Parse header, locate sections
   - Extract flats and records
   - Build in-memory database representation

2. ✅ **Parse YAML tweak files**
   - Support TweakXL YAML syntax
   - Flat modifications
   - Record modifications
   - Array operations (append, remove)

3. ✅ **Parse .tweak files**
   - Use PEGTL grammar from TweakXL
   - Build same AST structure
   - Convert to modification list

4. ✅ **Apply modifications**
   - Assign values to flats
   - Modify record properties
   - Append/remove from arrays
   - Basic type coercion

5. ✅ **Write modified TweakDB**
   - Rebuild binary structure
   - Maintain format compatibility
   - Preserve metadata/offsets

6. ✅ **CLI Interface**
   - `--game-dir <path>` - Locate game
   - `--mods-dir <path>` - Mod directory
   - `--backup` / `--restore` - Safe operations
   - `--dry-run` - Preview changes

**Nice to Have (v1.x):**

- ⚠️ RED tweak file support
- ⚠️ Record cloning/inheritance
- ⚠️ Validation warnings
- ⚠️ Mod load order control

**Out of Scope (v1):**

- ❌ Script extensions (requires runtime)
- ❌ Hot reloading (requires runtime)
- ❌ GUI application
- ❌ Redscript integration
- ❌ Dynamic TweakDB queries
- ❌ Runtime hooks/plugins

### Compatibility Target

**Mod Compatibility:**
- 80%+ of YAML-based TweakXL mods should work
- 90%+ of .tweak-based mods should work
- RED tweak mods: best effort
- Script-heavy mods: won't work (document limitation)

**Test Cases:**
1. NIGHT CITY ALIVE - Traffic/gang density mod
2. Simple item stat mod (e.g., weapon damage tweak)
3. YAML-heavy mod (TBD - find example)

**Success Criteria:**
- Test mods produce expected in-game results
- No game crashes
- Behavior matches Windows TweakXL (where applicable)

---

## Development Phases (Revised)

### Phase 1: Foundation & Research
**Duration:** 1-2 weeks
**Goal:** Understand formats without game access

**Tasks:**
1. Study WolvenKit C# source for TweakDB parser
2. Document binary format (header, sections, types)
3. Study TweakXL YAML/tweak parsers
4. Create test data (synthetic tweakdb.bin header)
5. Set up build system (CMake)
6. Integrate dependencies (yaml-cpp, PEGTL, spdlog)

**Deliverables:**
- `docs/tweakdb-format.md` - Binary format spec
- `docs/tweak-formats.md` - Mod file formats
- CMake project that builds
- Hello world CLI

### Phase 2: TweakDB Binary Parser
**Duration:** 2-3 weeks
**Goal:** Read and understand tweakdb.bin

**Tasks:**
1. Implement header parser
2. Implement flats parser
3. Implement records parser
4. Build in-memory database model
5. Write tests (against synthetic data)
6. Validate against WolvenKit's understanding

**Deliverables:**
- `src/tweakdb/Reader.cpp` - Binary parser
- `src/tweakdb/Database.cpp` - In-memory model
- Unit tests
- Tool to dump TweakDB to JSON (like TweakDump)

### Phase 3: Mod File Parsers
**Duration:** 2-3 weeks
**Goal:** Parse YAML and .tweak files

**Tasks:**
1. Integrate yaml-cpp, parse basic YAML
2. Implement TweakXL YAML syntax support
3. Port PEGTL grammar from TweakXL
4. Implement .tweak parser
5. Build modification representation
6. Write tests

**Deliverables:**
- `src/parsers/YAMLParser.cpp`
- `src/parsers/TweakParser.cpp` (PEGTL-based)
- `src/model/Modification.cpp` - Mod representation
- Sample mod files for testing

### Phase 4: Bridge Layer & Application
**Duration:** 2-3 weeks
**Goal:** Apply mods to TweakDB

**Tasks:**
1. Implement modification application logic
2. Handle different operation types (assign, append, remove)
3. Type coercion (string → CName, etc.)
4. Implement TweakDB writer (binary output)
5. Build CLI tool
6. End-to-end test

**Deliverables:**
- `src/tweakdb/Writer.cpp` - Binary writer
- `src/bridge/Applicator.cpp` - Apply modifications
- `src/cli/main.cpp` - CLI interface
- Working tool: `tweakxl-mac`

### Phase 5: Testing & Validation
**Duration:** Ongoing / 1-2 weeks focused testing
**Goal:** Prove it works with real mods

**Tasks:**
1. Obtain real tweakdb.bin (from game or community)
2. Obtain NIGHT CITY ALIVE mod files
3. Run tool, generate modded tweakdb.bin
4. Test in game (requires macOS + game)
5. Document issues and gaps
6. Iterate on bugs

**Deliverables:**
- `docs/compatibility-report.md`
- Bug fixes
- v1.0 release candidate

### Phase 6: Documentation & Release
**Duration:** 1 week
**Goal:** Make it usable for others

**Tasks:**
1. Write user guide
2. Installation instructions
3. Troubleshooting guide
4. Known limitations document
5. Create releases (GitHub)
6. Announce to community

**Deliverables:**
- `docs/user-guide.md`
- `docs/installation.md`
- `docs/troubleshooting.md`
- v1.0 release

---

## Critical Unknowns

### Questions We Must Answer

1. **Binary Format Details:**
   - Exact header structure?
   - How are types encoded?
   - How are arrays stored?
   - String pool format?

2. **macOS Game Specifics:**
   - Is TweakDB format identical to Windows?
   - Are there Metal 3 port differences?
   - Does modded TweakDB work same way?

3. **Type System:**
   - How are CName, TweakDBID stored?
   - Type coercion rules?
   - Inheritance mechanics?

4. **Validation:**
   - Does game validate TweakDB on load?
   - What causes crashes vs. warnings?
   - Are there checksums?

### How We'll Answer Them

1. **Study WolvenKit source** (C# → C++ mental translation)
2. **Read community wikis** (RedModding wiki)
3. **Examine TweakXL source** (where it interacts with binary data)
4. **Test with community** (beta testers with macOS game)
5. **Iterate** (fix bugs as we discover them)

---

## Risk Assessment

### High Risks

1. **Binary format too complex**
   - Mitigation: Start simple (flats only), iterate
   - Fallback: JSON-based intermediate format

2. **No test data available**
   - Mitigation: Synthetic test data, community samples
   - Fallback: Request from community early

3. **macOS port differences**
   - Mitigation: Document assumptions, plan for variations
   - Fallback: Windows-only first, macOS later

### Medium Risks

1. **Type system complexity**
   - Mitigation: Support common types first
   - Fallback: Limit v1 scope to proven types

2. **Mod compatibility lower than hoped**
   - Mitigation: Clear documentation of limitations
   - Fallback: "Best effort" v1, improve v2

3. **Performance (large TweakDB)**
   - Mitigation: Use memory mapping, efficient data structures
   - Fallback: Progress bars, optimization pass

### Low Risks

1. **Build system issues** - CMake is well-known
2. **Parser bugs** - yaml-cpp and PEGTL are mature
3. **CLI usability** - Can iterate based on feedback

---

## Success Metrics

### v1.0 Goals

1. ✅ **Reads vanilla tweakdb.bin** without crashing
2. ✅ **Parses NIGHT CITY ALIVE** mod files
3. ✅ **Generates modified tweakdb.bin** that game accepts
4. ✅ **In-game effects visible** (higher gang density)
5. ✅ **2+ real mods working** end-to-end
6. ✅ **Clear documentation** for users

### Long-term Goals

- 50+ mods confirmed working
- Community contributions (bug reports, PRs)
- macOS Cyberpunk modding ecosystem grows
- Maybe: GUI wrapper, mod manager integration

---

## Next Steps

Based on this research, the immediate next steps are:

1. **Create detailed technical plan** (`docs/revised-plan.md`)
2. **Set up project structure** (CMake, dependencies)
3. **Begin Phase 1: TweakDB format documentation**
4. **Study WolvenKit TweakDB parser** (reverse-engineer format)
5. **Create synthetic test data** (minimal valid tweakdb.bin)

Let's build this! 🚀
