#include "io/Screenshot.h"

#include <glad/glad.h>

#include <cstdio>
#include <ctime>
#include <string>
#include <sys/stat.h>
#if defined(_WIN32)
#  include <direct.h>
#endif

// ── stb_image_write (single-header, implementation unit) ─────────────────────
// Suppress warnings from the third-party header.
#if defined(__GNUC__) || defined(__clang__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"
#if defined(__GNUC__) || defined(__clang__)
#  pragma GCC diagnostic pop
#endif

#include <algorithm>
#include <vector>

// ─────────────────────────────────────────────────────────────────────────────
// Internal helpers
// ─────────────────────────────────────────────────────────────────────────────

namespace {

bool pathExists(const std::string& path)
{
    struct stat st{};
    return (::stat(path.c_str(), &st) == 0);
}

bool makeDir(const std::string& dir)
{
    if (dir.empty() || pathExists(dir)) return true;
#if defined(_WIN32)
    return (_mkdir(dir.c_str()) == 0);
#else
    return (::mkdir(dir.c_str(), 0755) == 0);
#endif
}

std::string parentDir(const std::string& path)
{
    auto pos = path.find_last_of("/\\");
    if (pos == std::string::npos) return {};
    return path.substr(0, pos);
}

} // anonymous namespace

// ─────────────────────────────────────────────────────────────────────────────
// Screenshot::capture
// ─────────────────────────────────────────────────────────────────────────────

Screenshot::Result Screenshot::capture(int width, int height,
                                        const std::string& filepath)
{
    Result result;

    if (width <= 0 || height <= 0) {
        result.message = "Screenshot failed: invalid dimensions "
                         + std::to_string(width) + "x" + std::to_string(height);
        return result;
    }

    // Ensure parent directory exists.
    std::string dir = parentDir(filepath);
    if (!dir.empty() && !makeDir(dir)) {
        result.message = "Screenshot failed: cannot create directory '" + dir + "'";
        std::fprintf(stderr, "[Screenshot] %s\n", result.message.c_str());
        return result;
    }

    // Read pixels from the default framebuffer.
    // GL_READ_FRAMEBUFFER should still be 0 (default) after the scene pass.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    const int channels = 3;
    std::vector<unsigned char> pixels(
        static_cast<size_t>(width) * static_cast<size_t>(height) * channels);

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    // OpenGL origin is bottom-left; PNG origin is top-left → flip vertically.
    const int rowBytes = width * channels;
    std::vector<unsigned char> flipped(pixels.size());
    for (int row = 0; row < height; ++row) {
        const unsigned char* src = pixels.data() + (height - 1 - row) * rowBytes;
        unsigned char*       dst = flipped.data() + row * rowBytes;
        std::copy(src, src + rowBytes, dst);
    }

    // Write PNG.
    if (!stbi_write_png(filepath.c_str(), width, height, channels,
                        flipped.data(), rowBytes)) {
        result.message = "Screenshot failed: stbi_write_png could not write '"
                         + filepath + "'";
        std::fprintf(stderr, "[Screenshot] %s\n", result.message.c_str());
        return result;
    }

    result.success = true;
    result.message = "Screenshot saved: '" + filepath + "'";
    std::printf("[Screenshot] %s\n", result.message.c_str());
    return result;
}

// ─────────────────────────────────────────────────────────────────────────────
// Screenshot::generateFilename
// ─────────────────────────────────────────────────────────────────────────────

std::string Screenshot::generateFilename(const std::string& dir)
{
    std::time_t t = std::time(nullptr);
    std::tm* tm_info = std::localtime(&t);

    char buf[64] = {};
    std::strftime(buf, sizeof(buf), "screenshot_%Y%m%d_%H%M%S.png", tm_info);

    return dir + "/" + buf;
}
