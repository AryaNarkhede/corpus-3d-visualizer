#pragma once

#include "Shader.h"
#include "Lighting.h"

// Forward declarations
class Model;
class Camera;

// ─── Renderer ─────────────────────────────────────────────────────────────────
// Manages the Blinn-Phong forward render pass.
// Owns the mesh shader.  The caller provides Model + Camera each frame.
// ─────────────────────────────────────────────────────────────────────────────
class Renderer {
public:
    // Load and compile shaders from the given directory.
    // Returns false if shaders cannot be loaded.
    bool init(const std::string& shaderDir);

    // Draw one full frame: clear, enable depth test, draw model.
    void draw(const Model& model, const Camera& camera,
              int viewportW, int viewportH) const;

    void shutdown();

    // ── Tweakable light / material (exposed for ImGui) ────────────────────────
    DirectionalLight& light()    { return m_light;    }
    Material&         material() { return m_material; }

private:
    Shader           m_shader;
    DirectionalLight m_light;
    Material         m_material;
};
