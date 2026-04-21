#pragma once

#include "Mesh.h"

#include <glm/glm.hpp>

#include <string>
#include <vector>

// ─── Model ────────────────────────────────────────────────────────────────────
// Loads an .obj file via tinyobjloader and stores a vector of GPU Meshes.
// Missing normals are computed automatically (flat per-triangle normals).
// ─────────────────────────────────────────────────────────────────────────────
class Model {
public:
    // Returns true on success; prints an error message on failure.
    bool load(const std::string& path);

    // Issue draw calls for all meshes.
    void draw() const;

    bool isLoaded() const { return m_loaded; }

    // Bounding sphere data, computed after load.
    glm::vec3 center()  const { return m_center; }
    float     radius()  const { return m_radius; }

private:
    std::vector<Mesh> m_meshes;
    glm::vec3         m_center{0.0f};
    float             m_radius  = 1.0f;
    bool              m_loaded  = false;

    void computeBounds(const std::vector<Vertex>& verts);
};
