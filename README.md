# TweakXL-macOS

A macOS port of [TweakXL](https://github.com/psiberx/cp2077-tweak-xl) that enables running existing Windows TweakXL mods on macOS Cyberpunk 2077.

## Project Status

🚧 **In Development** - Phase 0 (Bootstrap)

This project aims to bring TweakXL mod support to macOS Cyberpunk 2077 using an offline patching approach.

## What is TweakXL?

TweakXL is a powerful modding framework for Cyberpunk 2077 that allows modders to modify game behavior by editing TweakDB - the game's configuration database. It supports:

- YAML tweak files
- RED tweak files
- `.tweak` flat files
- Operations like Assign, Append, Remove
- Record creation and inheritance

## Why macOS Port?

While the original TweakXL works on Windows via RED4ext plugin injection, macOS requires a different approach. This project provides an **offline patcher** that modifies TweakDB files on disk before the game launches.

## Goal

Build a macOS version that is **as compatible as possible** with existing Windows TweakXL mods, requiring minimal to no changes for mod authors.

## Documentation

- [Mac Port Plan](docs/mac-port-plan.md) - Detailed development roadmap and technical plan

## Building

*Coming soon - Phase 0 in progress*

## Usage

*Coming soon - Phase 7*

## Contributing

This is an active development project. See the [Mac Port Plan](docs/mac-port-plan.md) for current status and roadmap.

## License

*TBD*

## Credits

- Original TweakXL by [psiberx](https://github.com/psiberx)
- TweakXL-macOS port development
