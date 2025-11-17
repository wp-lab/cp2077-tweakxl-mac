# TweakXL-macOS: Architecture Overview

**Date:** November 2025
**Version:** 2.0

---

## Multi-Layer Architecture

TweakXL-macOS uses a **layered approach** where offline patching provides the base layer and optional runtime hooking provides enhancement.

### Why Layered?

- **Separation of concerns**: Static vs. dynamic modifications
- **Performance**: Heavy work done once offline
- **Incremental development**: Ship v1.0 without waiting for v2.0
- **User choice**: Simple users don't need runtime complexity

---

## Layer 1: Offline Patching (v1.0)

### Overview

Modifies TweakDB binary files on disk **before** game launch.

```
┌─────────────────────────────────────────────────────┐
│                  Before Game Launch                  │
└─────────────────────────────────────────────────────┘
                         │
                         ▼
            ┌────────────────────────┐
            │   User runs command:   │
            │                        │
            │  tweakxl-mac           │
            │    --game-dir ...      │
            │    --mods-dir ~/mods   │
            └────────────────────────┘
                         │
                         ▼
        ┌────────────────────────────────┐
        │  Offline Patcher (tweakxl-mac) │
        ├────────────────────────────────┤
        │  1. Read tweakdb.bin           │
        │  2. Parse YAML/.tweak mods     │
        │  3. Apply modifications        │
        │  4. Write modded tweakdb.bin   │
        └────────────────────────────────┘
                         │
                         ▼
        ┌────────────────────────────────┐
        │  File System                   │
        ├────────────────────────────────┤
        │  r6/cache/                     │
        │    ├── tweakdb.bin (original)  │
        │    └── modded/                 │
        │        └── tweakdb.bin ✓       │
        └────────────────────────────────┘
                         │
                         ▼
               Game launches with
              pre-modified TweakDB
```

### Components

**Offline Patcher CLI (`tweakxl-mac`):**

```cpp
class OfflinePatcher {
public:
    // Main workflow
    void Patch(const Config& config) {
        // 1. Load original TweakDB
        auto db = TweakDBReader::Load(config.gameDir / "r6/cache/tweakdb.bin");

        // 2. Load all mods
        auto mods = ModLoader::LoadFromDirectory(config.modsDir);

        // 3. Apply modifications
        ModificationApplicator applicator;
        for (const auto& mod : mods) {
            applicator.Apply(db, mod);
        }

        // 4. Write modified TweakDB
        TweakDBWriter::Save(db, config.gameDir / "r6/cache/modded/tweakdb.bin");
    }
};
```

### Supported Mods (v1.0)

✅ **Static Data Modifications:**
- YAML tweaks (`.yaml`, `.yml`)
- .tweak flat files
- Operations: Assign, Append, Remove
- Record property changes
- Array manipulations

✅ **Popular Mod Examples:**
- NIGHT CITY ALIVE (gang density, traffic)
- Weapon/armor stat changes
- Economy tweaks (prices, rewards)
- Difficulty adjustments
- Vehicle modifications

❌ **Not Supported (need runtime):**
- Script extensions (redscript)
- Dynamic behavior hooks
- Hot reloading
- UI modifications beyond data

### When to Use

**Perfect for:**
- Users who want simple mod installation
- Static gameplay tweaks
- Most popular TweakXL mods (80%+)

**Advantages:**
- No game modification needed
- Safe and reversible (backup/restore)
- Works without disabling security features
- No ongoing maintenance with game updates

---

## Layer 2: Runtime Hooking (v2.0+)

### Overview

Optional enhancement that injects into game process at runtime.

```
┌─────────────────────────────────────────────────────┐
│              Game Launch (Runtime)                   │
└─────────────────────────────────────────────────────┘
                         │
                         ▼
        ┌────────────────────────────────┐
        │  Game Process Starts           │
        ├────────────────────────────────┤
        │  1. Load tweakdb.bin           │
        │     (already modified offline) │
        │                                │
        │  2. Initialize TweakDB         │
        │     in memory                  │
        └────────────────────────────────┘
                         │
                         ▼
        ┌────────────────────────────────┐
        │  RED4ext-mac Injection         │
        ├────────────────────────────────┤
        │  - dylib injection             │
        │  - Hook TweakDB functions      │
        │  - Access game RTTI            │
        └────────────────────────────────┘
                         │
                         ▼
        ┌────────────────────────────────┐
        │  Script Extension Loader       │
        ├────────────────────────────────┤
        │  - Load redscript mods         │
        │  - Register new functions      │
        │  - Apply dynamic tweaks        │
        └────────────────────────────────┘
                         │
                         ▼
        Game runs with both:
        - Offline base modifications
        - Runtime dynamic enhancements
```

### Components

**RED4ext-mac Framework:**

```cpp
// Conceptual API (v2.0 design, not implemented yet)
namespace RED4ext {
    class TweakDBHook {
    public:
        // Hook into game's TweakDB initialization
        static void InstallHooks();

        // Called when TweakDB is initialized
        static void OnTweakDBInit(Red::TweakDB* db) {
            // TweakDB here already has offline modifications!

            // Apply runtime modifications on top
            LoadScriptExtensions(db);
            RegisterDynamicTweaks(db);
        }
    };
}
```

### Supported Mods (v2.0)

✅ **Everything from v1.0, plus:**
- Script extensions (redscript integration)
- Dynamic behavior hooks
- Hot reloading (developer feature)
- Runtime TweakDB queries
- Complex mod interactions

✅ **Example Advanced Mods:**
- AI behavior overhauls
- New game systems
- Dynamic quest logic
- UI modifications

### When to Use

**Perfect for:**
- Advanced users
- Mod developers
- Complex behavior mods
- Full Windows TweakXL compatibility

**Trade-offs:**
- Requires more complex setup
- May need security workarounds
- Ongoing maintenance with updates
- Development timeline: 12-18 months

---

## Combined Architecture

### Full System Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                        User's Mac                                │
│                                                                  │
│  ┌────────────────────────────────────────────────────────┐    │
│  │  ~/mods/                                               │    │
│  │    ├── offline/  (YAML, .tweak files)                  │    │
│  │    └── runtime/  (Script extensions)                   │    │
│  └────────────────────────────────────────────────────────┘    │
│           │                               │                     │
│           │ Processed offline             │ Loaded at runtime  │
│           ▼                               ▼                     │
│  ┌──────────────────┐          ┌──────────────────┐           │
│  │ tweakxl-mac      │          │ RED4ext-mac      │           │
│  │ (Offline Tool)   │          │ (Runtime Hook)   │           │
│  └──────────────────┘          └──────────────────┘           │
│           │                               │                     │
│           ▼                               │                     │
│  ┌────────────────────────────────────────┼──────────────┐    │
│  │  Cyberpunk 2077.app                   │              │    │
│  │                                        │              │    │
│  │  r6/cache/modded/tweakdb.bin ◄────────┘              │    │
│  │  (Pre-modified on disk)               │              │    │
│  │                                        │              │    │
│  │  ┌──────────────────────────────────┐ │              │    │
│  │  │  Game Process                    │ │              │    │
│  │  │                                  │ │              │    │
│  │  │  ┌────────────────────────────┐ │ │              │    │
│  │  │  │  TweakDB (in memory)       │ │ │              │    │
│  │  │  │                            │◄┼─┼──────────────┘    │
│  │  │  │  Base: Offline mods        │ │ │  Runtime mods    │
│  │  │  │  + Runtime enhancements    │ │ │  injected here   │
│  │  │  └────────────────────────────┘ │ │                   │
│  │  └──────────────────────────────────┘ │                   │
│  └────────────────────────────────────────┘                   │
└─────────────────────────────────────────────────────────────────┘
```

### Data Flow

**Step 1: Offline Patching (v1.0)**
```
Mod files (YAML/.tweak)
    → Parse
    → Build modification list
    → Apply to TweakDB binary
    → Write modded/tweakdb.bin
```

**Step 2: Game Launch**
```
Game starts
    → Loads modded/tweakdb.bin
    → TweakDB initialized with offline mods already applied
```

**Step 3: Runtime Injection (v2.0)**
```
RED4ext-mac injects
    → Hooks TweakDB functions
    → Loads script extensions
    → Applies dynamic modifications ON TOP of offline base
```

### Mod Compatibility Matrix

| Mod Type | v1.0 Only | v1.0 + v2.0 |
|----------|-----------|-------------|
| Static tweaks (YAML) | ✅ | ✅ |
| .tweak files | ✅ | ✅ |
| Array operations | ✅ | ✅ |
| Record modifications | ✅ | ✅ |
| Script extensions | ❌ | ✅ |
| Hot reload | ❌ | ✅ |
| Dynamic hooks | ❌ | ✅ |
| UI modifications | ❌ | ✅ |

---

## Development Roadmap

### v1.0: Offline Patching (Current Focus)

**Timeline:** 15 weeks (part-time)

**Phases:**
1. Foundation (2 weeks)
2. TweakDB Reader (2 weeks)
3. Mod Parsers (2 weeks)
4. Bridge Layer (2 weeks)
5. TweakDB Writer (2 weeks)
6. CLI Tool (2 weeks)
7. Testing (2 weeks)
8. Release (1 week)

**Deliverable:** `tweakxl-mac` CLI tool

### v1.5: Stabilization (Ongoing)

**Timeline:** 2-4 weeks

**Focus:**
- Bug fixes from community feedback
- Documentation improvements
- Additional mod testing
- Performance optimization

### v2.0: Runtime Hooking (Future)

**Timeline:** 12-18 months (separate project)

**Prerequisites:**
- v1.0 stable and adopted
- Community demand confirmed
- Expert volunteer(s) with macOS reverse engineering skills

**Phases:**
1. Reverse engineering (3-6 months)
   - Analyze macOS game binary
   - Map game functions
   - Document RTTI structure

2. Injection framework (2-3 months)
   - dylib injection mechanism
   - Code signing workarounds
   - Hook infrastructure

3. Script extension support (2-3 months)
   - Redscript integration
   - API mapping
   - Runtime TweakDB access

4. Testing & stabilization (2-3 months)
   - Game update compatibility
   - Memory safety
   - Performance profiling

**Deliverable:** `red4ext-mac` framework

### Decision Points

**Should we proceed with v2.0?**

Evaluate after v1.0 ships:
- Community adoption level?
- Demand for script extensions?
- Available expertise?
- Apple's security stance?

**Possible outcomes:**
- ✅ Proceed with v2.0 (high demand + expertise available)
- ⏸️ Defer to v3.0 (low demand or technical blockers)
- 🤝 Community fork (another team takes it on)
- ❌ Stay with v1.0 only (sufficient for most users)

---

## Why This Architecture Works

### 1. Pragmatic Phasing

**v1.0 delivers value NOW:**
- Don't wait 18 months for perfection
- 80% solution in 15 weeks
- Users can mod their games TODAY

**v2.0 adds value LATER:**
- Only if needed
- Only if feasible
- Doesn't block v1.0

### 2. Layered Approach

**Each layer is independent:**
- v1.0 works standalone
- v2.0 enhances, doesn't replace
- No forced complexity

**Each layer has clear purpose:**
- v1.0: Static data (most mods)
- v2.0: Dynamic behavior (advanced mods)

### 3. User Choice

**Simple users:**
- Install v1.0 only
- Use YAML/.tweak mods
- No security workarounds needed

**Advanced users:**
- Install v1.0 + v2.0
- Use all mod types
- Accept additional complexity

### 4. Technical Synergy

**Runtime benefits from offline:**
- Offline does heavy lifting once
- Runtime only handles dynamic stuff
- Better performance than runtime-only

**Offline benefits from runtime:**
- Clear scope (static only)
- Simpler implementation
- Easier to test and maintain

---

## Comparison with Windows TweakXL

### Windows TweakXL

**Architecture:** Runtime-only
- RED4ext plugin (DLL injection)
- Parses mods at game launch
- Modifies TweakDB in memory
- All mods require runtime component

### TweakXL-macOS

**Architecture:** Hybrid (offline + optional runtime)
- v1.0: Offline patching (standalone)
- v2.0: Runtime hooking (optional)
- Static mods processed once
- Dynamic mods at runtime only

### Advantages of Hybrid

| Aspect | Windows (Runtime Only) | macOS (Hybrid) |
|--------|------------------------|----------------|
| **Setup** | Medium complexity | Simple (v1.0) / Complex (v2.0) |
| **Performance** | Parse mods every launch | Parse once offline |
| **Security** | Requires admin/mods | v1.0: No workarounds |
| **Maintenance** | Update with every patch | v1.0: Independent |
| **Compatibility** | 100% of mods | 80% (v1.0) / 100% (v1.0+v2.0) |

---

## Conclusion

**TweakXL-macOS uses a layered architecture:**

1. **Layer 1 (v1.0):** Offline patching - Simple, safe, covers most use cases
2. **Layer 2 (v2.0):** Runtime hooking - Advanced, optional, full compatibility

**This approach:**
- ✅ Delivers value quickly (v1.0 in 15 weeks)
- ✅ Enables future enhancements (v2.0 when ready)
- ✅ Gives users choice (simple vs. advanced)
- ✅ Maximizes performance (right tool for each job)

**Next step:** Build v1.0 (offline patching)

**Future consideration:** Build v2.0 (runtime hooking) based on community feedback

---

## References

- [RESEARCH_FINDINGS.md](RESEARCH_FINDINGS.md) - Background research
- [REVISED_PLAN.md](REVISED_PLAN.md) - Implementation plan
- [Windows TweakXL](https://github.com/psiberx/cp2077-tweak-xl) - Original implementation
- [RED4ext](https://github.com/WopsS/RED4ext) - Windows runtime framework
