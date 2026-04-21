#pragma once

#include "Shader.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <vector>

// ─── MarkerRenderer ───────────────────────────────────────────────────────────
// Renders annotation world-space positions as round point sprites in the 3-D
// scene.  Designed to be called once per frame after the main scene pass.
//
// Usage:
//   if (m_markerReady)
//       m_markerRenderer.draw(positions, viewProj);
// ─────────────────────────────────────────────────────────────────────────────
class MarkerRenderer {
public:
    MarkerRenderer()  = default;
    ~MarkerRenderer() { shutdown(); }

    // Not copyable
    MarkerRenderer(const MarkerRenderer&)            = delete;
    MarkerRenderer& operator=(const MarkerRenderer&) = delete;

    // Load shaders and allocate GL objects. Returns false on failure.
    bool init(const std::string& shaderDir);

    // Upload positions and draw them as point sprites.
    // viewProj = projection * view  (no model matrix – positions are world-space).
    void draw(const std::vector<glm::vec3>& positions,
              const glm::mat4&              viewProj) const;

    void shutdown();

private:
    Shader  m_shader;
    GLuint  m_vao = 0;
    GLuint  m_vbo = 0;
};
