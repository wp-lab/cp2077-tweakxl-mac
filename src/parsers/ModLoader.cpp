#include "ModLoader.hpp"
#include <spdlog/spdlog.h>
#include <algorithm>

namespace TweakXL {

TweakSource ModLoader::LoadFromDirectory(const std::filesystem::path& modsDir, bool recursive) {
    ClearState();
    recursive_ = recursive;

    TweakSource mergedSource(modsDir.string());

    // Check if directory exists
    if (!std::filesystem::exists(modsDir)) {
        AddError("Mods directory does not exist: " + modsDir.string());
        spdlog::error("Mods directory not found: {}", modsDir.string());
        return mergedSource;
    }

    if (!std::filesystem::is_directory(modsDir)) {
        AddError("Path is not a directory: " + modsDir.string());
        spdlog::error("Not a directory: {}", modsDir.string());
        return mergedSource;
    }

    // Scan for mod files
    auto modFiles = ScanDirectory(modsDir, recursive);

    if (modFiles.empty()) {
        spdlog::warn("No mod files found in: {}", modsDir.string());
        return mergedSource;
    }

    spdlog::info("Found {} mod file(s) in: {}", modFiles.size(), modsDir.string());

    // Load each file
    size_t successCount = 0;
    for (size_t i = 0; i < modFiles.size(); ++i) {
        const auto& file = modFiles[i];

        ReportProgress(file, i + 1, modFiles.size());

        try {
            TweakSource fileSource = LoadFile(file);

            if (fileSource.GetTotalModificationCount() > 0) {
                mergedSource.Merge(fileSource);
                successCount++;
                loadedFiles_.push_back(file);
            }

        } catch (const std::exception& e) {
            AddError("Failed to load " + file.filename().string() + ": " + e.what());
            spdlog::error("Error loading {}: {}", file.string(), e.what());
        }
    }

    spdlog::info("Successfully loaded {}/{} mod files ({} flats, {} records)",
                 successCount, modFiles.size(),
                 mergedSource.GetFlatCount(),
                 mergedSource.GetRecordCount());

    return mergedSource;
}

TweakSource ModLoader::LoadFile(const std::filesystem::path& filepath) {
    if (!std::filesystem::exists(filepath)) {
        throw std::runtime_error("File does not exist: " + filepath.string());
    }

    // Determine file type and parse
    String extension = filepath.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    if (extension == ".yaml" || extension == ".yml") {
        spdlog::debug("Parsing YAML file: {}", filepath.filename().string());
        return yamlParser_.Parse(filepath);
    }
    // TODO: Add .tweak parser when implemented
    else {
        throw std::runtime_error("Unsupported file type: " + extension);
    }
}

TweakSource ModLoader::LoadFromDirectories(const std::vector<std::filesystem::path>& dirs, bool recursive) {
    ClearState();
    recursive_ = recursive;

    TweakSource mergedSource("<multiple>");

    for (const auto& dir : dirs) {
        try {
            TweakSource dirSource = LoadFromDirectory(dir, recursive);
            mergedSource.Merge(dirSource);

        } catch (const std::exception& e) {
            AddError("Failed to load from " + dir.string() + ": " + e.what());
            spdlog::error("Error loading from {}: {}", dir.string(), e.what());
        }
    }

    spdlog::info("Loaded mods from {} directories ({} total files, {} flats, {} records)",
                 dirs.size(), loadedFiles_.size(),
                 mergedSource.GetFlatCount(), mergedSource.GetRecordCount());

    return mergedSource;
}

std::vector<std::filesystem::path> ModLoader::ScanDirectory(const std::filesystem::path& dir, bool recursive) {
    std::vector<std::filesystem::path> modFiles;

    try {
        if (recursive) {
            // Recursive scan
            for (const auto& entry : std::filesystem::recursive_directory_iterator(dir)) {
                if (entry.is_regular_file() && IsModFile(entry.path()) && !ShouldIgnore(entry.path())) {
                    modFiles.push_back(entry.path());
                }
            }
        } else {
            // Non-recursive scan
            for (const auto& entry : std::filesystem::directory_iterator(dir)) {
                if (entry.is_regular_file() && IsModFile(entry.path()) && !ShouldIgnore(entry.path())) {
                    modFiles.push_back(entry.path());
                }
            }
        }

        // Sort files alphabetically for consistent load order
        std::sort(modFiles.begin(), modFiles.end());

    } catch (const std::filesystem::filesystem_error& e) {
        AddError("Error scanning directory: " + String(e.what()));
        spdlog::error("Filesystem error: {}", e.what());
    }

    return modFiles;
}

bool ModLoader::IsModFile(const std::filesystem::path& filepath) const {
    String extension = filepath.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    return extension == ".yaml" || extension == ".yml" || extension == ".tweak";
}

bool ModLoader::ShouldIgnore(const std::filesystem::path& filepath) const {
    String filename = filepath.filename().string();

    // Ignore hidden files
    if (!filename.empty() && filename[0] == '.') {
        return true;
    }

    // Ignore backup files
    if ((!filename.empty() && filename.back() == '~') ||
        (filename.size() >= 4 && filename.substr(filename.size() - 4) == ".bak")) {
        return true;
    }

    // Check custom ignore patterns
    for (const auto& pattern : ignorePatterns_) {
        if (filename.find(pattern) != String::npos) {
            return true;
        }
    }

    return false;
}

void ModLoader::AddError(const String& message) {
    errors_.push_back(message);
}

void ModLoader::ReportProgress(const std::filesystem::path& file, size_t current, size_t total) {
    if (progressCallback_) {
        progressCallback_(file, current, total);
    }
}

void ModLoader::ClearState() {
    loadedFiles_.clear();
    errors_.clear();
}

} // namespace TweakXL
