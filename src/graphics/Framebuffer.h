#pragma once

#include <glad/glad.h>

// ─── Framebuffer ──────────────────────────────────────────────────────────────
// Offscreen FBO with a GL_RGBA8 colour renderbuffer (for ID encoding) and a
// GL_DEPTH_COMPONENT24 renderbuffer (for depth readback).
// Used exclusively by the Picker for the FBO colour-picking pass.
// ─────────────────────────────────────────────────────────────────────────────
class Framebuffer {
public:
    Framebuffer()  = default;
    ~Framebuffer() { destroy(); }

    // Not copyable
    Framebuffer(const Framebuffer&)            = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;

    // Create the FBO with the given dimensions.
    // Returns false if creation or completeness check fails.
    bool create(int width, int height);

    // Resize renderbuffers to match a new window size.
    // No-op if dimensions are unchanged.
    void resize(int width, int height);

    // Release all GPU resources.
    void destroy();

    // Bind / unbind this FBO as the current draw+read framebuffer.
    void bind()   const;
    void unbind() const;

    // Check and log FBO completeness status; returns true if complete.
    bool isComplete() const;

    bool isValid() const { return m_fbo != 0; }
    int  width()   const { return m_width;    }
    int  height()  const { return m_height;   }

private:
    GLuint m_fbo      = 0;
    GLuint m_colorRbo = 0;  // GL_RGBA8 colour renderbuffer
    GLuint m_depthRbo = 0;  // GL_DEPTH_COMPONENT24 depth renderbuffer
    int    m_width    = 0;
    int    m_height   = 0;
};
