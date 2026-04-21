#include "io/JsonPersistence.h"

#include <nlohmann/json.hpp>

#include <cstdio>
#include <fstream>
#include <stdexcept>
#include <string>
#include <sys/stat.h>
#if defined(_WIN32)
#  include <direct.h>
#endif

// ─────────────────────────────────────────────────────────────────────────────
// Internal helpers
// ─────────────────────────────────────────────────────────────────────────────

namespace {

// Returns true if path exists (file or directory).
bool pathExists(const std::string& path)
{
    struct stat st{};
    return (::stat(path.c_str(), &st) == 0);
}

// Returns parent directory component of a path.
// "data/annotations.json" → "data"
// "annotations.json"      → ""
// "/tmp/foo/bar.json"     → "/tmp/foo"
std::string parentDir(const std::string& path)
{
    auto pos = path.find_last_of("/\\");
    if (pos == std::string::npos) return {};
    return path.substr(0, pos);
}

// Create a single directory (not recursive). Returns true on success or if
// the directory already exists.
bool makeDir(const std::string& dir)
{
    if (dir.empty() || pathExists(dir)) return true;
#if defined(_WIN32)
    return (_mkdir(dir.c_str()) == 0);
#else
    return (::mkdir(dir.c_str(), 0755) == 0);
#endif
}

} // anonymous namespace

// ─────────────────────────────────────────────────────────────────────────────
// JsonPersistence::saveAnnotations
// ─────────────────────────────────────────────────────────────────────────────

JsonPersistence::SaveResult
JsonPersistence::saveAnnotations(const std::vector<Annotation>& annotations,
                                 const std::string&             filepath)
{
    SaveResult result;

    // Ensure parent directory exists.
    std::string dir = parentDir(filepath);
    if (!dir.empty() && !makeDir(dir)) {
        result.message = "Save failed: cannot create directory '" + dir + "'";
        std::fprintf(stderr, "[JsonPersistence] %s\n", result.message.c_str());
        return result;
    }

    // Build JSON document.
    nlohmann::json doc;
    doc["schema_version"] = kSchemaVersion;
    doc["annotations"]    = nlohmann::json::array();

    for (const auto& a : annotations) {
        nlohmann::json entry;
        entry["id"]        = a.id;
        entry["label"]     = a.label;
        entry["position"]  = { {"x", a.worldPos.x},
                               {"y", a.worldPos.y},
                               {"z", a.worldPos.z} };
        entry["object_id"] = a.objectId;
        doc["annotations"].push_back(std::move(entry));
    }

    // Write to file.
    std::ofstream ofs(filepath);
    if (!ofs.is_open()) {
        result.message = "Save failed: cannot open '" + filepath + "' for writing";
        std::fprintf(stderr, "[JsonPersistence] %s\n", result.message.c_str());
        return result;
    }

    try {
        ofs << doc.dump(2) << '\n';
    } catch (const std::exception& e) {
        result.message = std::string("Save failed during serialisation: ") + e.what();
        std::fprintf(stderr, "[JsonPersistence] %s\n", result.message.c_str());
        return result;
    }

    result.success = true;
    result.message = "Saved " + std::to_string(static_cast<int>(annotations.size()))
                     + " annotation(s) to '" + filepath + "'";
    std::printf("[JsonPersistence] %s\n", result.message.c_str());
    return result;
}

// ─────────────────────────────────────────────────────────────────────────────
// JsonPersistence::loadAnnotations
// ─────────────────────────────────────────────────────────────────────────────

JsonPersistence::LoadResult
JsonPersistence::loadAnnotations(const std::string&       filepath,
                                 std::vector<Annotation>& outAnnotations)
{
    LoadResult result;

    // Open file.
    std::ifstream ifs(filepath);
    if (!ifs.is_open()) {
        result.message = "Load failed: file not found '" + filepath + "'";
        std::fprintf(stderr, "[JsonPersistence] %s\n", result.message.c_str());
        return result;
    }

    // Parse JSON.
    nlohmann::json doc;
    try {
        ifs >> doc;
    } catch (const nlohmann::json::parse_error& e) {
        result.message = std::string("Load failed: JSON parse error — ") + e.what();
        std::fprintf(stderr, "[JsonPersistence] %s\n", result.message.c_str());
        return result;
    }

    // Validate schema version.
    if (!doc.contains("schema_version") || !doc["schema_version"].is_number_integer()) {
        result.message = "Load failed: missing or invalid 'schema_version' field";
        std::fprintf(stderr, "[JsonPersistence] %s\n", result.message.c_str());
        return result;
    }
    int version = doc["schema_version"].get<int>();
    if (version != kSchemaVersion) {
        result.message = "Load failed: unsupported schema_version "
                         + std::to_string(version)
                         + " (expected " + std::to_string(kSchemaVersion) + ")";
        std::fprintf(stderr, "[JsonPersistence] %s\n", result.message.c_str());
        return result;
    }

    // Validate annotations array.
    if (!doc.contains("annotations") || !doc["annotations"].is_array()) {
        result.message = "Load failed: 'annotations' key is missing or not an array";
        std::fprintf(stderr, "[JsonPersistence] %s\n", result.message.c_str());
        return result;
    }

    // Parse each entry.
    for (const auto& entry : doc["annotations"]) {
        // Required: id (integer)
        if (!entry.contains("id") || !entry["id"].is_number_integer()) {
            std::fprintf(stderr,
                         "[JsonPersistence] Skipping entry: missing/invalid 'id'\n");
            ++result.skippedCount;
            continue;
        }

        // Required: label (string)
        if (!entry.contains("label") || !entry["label"].is_string()) {
            std::fprintf(stderr,
                         "[JsonPersistence] Skipping entry id=%d: missing/invalid 'label'\n",
                         entry["id"].get<int>());
            ++result.skippedCount;
            continue;
        }

        // Required: position (object with numeric x, y, z)
        if (!entry.contains("position") || !entry["position"].is_object()) {
            std::fprintf(stderr,
                         "[JsonPersistence] Skipping entry id=%d: missing/invalid 'position'\n",
                         entry["id"].get<int>());
            ++result.skippedCount;
            continue;
        }
        const auto& pos = entry["position"];
        if (!pos.contains("x") || !pos["x"].is_number() ||
            !pos.contains("y") || !pos["y"].is_number() ||
            !pos.contains("z") || !pos["z"].is_number()) {
            std::fprintf(stderr,
                         "[JsonPersistence] Skipping entry id=%d: 'position' missing x/y/z\n",
                         entry["id"].get<int>());
            ++result.skippedCount;
            continue;
        }

        Annotation a;
        a.id       = entry["id"].get<int>();
        a.label    = entry["label"].get<std::string>();
        a.worldPos = { pos["x"].get<float>(),
                       pos["y"].get<float>(),
                       pos["z"].get<float>() };

        // Optional: object_id
        if (entry.contains("object_id") && entry["object_id"].is_number_integer()) {
            a.objectId = entry["object_id"].get<int>();
        } else {
            a.objectId = -1;
        }

        outAnnotations.push_back(std::move(a));
        ++result.loadedCount;
    }

    result.success = true;
    result.message = "Loaded " + std::to_string(result.loadedCount)
                     + " annotation(s)";
    if (result.skippedCount > 0)
        result.message += " (" + std::to_string(result.skippedCount) + " skipped)";
    result.message += " from '" + filepath + "'";
    std::printf("[JsonPersistence] %s\n", result.message.c_str());
    return result;
}
