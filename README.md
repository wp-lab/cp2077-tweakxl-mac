# TweakXL-macOS

> **Offline TweakDB patcher for macOS Cyberpunk 2077**

Enable existing Windows TweakXL mods to work on macOS Cyberpunk 2077 through pre-game binary patching.

---

## 🎮 What is This?

**TweakXL-macOS** is a command-line tool that modifies Cyberpunk 2077's TweakDB (game configuration database) on disk, allowing you to use popular mods like **NIGHT CITY ALIVE** on macOS.

### Why is this needed?

The original [TweakXL](https://github.com/psiberx/cp2077-tweak-xl) for Windows uses **runtime hooking** (RED4ext DLL injection), which doesn't work on macOS. This project takes a different approach: **offline patching** - modifying the game files before launch.

---

## 📊 Project Status

**Current Phase:** Research Complete → Foundation (Phase 1)

✅ **Completed:**
- Comprehensive modding ecosystem research
- TweakDB binary format analysis
- TweakXL architecture study
- Detailed implementation plan

🚧 **In Progress:**
- Project structure setup
- CMake configuration
- Core data structures

📋 **Roadmap:** See [REVISED_PLAN.md](docs/REVISED_PLAN.md)

---

## 🔍 Background

### Cyberpunk 2077 on macOS

Cyberpunk 2077: Ultimate Edition was released natively for macOS on **July 17, 2025**:
- Native Apple Silicon support (M1+)
- Uses Metal 3 rendering
- Full cross-platform save support
- Same TweakDB file format as Windows

### TweakDB & Modding

**TweakDB** is Cyberpunk 2077's proprietary configuration database containing:
- Game entity properties (NPCs, vehicles, items, etc.)
- Gameplay parameters (difficulty, AI behavior, etc.)
- Economy settings (prices, rewards, etc.)

**TweakXL** lets modders change these values using friendly file formats:
- **YAML tweaks** - Easy-to-read configuration files
- **.tweak files** - Flat value definitions

---

## 🎯 Goals

### v1.0 Target

- ✅ **Read** vanilla `tweakdb.bin` files
- ✅ **Parse** YAML and .tweak mod files
- ✅ **Apply** modifications (Assign, Append, Remove operations)
- ✅ **Write** modified TweakDB back to disk
- ✅ **Compatible** with 80%+ of existing Windows TweakXL mods

### Test Cases

1. **NIGHT CITY ALIVE** - Gang density and AI behavior mod
2. Equipment/weapon stat modifications
3. YAML-based gameplay tweaks

---

## 📚 Documentation

| Document | Description |
|----------|-------------|
| [RESEARCH_FINDINGS.md](docs/RESEARCH_FINDINGS.md) | Comprehensive research: TweakDB format, modding ecosystem, tools |
| [REVISED_PLAN.md](docs/REVISED_PLAN.md) | Detailed 8-phase implementation plan (15 weeks) |
| [ARCHITECTURE.md](docs/ARCHITECTURE.md) | Layered architecture: Offline patching + optional runtime hooking |
| [MOD_COMPATIBILITY.md](docs/MOD_COMPATIBILITY.md) | Mod compatibility design: Windows mods work with zero changes |
| [getting-started.md](docs/getting-started.md) | Development environment setup |
| [mac-port-plan.md](docs/mac-port-plan.md) | Original high-level plan (superseded by REVISED_PLAN) |

---

## 🛠️ Technology Stack

- **Language:** C++17
- **Build System:** CMake 3.15+
- **Dependencies:**
  - [yaml-cpp](https://github.com/jbeder/yaml-cpp) - YAML parsing
  - [PEGTL](https://github.com/taocpp/PEGTL) - .tweak file grammar
  - [spdlog](https://github.com/gabime/spdlog) - Logging
  - [CLI11](https://github.com/CLIUtils/CLI11) - Command-line interface
  - [Catch2](https://github.com/catchorg/Catch2) - Testing

---

## 🚀 Building

**Status:** Not yet available (Phase 1 in progress)

Once ready:
```bash
git clone https://github.com/YOUR_USERNAME/cp2077-tweakxl-mac.git
cd cp2077-tweakxl-mac
mkdir build && cd build
cmake ..
make
```

---

## 📖 Usage

**Status:** Coming in Phase 6

Planned interface:
```bash
# Apply mods from directory
tweakxl-mac --game-dir "/Applications/Cyberpunk 2077.app" --mods-dir ~/mods

# Dry run (preview changes)
tweakxl-mac --game-dir "/Applications/Cyberpunk 2077.app" --mods-dir ~/mods --dry-run

# Backup and restore
tweakxl-mac --backup
tweakxl-mac --restore
```

---

## 🤝 Contributing

This is an active development project. We welcome:
- **Bug reports** - If you find issues
- **Testing** - Especially if you have macOS Cyberpunk 2077
- **Documentation** - Help improve guides
- **Code** - PRs welcome (see [REVISED_PLAN.md](docs/REVISED_PLAN.md) for architecture)

### Development Setup

See [docs/getting-started.md](docs/getting-started.md)

---

## 📋 Roadmap

| Phase | Status | Description |
|-------|--------|-------------|
| 0. Research | ✅ Complete | Understand formats, ecosystem, constraints |
| 1. Foundation | 🚧 In Progress | Project setup, test data, core structures |
| 2. TweakDB Reader | ⏳ Planned | Parse binary format |
| 3. Mod Parsers | ⏳ Planned | Parse YAML and .tweak files |
| 4. Bridge Layer | ⏳ Planned | Apply modifications |
| 5. TweakDB Writer | ⏳ Planned | Write modified binary |
| 6. CLI Tool | ⏳ Planned | User interface |
| 7. Testing | ⏳ Planned | Real mods, in-game validation |
| 8. Release | ⏳ Planned | Documentation, v1.0 |

**Estimated Timeline:** 15 weeks part-time (see [REVISED_PLAN.md](docs/REVISED_PLAN.md))

---

## 🔬 Research

This project is built on research into:
- **WolvenKit** - Open-source C# TweakDB parser
- **Gibbed.RED4** - RED4 engine tools
- **TweakXL** - Windows runtime implementation
- **Community Wikis** - RedModding documentation

Key insight: **Offline patching works!** TweakDB is just a binary file we can read, modify, and write back.

---

## ⚠️ Limitations (v1.0)

**Supported:**
- ✅ YAML tweaks (`.yaml`, `.yml`)
- ✅ .tweak flat files
- ✅ Operations: Assign, Append, Remove
- ✅ Most data types (int, float, bool, string, CName, arrays)

**Not Supported:**
- ❌ Script extensions (requires runtime access)
- ❌ Hot reloading (requires runtime access)
- ❌ Complex record inheritance (maybe v2)
- ❌ RED tweak format (maybe v1.x)

---

## 📄 License

*To be determined* (likely MIT to match original TweakXL)

---

## 🙏 Credits

- **[psiberx](https://github.com/psiberx)** - Original TweakXL creator
- **[gibbed](https://github.com/gibbed)** - RED4 reverse engineering
- **[WolvenKit Team](https://github.com/WolvenKit)** - Open-source modding tools
- **RedModding Community** - Documentation and support

---

## 📬 Contact

- **Issues:** [GitHub Issues](https://github.com/wp-lab/cp2077-tweakxl-mac/issues)
- **Discussions:** [GitHub Discussions](https://github.com/wp-lab/cp2077-tweakxl-mac/discussions) (if enabled)

---

**Note:** This is an independent community project, not affiliated with CD Projekt Red or the original TweakXL developer.
