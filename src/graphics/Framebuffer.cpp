#include "Framebuffer.h"

#include <cstdio>

bool Framebuffer::create(int width, int height)
{
    m_width  = width;
    m_height = height;

    glGenFramebuffers(1, &m_fbo);
    createAttachments();

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::fprintf(stderr, "[Framebuffer] Incomplete – status 0x%x\n", status);
        destroy();
        return false;
    }
    return true;
}

void Framebuffer::resize(int width, int height)
{
    if (width == m_width && height == m_height) return;
    if (width <= 0 || height <= 0) return;

    m_width  = width;
    m_height = height;

    if (m_colorTex) { glDeleteTextures(1, &m_colorTex);      m_colorTex = 0; }
    if (m_depthRbo) { glDeleteRenderbuffers(1, &m_depthRbo);  m_depthRbo = 0; }

    createAttachments();
}

void Framebuffer::bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_width, m_height);
}

void Framebuffer::unbind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::readPixel(int x, int y, unsigned char rgba[4]) const
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo);
    glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

float Framebuffer::readDepth(int x, int y) const
{
    float d = 1.0f;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo);
    glReadPixels(x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &d);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    return d;
}

void Framebuffer::createAttachments()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    glGenTextures(1, &m_colorTex);
    glBindTexture(GL_TEXTURE_2D, m_colorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, m_colorTex, 0);

    glGenRenderbuffers(1, &m_depthRbo);
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24,
                          m_width, m_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                              GL_RENDERBUFFER, m_depthRbo);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::destroy()
{
    if (m_colorTex) { glDeleteTextures(1, &m_colorTex);      m_colorTex = 0; }
    if (m_depthRbo) { glDeleteRenderbuffers(1, &m_depthRbo);  m_depthRbo = 0; }
    if (m_fbo)      { glDeleteFramebuffers(1, &m_fbo);        m_fbo      = 0; }
    m_width = m_height = 0;
}
