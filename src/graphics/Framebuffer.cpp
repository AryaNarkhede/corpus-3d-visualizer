#include "Framebuffer.h"

#include <cstdio>

// ─── Framebuffer::create ──────────────────────────────────────────────────────

bool Framebuffer::create(int width, int height)
{
    destroy();

    m_width  = width;
    m_height = height;

    // ── Colour renderbuffer (RGBA8) ───────────────────────────────────────────
    glGenRenderbuffers(1, &m_colorRbo);
    glBindRenderbuffer(GL_RENDERBUFFER, m_colorRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, width, height);

    // ── Depth renderbuffer (DEPTH24) ──────────────────────────────────────────
    glGenRenderbuffers(1, &m_depthRbo);
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);

    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // ── Assemble FBO ─────────────────────────────────────────────────────────
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                              GL_RENDERBUFFER, m_colorRbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                              GL_RENDERBUFFER, m_depthRbo);

    if (!isComplete()) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        destroy();
        return false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    std::printf("[Framebuffer] Created %dx%d picking FBO\n", width, height);
    return true;
}

// ─── Framebuffer::resize ──────────────────────────────────────────────────────

void Framebuffer::resize(int width, int height)
{
    if (m_width == width && m_height == height) return;
    if (m_fbo == 0) { create(width, height); return; }

    m_width  = width;
    m_height = height;

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // Recreate colour renderbuffer
    glDeleteRenderbuffers(1, &m_colorRbo);
    glGenRenderbuffers(1, &m_colorRbo);
    glBindRenderbuffer(GL_RENDERBUFFER, m_colorRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                              GL_RENDERBUFFER, m_colorRbo);

    // Recreate depth renderbuffer
    glDeleteRenderbuffers(1, &m_depthRbo);
    glGenRenderbuffers(1, &m_depthRbo);
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                              GL_RENDERBUFFER, m_depthRbo);

    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    isComplete(); // logs status if incomplete

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    std::printf("[Framebuffer] Resized picking FBO to %dx%d\n", width, height);
}

// ─── Framebuffer::destroy ─────────────────────────────────────────────────────

void Framebuffer::destroy()
{
    if (m_fbo)      { glDeleteFramebuffers(1,  &m_fbo);      m_fbo      = 0; }
    if (m_colorRbo) { glDeleteRenderbuffers(1, &m_colorRbo); m_colorRbo = 0; }
    if (m_depthRbo) { glDeleteRenderbuffers(1, &m_depthRbo); m_depthRbo = 0; }
    m_width = m_height = 0;
}

// ─── Framebuffer::bind / unbind ───────────────────────────────────────────────

void Framebuffer::bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
}

void Framebuffer::unbind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// ─── Framebuffer::isComplete ──────────────────────────────────────────────────

bool Framebuffer::isComplete() const
{
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status == GL_FRAMEBUFFER_COMPLETE) return true;

    const char* msg = "unknown";
    switch (status) {
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:         msg = "incomplete attachment";         break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: msg = "missing attachment";             break;
        case GL_FRAMEBUFFER_UNSUPPORTED:                   msg = "unsupported format combination"; break;
        default: break;
    }
    std::fprintf(stderr, "[Framebuffer] Incomplete FBO (0x%04X): %s\n",
                 static_cast<unsigned>(status), msg);
    return false;
}
