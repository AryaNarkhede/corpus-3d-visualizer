#pragma once

#include "app/Types.h"

#include <string>
#include <vector>

// ─── JsonPersistence ──────────────────────────────────────────────────────────
// Milestone 5: Save and load annotations to/from a JSON file.
//
// JSON schema (version 1):
//   {
//     "schema_version": 1,
//     "annotations": [
//       {
//         "id":        <int>,
//         "label":     <string>,
//         "position":  { "x": <float>, "y": <float>, "z": <float> },
//         "object_id": <int>           // -1 when no object
//       }
//     ]
//   }
//
// Validation rules:
//   - "schema_version" must be present and equal to kSchemaVersion.
//   - "annotations" must be a JSON array.
//   - Each entry must have "id" (int), "label" (string), and "position" (object
//     with numeric "x", "y", "z"). Missing or wrong-type fields cause the entry
//     to be skipped; the rest are still loaded.
//   - "object_id" is optional; defaults to -1 if absent or wrong type.
//   - Unknown top-level keys are silently ignored (forward compatibility).
// ─────────────────────────────────────────────────────────────────────────────

namespace JsonPersistence {

// Schema version written/expected by this implementation.
inline constexpr int kSchemaVersion = 1;

// Result of a save operation.
struct SaveResult {
    bool        success = false;
    std::string message;          // human-readable summary / error
};

// Result of a load operation.
struct LoadResult {
    bool        success      = false;
    int         loadedCount  = 0;
    int         skippedCount = 0;
    std::string message;          // human-readable summary / error
};

// Serialise annotations to filepath.
// Creates parent directories (one level deep) if they do not exist.
// Returns SaveResult with success=false and a descriptive message on error.
SaveResult saveAnnotations(const std::vector<Annotation>& annotations,
                           const std::string&             filepath);

// Deserialise annotations from filepath into outAnnotations.
// Entries that fail validation are skipped; the rest are appended.
// On hard failure (file not found, parse error, wrong version) success=false.
LoadResult loadAnnotations(const std::string&       filepath,
                           std::vector<Annotation>& outAnnotations);

} // namespace JsonPersistence
