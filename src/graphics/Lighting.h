#pragma once

#include <glm/glm.hpp>

// ─── Lighting data structures for Blinn-Phong shading ─────────────────────────

struct DirectionalLight {
    glm::vec3 direction{0.4f, 0.8f, 0.6f}; // points FROM scene TO light (world space)
    glm::vec3 color{1.0f, 1.0f, 1.0f};
    float     ambientStrength = 0.15f;
};

struct Material {
    glm::vec3 diffuse{0.7f, 0.75f, 0.8f};
    glm::vec3 specular{0.4f, 0.4f, 0.4f};
    float     shininess = 64.0f;
};
