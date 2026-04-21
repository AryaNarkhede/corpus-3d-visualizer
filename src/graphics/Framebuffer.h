#pragma once

#include <glad/glad.h>

class Framebuffer {
public:
    Framebuffer()  = default;
    ~Framebuffer() { destroy(); }

    Framebuffer(const Framebuffer&)            = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;

    bool create(int width, int height);
    void resize(int width, int height);
    void bind()   const;
    void unbind() const;

    void readPixel(int x, int y, unsigned char rgba[4]) const;
    float readDepth(int x, int y) const;

    int width()  const { return m_width;  }
    int height() const { return m_height; }

private:
    GLuint m_fbo        = 0;
    GLuint m_colorTex   = 0;
    GLuint m_depthRbo   = 0;
    int    m_width      = 0;
    int    m_height     = 0;

    void destroy();
    void createAttachments();
};
