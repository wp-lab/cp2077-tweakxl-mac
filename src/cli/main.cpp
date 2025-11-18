#include <CLI/CLI.hpp>
#include <spdlog/spdlog.h>
#include <iostream>
#include <filesystem>
#include "../tweakdb/Reader.hpp"
#include "../tweakdb/Writer.hpp"
#include "../tweakdb/Database.hpp"
#include "../parsers/ModLoader.hpp"
#include "../bridge/Applicator.hpp"

namespace fs = std::filesystem;
using namespace TweakXL;

// Helper to find tweakdb.bin in game directory
std::optional<String> FindTweakDB(const String& gameDir) {
    // macOS: Cyberpunk 2077.app/Contents/Resources/Data/r6/cache/tweakdb.bin
    fs::path gamePath(gameDir);
    fs::path tweakdbPath = gamePath / "Contents" / "Resources" / "Data" / "r6" / "cache" / "tweakdb.bin";

    if (fs::exists(tweakdbPath)) {
        return tweakdbPath.string();
    }

    spdlog::error("Could not find tweakdb.bin in: {}", gameDir);
    spdlog::info("Expected location: {}", tweakdbPath.string());
    return std::nullopt;
}

// Backup original tweakdb.bin
bool BackupTweakDB(const String& tweakdbPath) {
    String backupPath = tweakdbPath + ".backup";

    // Don't overwrite existing backup
    if (fs::exists(backupPath)) {
        spdlog::debug("Backup already exists: {}", backupPath);
        return true;
    }

    try {
        fs::copy_file(tweakdbPath, backupPath);
        spdlog::info("Created backup: {}", backupPath);
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Failed to create backup: {}", e.what());
        return false;
    }
}

int main(int argc, char** argv) {
    CLI::App app{"TweakXL-macOS - Offline TweakDB patcher for macOS Cyberpunk 2077"};

    // Version
    app.set_version_flag("--version", "1.0.0");

    // Options
    std::string gameDir;
    std::string modsDir;
    std::string tweakdbPath;
    std::string outputPath;
    bool dryRun = false;
    bool verbose = false;
    bool noBackup = false;

    app.add_option("--game-dir", gameDir, "Path to Cyberpunk 2077.app");
    app.add_option("--tweakdb", tweakdbPath, "Direct path to tweakdb.bin (overrides --game-dir)");
    app.add_option("--mods-dir", modsDir, "Path to mods directory")
        ->required();
    app.add_option("--output,-o", outputPath, "Output path for modified tweakdb.bin");
    app.add_flag("--dry-run", dryRun, "Preview changes without writing output");
    app.add_flag("--no-backup", noBackup, "Skip creating backup of original tweakdb.bin");
    app.add_flag("-v,--verbose", verbose, "Verbose output");

    CLI11_PARSE(app, argc, argv);

    // Set up logging
    if (verbose) {
        spdlog::set_level(spdlog::level::debug);
    } else {
        spdlog::set_level(spdlog::level::info);
    }

    spdlog::info("=== TweakXL-macOS v1.0.0 ===");
    spdlog::info("Offline TweakDB patcher for macOS");
    spdlog::info("");

    // Determine tweakdb.bin path
    if (tweakdbPath.empty()) {
        if (gameDir.empty()) {
            spdlog::error("Either --game-dir or --tweakdb must be specified");
            return 1;
        }

        auto found = FindTweakDB(gameDir);
        if (!found) {
            return 1;
        }
        tweakdbPath = *found;
    }

    spdlog::info("TweakDB file: {}", tweakdbPath);
    spdlog::info("Mods directory: {}", modsDir);

    // Verify files exist
    if (!fs::exists(tweakdbPath)) {
        spdlog::error("TweakDB file not found: {}", tweakdbPath);
        return 1;
    }

    if (!fs::exists(modsDir)) {
        spdlog::error("Mods directory not found: {}", modsDir);
        return 1;
    }

    // Set output path
    if (outputPath.empty()) {
        if (dryRun) {
            outputPath = "tweakdb.bin.dry-run";
        } else {
            outputPath = tweakdbPath; // Overwrite original
        }
    }

    spdlog::info("Output file: {}", outputPath);
    spdlog::info("");

    // Step 1: Read original TweakDB
    spdlog::info("[1/5] Reading original TweakDB...");
    TweakDBReader reader(tweakdbPath);

    if (!reader.Load()) {
        spdlog::error("Failed to load TweakDB");
        return 1;
    }

    spdlog::info("  Loaded {} flats, {} records",
                 reader.GetFlatCount(), reader.GetRecordCount());

    // Step 2: Create in-memory database from loaded data
    spdlog::info("[2/5] Creating in-memory database...");
    TweakDB db;

    // Copy flats from reader to database
    for (const auto& [id, flat] : reader.GetFlats()) {
        db.SetFlat(flat);
    }

    // Copy records from reader to database
    for (const auto& [id, record] : reader.GetRecords()) {
        db.SetRecord(record);
    }

    spdlog::info("  Database initialized with {} entries",
                 db.GetFlatCount() + db.GetRecordCount());

    // Step 3: Load mods
    spdlog::info("[3/5] Loading mods from: {}", modsDir);
    ModLoader modLoader;

    TweakSource mergedMods = modLoader.LoadFromDirectory(modsDir, true);

    if (modLoader.HasErrors()) {
        const auto& errors = modLoader.GetErrors();
        spdlog::warn("  Encountered {} errors while loading mods:", errors.size());
        for (const auto& error : errors) {
            spdlog::warn("    {}", error);
        }
    }

    spdlog::info("  Loaded {} file(s)", modLoader.GetLoadedFileCount());

    if (mergedMods.GetTotalModificationCount() == 0) {
        spdlog::warn("No modifications found in mods");
        spdlog::info("Nothing to do!");
        return 0;
    }

    spdlog::info("  Total modifications: {} flats, {} records",
                 mergedMods.GetFlatCount(), mergedMods.GetRecordCount());

    // Step 4: Apply modifications
    spdlog::info("[4/5] Applying modifications...");
    ModificationApplicator applicator;

    if (!applicator.Apply(db, mergedMods)) {
        spdlog::error("Failed to apply modifications");
        if (applicator.HasErrors()) {
            const auto& appErrors = applicator.GetErrors();
            spdlog::error("  Errors:");
            for (const auto& error : appErrors) {
                spdlog::error("    {}", error);
            }
        }
        return 1;
    }

    spdlog::info("  Applied {} flats, {} records",
                 applicator.GetAppliedFlatCount(),
                 applicator.GetAppliedRecordCount());

    if (applicator.HasErrors()) {
        const auto& appErrors = applicator.GetErrors();
        spdlog::warn("  Warnings ({}):", appErrors.size());
        for (const auto& error : appErrors) {
            spdlog::warn("    {}", error);
        }
    }

    // Step 5: Write modified TweakDB
    if (dryRun) {
        spdlog::info("[5/5] Dry run - skipping write");
        spdlog::info("");
        spdlog::info("=== Dry run completed successfully ===");
        spdlog::info("No files were modified");
        return 0;
    }

    // Create backup if overwriting original
    if (outputPath == tweakdbPath && !noBackup) {
        spdlog::info("[5/5] Creating backup and writing modified TweakDB...");
        if (!BackupTweakDB(tweakdbPath)) {
            spdlog::error("Failed to create backup, aborting");
            return 1;
        }
    } else {
        spdlog::info("[5/5] Writing modified TweakDB...");
    }

    TweakDBWriter writer(outputPath);
    if (!writer.Write(db)) {
        spdlog::error("Failed to write TweakDB");
        return 1;
    }

    spdlog::info("  Written to: {}", outputPath);
    spdlog::info("");
    spdlog::info("=== Patching completed successfully ===");
    spdlog::info("Modified TweakDB: {}", outputPath);

    if (!noBackup && outputPath == tweakdbPath) {
        spdlog::info("Original backed up: {}.backup", tweakdbPath);
    }

    return 0;
}
