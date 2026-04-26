#pragma once

#include <string>

// ─── Screenshot ───────────────────────────────────────────────────────────────
// Captures the current OpenGL default framebuffer and writes it to a PNG file.
// Requires glad headers to be included before using (OpenGL context must exist).
// ─────────────────────────────────────────────────────────────────────────────

namespace Screenshot {

struct Result {
    bool        success = false;
    std::string message;   // human-readable summary / error
};

// Capture the current default framebuffer (width x height pixels) and write
// to 'filepath' as a PNG image.
// Creates the parent directory (one level) if it does not exist.
Result capture(int width, int height, const std::string& filepath);

// Generate a timestamped filename under 'dir', e.g.
//   "data/screenshots/screenshot_20250426_153212.png"
std::string generateFilename(const std::string& dir = "data/screenshots");

} // namespace Screenshot
