#pragma once

#include "Types.h"

#include <string>
#include <vector>

// ─── AnnotationStore ──────────────────────────────────────────────────────────
// In-memory store for Annotation objects.
// Provides add / remove / list with a simple auto-incrementing ID.
// Milestone 5 adds replaceAll() for JSON persistence (load).
// ─────────────────────────────────────────────────────────────────────────────
class AnnotationStore {
public:
    // Add a new annotation. Returns the assigned id.
    int  add(const std::string& label, const glm::vec3& worldPos, int objectId = -1);

    // Remove the annotation with the given id.
    // Returns true if found and removed, false otherwise.
    bool remove(int id);

    // Read-only access to the full list.
    const std::vector<Annotation>& all() const { return m_annotations; }

    // Replace all annotations with the provided list (Milestone 5 – load).
    // Resets m_nextId to one above the highest id in the list (or 1 if empty).
    void replaceAll(std::vector<Annotation> loaded);

    // Remove all annotations (e.g. when a new model is loaded).
    void clear();

private:
    std::vector<Annotation> m_annotations;
    int                     m_nextId = 1;
};
