# TweakXL-macOS Mod Examples

This directory contains example mod files demonstrating various TweakDB modification techniques for Cyberpunk 2077 on macOS.

## Example Mods

### 1. player_stats.yaml
**Purpose**: Demonstrates simple flat modifications

**Features**:
- Modifying player health, stamina, and armor
- Adjusting movement speed and combat stats
- Setting boolean flags for perks and abilities

**Use Case**: Basic stat tweaking for single-player gameplay

### 2. custom_weapons.yaml
**Purpose**: Demonstrates record creation and inheritance

**Features**:
- Creating new weapon records using `$type` and `$base`
- Customizing weapon properties (damage, range, crit stats)
- Modifying existing weapon presets

**Use Case**: Adding custom weapons or rebalancing existing ones

### 3. inventory_tweaks.yaml
**Purpose**: Demonstrates array operations

**Features**:
- `$append`: Adding items to arrays
- `$remove`: Removing items from arrays
- `$assign`: Replacing entire arrays
- Managing loot pools and vendor inventories

**Use Case**: Customizing what items appear in-game

### 4. difficulty_rebalance.yaml
**Purpose**: Comprehensive gameplay modification

**Features**:
- Adjusting enemy stats system-wide
- Modifying healing and crafting mechanics
- Economic rebalancing
- Skill progression tweaking
- Law enforcement behavior changes

**Use Case**: Creating a complete difficulty overhaul mod

## YAML Format Reference

### Simple Flats
```yaml
# Key: Value pairs modify single values
Items.Katana.damage: 100
Player.health: 500.5
Player.name: "Johnny"
Player.isAlive: true
```

### Arrays
```yaml
# Regular array assignment
Player.inventory:
  - item1
  - item2
  - item3

# Append to existing array
Player.skills:
  $append:
    - newSkill1
    - newSkill2

# Remove from array
Loot.unwanted:
  $remove:
    - junkItem

# Replace entire array
Vendor.stock:
  $assign:
    - weapon1
    - weapon2
```

### Records
```yaml
# Create new record with inheritance
Items.MyWeapon:
  $type: Weapon_Record
  $base: Items.Preset_Katana_Default
  damage: 150
  range: 2.5
  critChance: 0.45

# Record without base (creates from scratch)
CustomNPC.MyCharacter:
  $type: Character_Record
  name: "Custom Character"
  health: 1000
  faction: "Player"
```

## Type Detection

The YAML parser automatically detects value types:
- **Integers**: `100`, `-50`, `0`
- **Floats**: `1.5`, `3.14`, `-0.5`
- **Booleans**: `true`, `false`
- **Strings**: `"text"`, `text` (auto-detected)
- **Arrays**: YAML sequences (lists)

## Special Keys

Keys starting with `$` have special meaning:
- `$type`: Specifies record type (for new records)
- `$base`: Specifies parent record for inheritance
- `$append`: Appends to an array instead of replacing
- `$remove`: Removes items from an array
- `$assign`: Explicitly replaces an array (default behavior)

## File Structure

Mods can mix flats and records in the same file:
```yaml
# Flats
Player.health: 500

# Record
Items.MyWeapon:
  damage: 100

# More flats
Player.stamina: 300
```

## Usage with TweakXL-macOS

1. Place `.yaml` or `.yml` files in your mods directory
2. Run TweakXL-macOS to patch `tweakdb.bin`
3. Launch Cyberpunk 2077

```bash
tweakxl-mac --game-dir "/Applications/Cyberpunk 2077.app" \
            --mods-dir "~/Documents/Cyberpunk Mods" \
            --backup
```

## Compatibility

These mods are examples and use **fictional** TweakDB IDs. Real mods should reference actual game TweakDB entries. To find valid IDs:
1. Use community-created TweakDB dumps
2. Refer to existing Windows TweakXL mods
3. Use WolvenKit to explore game files

## Best Practices

1. **Use meaningful names**: `Items.MyCustomKatana` is better than `Items.Weapon1`
2. **Comment your mods**: Use `#` for comments explaining what each section does
3. **Test incrementally**: Start with small changes before making sweeping modifications
4. **Backup your game**: Always backup `tweakdb.bin` before patching
5. **One purpose per file**: Keep related changes together, but don't mix unrelated mods

## Creating Your Own Mods

1. Copy one of these examples as a template
2. Modify the TweakDB IDs to match actual game values
3. Adjust values to your preference
4. Test with TweakXL-macOS
5. Share with the community!

## Additional Resources

- [TweakDB Documentation](https://wiki.redmodding.org/cyberpunk-2077-modding/for-mod-creators-theory/tweaks/tweaks)
- [Windows TweakXL Mods](https://www.nexusmods.com/cyberpunk2077/mods/categories/58/) (for reference)
- [WolvenKit](https://github.com/WolvenKit/WolvenKit) (for exploring game files)

## License

These examples are provided as-is for educational purposes. Modify and share freely!
