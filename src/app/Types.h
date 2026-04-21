#pragma once

#include <glm/glm.hpp>

struct PickResult {
    bool      valid    = false;
    glm::vec3 worldPos{0.0f};
    int       objectId = -1;
    float     depth    = 1.0f;
};
