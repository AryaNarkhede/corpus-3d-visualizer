#pragma once

#include "Shader.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <vector>

// ─── LineRenderer ─────────────────────────────────────────────────────────────
// Renders line segments in the 3-D scene using GL_LINES.
// Designed for the measurement tool: draws coloured line segments between pairs
// of world-space points.
//
// Usage:
//   lineRenderer.begin();
//   lineRenderer.addSegment(a, b);      // adds one line segment
//   lineRenderer.draw(viewProj, color); // flush to GPU and render
// ─────────────────────────────────────────────────────────────────────────────
class LineRenderer {
public:
    LineRenderer()  = default;
    ~LineRenderer() { shutdown(); }

    LineRenderer(const LineRenderer&)            = delete;
    LineRenderer& operator=(const LineRenderer&) = delete;

    // Load shaders, allocate GL objects. Returns false on failure.
    bool init(const std::string& shaderDir);

    // Clear the pending segment list.
    void begin();

    // Queue a line segment between two world-space points.
    void addSegment(const glm::vec3& a, const glm::vec3& b);

    // Upload queued segments and render them.
    // viewProj = projection * view.  color is RGBA.
    void draw(const glm::mat4& viewProj, const glm::vec4& color) const;

    void shutdown();

    bool isValid() const { return m_shader.isValid(); }

private:
    Shader  m_shader;
    GLuint  m_vao = 0;
    GLuint  m_vbo = 0;

    std::vector<glm::vec3> m_vertices; // 2 vertices per segment
};
