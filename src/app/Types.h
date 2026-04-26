#pragma once

#include <glm/glm.hpp>
#include <string>

struct PickResult {
    bool      valid    = false;
    glm::vec3 worldPos{0.0f};
    int       objectId = -1;
    float     depth    = 1.0f;
};

// ─── Annotation ───────────────────────────────────────────────────────────────
// Lightweight annotation data model (Milestone 4).
// Each annotation records the label, world-space position, and the object ID
// from the pick result that originated it.
// ─────────────────────────────────────────────────────────────────────────────
struct Annotation {
    int         id       = 0;
    std::string label;
    glm::vec3   worldPos{0.0f};
    int         objectId = -1;
};

// ─── InteractionMode ──────────────────────────────────────────────────────────
// Controls what a right-click pick does.
//   Annotate : pick + label → create an Annotation marker (original behaviour)
//   Measure  : pick two points → compute & display distance
//              pick three points → compute & display angle at the middle point
// ─────────────────────────────────────────────────────────────────────────────
enum class InteractionMode { Annotate, Measure };
