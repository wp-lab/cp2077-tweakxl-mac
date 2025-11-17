#include <CLI/CLI.hpp>
#include <spdlog/spdlog.h>
#include <iostream>

int main(int argc, char** argv) {
    CLI::App app{"TweakXL-macOS - Offline TweakDB patcher for macOS Cyberpunk 2077"};

    // Version
    app.set_version_flag("--version", "1.0.0");

    // Options
    std::string gameDir;
    std::string modsDir;
    bool dryRun = false;
    bool verbose = false;

    app.add_option("--game-dir", gameDir, "Path to Cyberpunk 2077.app");
    app.add_option("--mods-dir", modsDir, "Path to mods directory");
    app.add_flag("--dry-run", dryRun, "Preview changes without applying");
    app.add_flag("-v,--verbose", verbose, "Verbose output");

    CLI11_PARSE(app, argc, argv);

    // Set up logging
    if (verbose) {
        spdlog::set_level(spdlog::level::debug);
    } else {
        spdlog::set_level(spdlog::level::info);
    }

    spdlog::info("TweakXL-macOS v1.0.0");
    spdlog::info("Phase 1 (Foundation) - Basic setup complete");

    if (!gameDir.empty()) {
        spdlog::info("Game directory: {}", gameDir);
    }

    if (!modsDir.empty()) {
        spdlog::info("Mods directory: {}", modsDir);
    }

    if (dryRun) {
        spdlog::info("Dry run mode - no changes will be made");
    }

    // TODO: Implement actual patching logic in future phases
    spdlog::warn("Full functionality coming in Phase 2-6");
    spdlog::info("Current status: Project builds successfully!");

    return 0;
}
