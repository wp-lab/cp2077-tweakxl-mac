#pragma once

#include "../model/TweakSource.hpp"
#include "YAMLParser.hpp"
#include <filesystem>
#include <vector>
#include <functional>

namespace TweakXL {

/**
 * ModLoader - Scans directories and loads mod files
 *
 * Supports:
 * - .yaml and .yml files
 * - Recursive directory scanning
 * - Load order management
 * - Conflict detection
 * - Statistics and reporting
 */
class ModLoader {
public:
    ModLoader() = default;

    // Load all mods from a directory
    TweakSource LoadFromDirectory(const std::filesystem::path& modsDir, bool recursive = true);

    // Load specific mod file
    TweakSource LoadFile(const std::filesystem::path& filepath);

    // Load multiple directories
    TweakSource LoadFromDirectories(const std::vector<std::filesystem::path>& dirs, bool recursive = true);

    // Statistics
    size_t GetLoadedFileCount() const { return loadedFiles_.size(); }
    const std::vector<std::filesystem::path>& GetLoadedFiles() const { return loadedFiles_; }
    const std::vector<String>& GetErrors() const { return errors_; }
    bool HasErrors() const { return !errors_.empty(); }

    // Configuration
    void SetRecursive(bool recursive) { recursive_ = recursive; }
    void AddIgnorePattern(const String& pattern) { ignorePatterns_.push_back(pattern); }
    void ClearIgnorePatterns() { ignorePatterns_.clear(); }

    // Progress callback (for UI feedback)
    using ProgressCallback = std::function<void(const std::filesystem::path&, size_t current, size_t total)>;
    void SetProgressCallback(ProgressCallback callback) { progressCallback_ = callback; }

private:
    bool recursive_ = true;
    std::vector<String> ignorePatterns_;
    std::vector<std::filesystem::path> loadedFiles_;
    std::vector<String> errors_;
    ProgressCallback progressCallback_;

    // File scanning
    std::vector<std::filesystem::path> ScanDirectory(const std::filesystem::path& dir, bool recursive);
    bool IsModFile(const std::filesystem::path& filepath) const;
    bool ShouldIgnore(const std::filesystem::path& filepath) const;

    // Parsers
    YAMLTweakParser yamlParser_;

    // Helpers
    void AddError(const String& message);
    void ReportProgress(const std::filesystem::path& file, size_t current, size_t total);
    void ClearState();
};

} // namespace TweakXL
