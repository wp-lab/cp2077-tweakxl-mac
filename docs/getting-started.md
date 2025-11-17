# Getting Started with TweakXL-macOS Development

## Current Status

**Phase:** Phase 0 (Bootstrap)
**Branch:** `claude/tweakxl-macos-port-plan-01T9jS4k4J5gsWNHShochisQ`

We're starting fresh with this port. While the plan mentions some tools like `tweakdb-patcher` as if they exist, we're building everything from scratch for this new repository.

## What We Have

- ✅ Project structure (docs/, src/, data/)
- ✅ Detailed plan document (docs/mac-port-plan.md)
- ✅ Understanding of original TweakXL architecture

## What We Need to Build

### Phase 0: Bootstrap (Current)

1. **Set up build system**
   - Choose between CMake or XMake (original uses XMake)
   - Configure for macOS compilation

2. **Build `tweakdb-patcher` CLI tool**
   - Basic C++ application
   - Read TweakDB file (`tweakdb.bin`)
   - Report file stats
   - Copy files
   - Simple byte patching (`--patch-byte`)
   - Hex dumping (`--dump-hex`)

3. **Verify macOS Cyberpunk 2077 compatibility**
   - Locate game TweakDB files on macOS
   - Make simple modifications
   - Confirm game accepts changes

### Immediate Next Steps

1. **Set up build system** (CMake or XMake)
2. **Create basic tweakdb-patcher skeleton**
3. **Implement file I/O for TweakDB**
4. **Add command-line argument parsing**
5. **Test with actual game files**

## Original TweakXL Architecture

### Key Components We'll Need to Port/Adapt

From the original TweakXL repository:

**Reusable (with adaptation):**
- `src/Red/TweakDB/Source/` - Parser for `.tweak` files
  - Grammar.hpp - PEGTL grammar
  - Parser.hpp/cpp - Builds tweak representation
  - Source.hpp - In-memory tweak objects
- `data/` - Metadata files

**Windows-specific (need alternatives):**
- `src/Red/TweakDB/Buffer.hpp` - Live TweakDB manipulation
- `src/Red/TweakDB/Manager.hpp` - Runtime manager
- RED4ext integration - DLL plugin system

### Build System

Original TweakXL uses:
- **XMake** for build configuration
- **C++** (98.5% of codebase)
- Dependencies:
  - RED4ext 1.28.0+ (Windows plugin system)
  - yaml-cpp (for YAML parsing)
  - PEGTL (for .tweak parsing)
  - TiltedCore (from vendor/)

For macOS offline patcher, we'll need:
- XMake or CMake
- yaml-cpp
- PEGTL
- Custom TweakDB file I/O (no RED4ext)

## Development Environment Setup

### Prerequisites

- macOS development machine
- Xcode Command Line Tools
- CMake or XMake
- C++17 or later compiler
- Cyberpunk 2077 macOS installation (for testing)

### TweakDB File Locations

On macOS Cyberpunk 2077:
```
[Game Installation]/Cyberpunk 2077.app/Contents/Resources/r6/cache/tweakdb.bin
[Game Installation]/Cyberpunk 2077.app/Contents/Resources/r6/cache/tweakdb_ep1.bin
```

**Note:** Exact paths need to be verified for macOS version.

## Architecture Decisions

### Why Offline Patcher?

1. **No RED4ext on macOS** - The plugin injection system is Windows-specific
2. **Simpler deployment** - No DLL management, just file patching
3. **Safer** - Changes applied before game launch, easy to revert
4. **Compatible** - Same file format on Windows and macOS

### Compatibility Goals

Our v1 will support:
- ✅ YAML tweaks (same format as Windows TweakXL)
- ✅ RED tweaks (where they affect TweakDB only)
- ✅ .tweak files (flat file format)
- ✅ Operations: Assign (=), Append (+=), Remove (-=)
- ⚠️  Record creation/inheritance (where needed by target mods)
- ❌ Runtime script extensions (future consideration)

## Testing Strategy

### Test Mods (Phase 1)

We'll validate compatibility with real mods:

1. **NIGHT CITY ALIVE**
   - Uses .tweak files
   - Modifies gang density, AI behavior
   - Good test of core functionality

2. **Additional YAML-based mod** (TBD)
   - Tests YAML parsing
   - Different operation types

3. **Simple example mod** (TBD)
   - Minimal test case
   - Easy to debug

## Questions to Answer

### Phase 0

- [ ] Where exactly are TweakDB files located in macOS Cyberpunk 2077?
- [ ] What's the file format structure? (We'll document in Phase 2)
- [ ] Can we make simple modifications that are visible in-game?

### Phase 1

- [ ] Which exact TweakXL features do our target mods use?
- [ ] What's the minimal viable feature set?
- [ ] Are there format differences between mod types?

## Resources

- [Original TweakXL](https://github.com/psiberx/cp2077-tweak-xl)
- [Mac Port Plan](mac-port-plan.md) - Detailed roadmap
- [PEGTL Documentation](https://github.com/taocpp/PEGTL)
- [yaml-cpp](https://github.com/jbeder/yaml-cpp)

## Next Steps

1. Choose build system (recommend CMake for macOS simplicity)
2. Set up basic project structure with CMakeLists.txt
3. Create `tweakdb-patcher` skeleton in `src/`
4. Implement basic file operations
5. Test with game files

Let's get started with Phase 0!
