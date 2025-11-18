# Mod Compatibility & User Experience Design

**Date:** November 2025
**Purpose:** Define how existing Windows TweakXL mods will work on macOS

---

## Core Philosophy

> **"Existing Windows TweakXL mods should work on macOS with ZERO changes wherever possible."**

Users should be able to:
1. Download a mod from Nexus Mods (Windows version)
2. Drop it in their mods folder
3. Run `tweakxl-mac`
4. Play with the mod working

**No manual conversion. No macOS-specific versions. Just works.**

---

## Mod Structure Analysis

### Typical Windows TweakXL Mod Layout

```
ModName/
├── r6/
│   └── tweaks/
│       ├── modname/
│       │   ├── settings.yaml
│       │   ├── weapons.yaml
│       │   └── npcs.tweak
│       └── modname.yaml
├── red4ext/
│   └── plugins/
│       └── modname.dll       # ← Won't work on macOS
└── readme.txt
```

### What We Support (v1.0)

```
ModName/
├── r6/
│   └── tweaks/              # ← YES! Extract these
│       ├── modname/
│       │   ├── settings.yaml    ✅ Parse
│       │   ├── weapons.yaml     ✅ Parse
│       │   └── npcs.tweak       ✅ Parse
│       └── modname.yaml         ✅ Parse
└── red4ext/
    └── plugins/
        └── modname.dll          ❌ Ignore (v1.0)
```

**Result:** ~80% of mods work because most mods are just YAML/tweak files!

---

## File Format Compatibility

### YAML Tweaks

**Windows TweakXL format:**
```yaml
# settings.yaml
PreventionSystem.setup.totalEntitiesLimit: 40

Items.Preset_Katana_Default:
  quality: Quality.Legendary
  mass: 20.0

Vehicle.v_sport2_quadra_type66_nomad:
  equipment:
    !append:
      - Vehicle.v_sport2_Equipment_Part_Extra
```

**macOS TweakXL-macOS:**
```yaml
# EXACTLY THE SAME! No changes needed.
PreventionSystem.setup.totalEntitiesLimit: 40

Items.Preset_Katana_Default:
  quality: Quality.Legendary
  mass: 20.0

Vehicle.v_sport2_quadra_type66_nomad:
  equipment:
    !append:
      - Vehicle.v_sport2_Equipment_Part_Extra
```

✅ **100% Compatible** - Same parser (yaml-cpp), same syntax

### .tweak Files

**Windows TweakXL format:**
```
# npcs.tweak
package NightCityAlive

gang.density = 2.5
ai.aggression = 1.8
traffic.multiplier = 1.5
```

**macOS TweakXL-macOS:**
```
# EXACTLY THE SAME! No changes needed.
package NightCityAlive

gang.density = 2.5
ai.aggression = 1.8
traffic.multiplier = 1.5
```

✅ **100% Compatible** - Same grammar (PEGTL from TweakXL source), same syntax

### RED Tweaks (.reds)

**Windows:**
```swift
// complex_mod.reds
// Script extension
```

**macOS v1.0:**
```
❌ Not supported (requires runtime)
```

**macOS v2.0:**
```
✅ Would be supported with RED4ext-mac
```

⚠️ **Partial Compatibility** - Script extensions need v2.0

---

## Mod Installation Methods

### Method 1: Manual Installation (Simple)

**User workflow:**
1. Download mod from Nexus Mods
2. Extract to `~/Cyberpunk2077Mods/`
3. Run tool:
   ```bash
   tweakxl-mac --mods-dir ~/Cyberpunk2077Mods
   ```

**Pros:**
- Simple for users
- Works with any mod source
- No special structure required

**Cons:**
- Manual download process
- No automatic updates

### Method 2: Windows Game Structure (Advanced)

For users who want to keep Windows mod structure:

**Setup:**
```bash
# Point to Windows-style game structure
tweakxl-mac --game-dir "/Applications/Cyberpunk 2077.app" \
            --mods-from-game-dir
```

**Tool scans:**
```
Cyberpunk 2077.app/Contents/Resources/
├── r6/
│   └── tweaks/          # ← Scan here for mods
│       ├── mod1/
│       ├── mod2/
│       └── mod3/
```

**Pros:**
- Familiar to Windows users
- Matches TweakXL exactly
- Easy migration

**Cons:**
- Modifying game bundle (macOS signatures)
- Less clean separation

### Method 3: Mod Manager Integration (Future)

**Vortex / mod manager support:**
```bash
# Mod manager calls our CLI
tweakxl-mac --mods-list mods.json --apply
```

**Future v1.5:**
- JSON-based mod list
- Programmatic interface
- Integration with mod managers

---

## Mod Discovery & Scanning

### What Gets Scanned?

**Pattern matching:**
```
Mods directory:
  ├── ModA/
  │   └── r6/tweaks/*.yaml    ✅ Found
  ├── ModB/
  │   └── r6/tweaks/*.tweak   ✅ Found
  ├── ModC/
  │   ├── info.json           ℹ️  Optional metadata
  │   └── r6/tweaks/*.yaml    ✅ Found
  └── some-random-file.txt    ❌ Ignored
```

**Recursive scanning:**
```cpp
class ModScanner {
    std::vector<Mod> ScanDirectory(const fs::path& dir) {
        std::vector<Mod> mods;

        // Recursively find all YAML and .tweak files
        for (auto& entry : fs::recursive_directory_iterator(dir)) {
            if (entry.path().extension() == ".yaml" ||
                entry.path().extension() == ".yml" ||
                entry.path().extension() == ".tweak") {

                mods.push_back(LoadMod(entry.path()));
            }
        }

        return mods;
    }
};
```

**Smart detection:**
- Looks for `r6/tweaks/` subdirectories
- Scans all `.yaml`, `.yml`, `.tweak` files
- Ignores non-mod files (readmes, images, etc.)
- Warns about `.reds` files (script extensions)

---

## Load Order & Conflict Resolution

### Problem: Multiple Mods Modifying Same Value

**Scenario:**
```yaml
# Mod A: weapons_buff.yaml
Items.Preset_Katana_Default:
  damage: 100

# Mod B: weapons_rebalance.yaml
Items.Preset_Katana_Default:
  damage: 50
```

**Which value wins?**

### Solution 1: Alphabetical (v1.0)

**Simple and predictable:**
```
Load order:
1. weapons_buff.yaml       → damage = 100
2. weapons_rebalance.yaml  → damage = 50 (WINS, last applied)
```

**Implementation:**
```cpp
// Sort mods alphabetically
std::sort(mods.begin(), mods.end(),
    [](const auto& a, const auto& b) {
        return a.path < b.path;
    });

// Apply in order (later mods override earlier)
for (const auto& mod : mods) {
    applicator.Apply(db, mod);
}
```

**User control:**
- Rename files: `01_base.yaml`, `02_tweaks.yaml`, `99_overrides.yaml`
- Clear and simple

### Solution 2: Explicit Load Order (v1.5)

**Config file:**
```yaml
# ~/.tweakxl-mac/load-order.yaml
load_order:
  - weapons_rebalance  # Load first (base)
  - weapons_buff       # Load last (overrides)
```

**CLI option:**
```bash
tweakxl-mac --load-order ~/.tweakxl-mac/load-order.yaml
```

### Solution 3: Smart Conflict Detection (v1.5)

**Warn users about conflicts:**
```
⚠️  Warning: Conflict detected!

Flat 'Items.Preset_Katana_Default:damage' modified by:
  1. weapons_buff.yaml → 100
  2. weapons_rebalance.yaml → 50 (ACTIVE)

Final value: 50

Recommendation: Review load order or remove conflicting mod.
```

**Implementation:**
```cpp
class ConflictDetector {
    void TrackModification(const TweakDBID& id,
                           const std::string& modName,
                           const Value& value) {
        if (modifications_[id].count > 1) {
            LogWarning("Conflict: {} modified by multiple mods", id);
            for (const auto& mod : modifications_[id].mods) {
                LogInfo("  - {} → {}", mod.name, mod.value);
            }
        }
    }
};
```

**v1.0 Decision:** Start with alphabetical, warn on conflicts

---

## Mod Metadata & Information

### Optional: info.json

**Mods can include metadata:**
```json
{
  "name": "Night City Alive",
  "version": "2.1.0",
  "author": "NightCityModder",
  "description": "Increases gang density and traffic",
  "requires": [],
  "conflicts": ["SimpleTrafficMod"],
  "website": "https://nexusmods.com/cyberpunk2077/mods/12345"
}
```

**Tool can display:**
```bash
$ tweakxl-mac --list-mods

Installed Mods:
1. Night City Alive v2.1.0 by NightCityModder
   - Increases gang density and traffic
   - Files: 5 YAML, 2 .tweak

2. Weapon Rebalance v1.0.3 by BalancePro
   ⚠️  Conflicts with: WeaponBuff
   - Files: 10 YAML
```

**Benefits:**
- Users know what's installed
- Conflict warnings
- Version tracking

**v1.0:** Optional, not required. Tool works without metadata.

---

## Error Handling & User Feedback

### When Mods Fail

**Scenario 1: Syntax Error in YAML**
```yaml
# broken.yaml
Items.BadItem
  damage 100   # Missing colon!
```

**Output:**
```
❌ Error: Failed to parse broken.yaml
   Line 2: Expected ':' after property name

   Skipping this mod and continuing with others.
   Fix the YAML syntax and try again.
```

**Behavior:** Skip broken mod, continue with others

### Scenario 2: Referenced TweakDBID Doesn't Exist

```yaml
# bad_reference.yaml
Items.NonExistentItem:
  damage: 100
```

**Output:**
```
⚠️  Warning: broken_reference.yaml
   TweakDBID 'Items.NonExistentItem' not found in TweakDB.

   This modification will be skipped.
   Check if mod is compatible with your game version.
```

**Behavior:** Warn but continue (mod might add it dynamically on Windows)

### Scenario 3: Type Mismatch

```yaml
# wrong_type.yaml
PreventionSystem.setup.totalEntitiesLimit: "forty"  # Should be int!
```

**Output:**
```
❌ Error: wrong_type.yaml
   Property 'totalEntitiesLimit' expects Int, got String ("forty")

   Skipping this modification.
```

**Behavior:** Skip invalid modification, continue with rest of mod

### Scenario 4: Script Extension Detected

```
ModFolder/
└── red4ext/plugins/something.dll
```

**Output:**
```
ℹ️  Info: Mod 'MyComplexMod' contains script extensions (red4ext/)

   Script extensions are not supported in v1.0 (offline patching only).
   The YAML/tweak portions of this mod will still work.

   For full compatibility, wait for v2.0 (runtime hooking).
```

**Behavior:** Process YAML/tweak files, warn about unsupported features

---

## Validation & Dry Run

### Preview Before Applying

**Dry run mode:**
```bash
tweakxl-mac --mods-dir ~/mods --dry-run
```

**Output:**
```
📋 Preview: What would change

Mods to apply (3):
  1. Night City Alive v2.1.0
  2. Weapon Rebalance v1.0.3
  3. Economy Tweaks v0.5

Changes to TweakDB:
  ✏️  Modified flats: 47
  ➕ Added records: 3
  🔧 Array operations: 12

Conflicts detected: 1
  ⚠️  Items.Preset_Katana_Default:damage
     - Weapon Rebalance → 50
     - Weapon Buff → 100 (ACTIVE)

Run without --dry-run to apply changes.
```

**Benefits:**
- Users see what will happen
- Catch conflicts before applying
- Safe experimentation

---

## Backup & Restore

### Automatic Backups

**Before any modification:**
```bash
tweakxl-mac --mods-dir ~/mods
```

**Tool automatically:**
```
Creating backup: ~/.tweakxl-mac/backups/tweakdb_2025-11-17_14-30-22.bin
Applying 3 mods...
✅ Done! Modified TweakDB written.

To restore: tweakxl-mac --restore ~/.tweakxl-mac/backups/tweakdb_2025-11-17_14-30-22.bin
```

**Backup management:**
```bash
# List backups
tweakxl-mac --list-backups

# Restore specific backup
tweakxl-mac --restore <backup-file>

# Restore vanilla (remove all mods)
tweakxl-mac --restore-vanilla
```

**Implementation:**
```cpp
class BackupManager {
    fs::path CreateBackup(const fs::path& tweakdb) {
        auto timestamp = GetTimestamp(); // 2025-11-17_14-30-22
        auto backupPath = GetBackupDir() / ("tweakdb_" + timestamp + ".bin");

        fs::copy(tweakdb, backupPath);

        // Keep last N backups, delete old ones
        CleanOldBackups(10);

        return backupPath;
    }
};
```

---

## Migration from Windows

### User Coming from Windows

**Their setup on Windows:**
```
C:\Games\Cyberpunk 2077\
└── r6\tweaks\
    ├── NightCityAlive\
    ├── WeaponRebalance\
    └── EconomyTweaks\
```

**On macOS Option 1: Extract mods**
```bash
# Copy mod folders to macOS
mkdir ~/Cyberpunk2077Mods
cp -r /Volumes/Windows/Games/Cyberpunk\ 2077/r6/tweaks/* ~/Cyberpunk2077Mods/

# Use with tool
tweakxl-mac --mods-dir ~/Cyberpunk2077Mods
```

**On macOS Option 2: Point to game structure**
```bash
# If they set up mods in macOS game bundle
tweakxl-mac --game-dir "/Applications/Cyberpunk 2077.app" \
            --mods-from-game-dir
```

**Compatibility:** 100% - Same files, same structure, same syntax!

---

## Testing Strategy for Existing Mods

### Test Suite: Popular Mods

**Must work perfectly (v1.0 targets):**
1. **NIGHT CITY ALIVE** - Traffic/gang density
   - Format: .tweak files
   - Complexity: Medium (arrays, flats)
   - Popularity: Very high

2. **Weapon/Armor Stat Mods**
   - Format: YAML
   - Complexity: Low (simple value changes)
   - Popularity: High

3. **Economy Rebalance**
   - Format: YAML
   - Complexity: Medium (many flats)
   - Popularity: Medium

**Should work with warnings:**
4. **Complex Mods with Scripts**
   - Format: YAML + .reds
   - Behavior: YAML works, scripts ignored
   - Warning: "Script extensions not supported"

### Compatibility Matrix

| Mod Category | v1.0 Support | Notes |
|--------------|--------------|-------|
| Pure YAML tweaks | ✅ 100% | Full support |
| Pure .tweak files | ✅ 100% | Full support |
| YAML + .tweak mix | ✅ 100% | Full support |
| Script extensions only | ❌ 0% | Need v2.0 |
| YAML + script mix | ⚠️ Partial | YAML works, scripts ignored |
| RED tweaks (.reds format) | ❓ TBD | Not .tweak! Need research |

---

## Documentation for Mod Users

### Quick Start Guide

**Create: `docs/user-guide.md`**

```markdown
# TweakXL-macOS User Guide

## Installing Mods

### Step 1: Download Mods
- Visit Nexus Mods (Cyberpunk 2077 section)
- Download Windows TweakXL mods (yes, Windows mods!)
- Extract to `~/Cyberpunk2077Mods/`

### Step 2: Apply Mods
```bash
tweakxl-mac --mods-dir ~/Cyberpunk2077Mods
```

### Step 3: Play!
Launch Cyberpunk 2077. Your mods are active.

## Troubleshooting

**Mod not working?**
- Check for syntax errors: `tweakxl-mac --validate`
- Try dry run: `tweakxl-mac --dry-run`
- Check compatibility: Does it use script extensions?

**Game crashes?**
- Restore backup: `tweakxl-mac --restore-vanilla`
- Test mods individually
- Report issues with mod details
```

---

## CLI Design for Usability

### User-Friendly Commands

**Basic usage (simple):**
```bash
# Apply all mods
tweakxl-mac

# Apply from specific directory
tweakxl-mac --mods-dir ~/mods
```

**Advanced usage:**
```bash
# Preview changes
tweakxl-mac --dry-run

# List installed mods
tweakxl-mac --list-mods

# Validate mods without applying
tweakxl-mac --validate

# Verbose output
tweakxl-mac --verbose

# Restore vanilla
tweakxl-mac --restore-vanilla
```

**Configuration file (`~/.tweakxl-mac/config.yaml`):**
```yaml
game_dir: "/Applications/Cyberpunk 2077.app"
mods_dir: "~/Cyberpunk2077Mods"
backup_dir: "~/.tweakxl-mac/backups"
max_backups: 10
conflict_resolution: "warn"  # or "error", "silent"
```

---

## Open Questions for Discussion

### 1. Default Mods Location

**Option A:** User-specified every time
```bash
tweakxl-mac --mods-dir ~/mods  # Always required
```

**Option B:** Smart defaults
```bash
tweakxl-mac  # Looks in ~/Cyberpunk2077Mods/ by default
```

**Option C:** Config file
```bash
# First time setup
tweakxl-mac --setup

# Then just
tweakxl-mac
```

**Recommendation:** Option C with Option B fallback

### 2. Conflict Resolution Strategy

**Option A:** Last-loaded wins (simple)
- Alphabetical order
- No warnings

**Option B:** Warn on conflicts (recommended)
- Last-loaded wins
- Show warnings
- Let user decide

**Option C:** Error on conflicts (strict)
- Refuse to apply if conflicts
- User must resolve manually

**Recommendation:** Option B for v1.0

### 3. Script Extension Handling

**Option A:** Ignore silently
- Process YAML/tweak only
- No warnings

**Option B:** Warn user (recommended)
- Process YAML/tweak
- Inform about unsupported features
- Suggest v2.0

**Option C:** Error and refuse
- Don't apply mod at all if has scripts
- User must remove scripts

**Recommendation:** Option B for v1.0

### 4. Mod Metadata

**Option A:** No metadata support (simple)
- Just scan files
- No mod info

**Option B:** Optional metadata (recommended)
- Support info.json if present
- Works without it
- Nice UX when available

**Option C:** Required metadata
- All mods must have info.json
- Strict validation

**Recommendation:** Option B for v1.0

---

## Summary: Design Decisions

### ✅ Confirmed Decisions

1. **100% file format compatibility** with Windows TweakXL
   - Same YAML syntax
   - Same .tweak syntax
   - No conversion needed

2. **Flexible mod locations**
   - Support Windows-style structure
   - Support standalone mod folders
   - Config file for defaults

3. **Alphabetical load order** (v1.0)
   - Simple and predictable
   - User control via renaming
   - Explicit load order in v1.5

4. **Warn on conflicts**
   - Last-loaded wins
   - Show warnings to user
   - Let user decide resolution

5. **Graceful error handling**
   - Skip broken mods, continue with others
   - Clear error messages
   - Validate mode for debugging

6. **Automatic backups**
   - Before every modification
   - Easy restore
   - Keep last N backups

7. **Dry run mode**
   - Preview before applying
   - Show conflicts
   - Safe experimentation

8. **Script extension warnings**
   - Process YAML/tweak portions
   - Warn about unsupported features
   - Inform about v2.0

### 🎯 Design Goals Achieved

- ✅ **Zero mod conversion** - Windows mods work as-is
- ✅ **Simple for basic users** - Just drop mods in folder
- ✅ **Powerful for advanced** - Config, load order, validation
- ✅ **Safe by default** - Backups, dry run, restore
- ✅ **Clear feedback** - Warnings, errors, conflict detection

---

## Next Steps

1. **Implement ModScanner** (Phase 3)
2. **Implement ModLoader** (Phase 3)
3. **Implement BackupManager** (Phase 6)
4. **Implement CLI interface** (Phase 6)
5. **Test with real mods** (Phase 7)

All design decisions documented here will guide implementation.
