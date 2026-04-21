#include "AnnotationStore.h"

#include <algorithm>

// ─── AnnotationStore::add ─────────────────────────────────────────────────────

int AnnotationStore::add(const std::string& label,
                          const glm::vec3&   worldPos,
                          int                objectId)
{
    Annotation a;
    a.id       = m_nextId++;
    a.label    = label;
    a.worldPos = worldPos;
    a.objectId = objectId;
    m_annotations.push_back(std::move(a));
    return m_annotations.back().id;
}

// ─── AnnotationStore::remove ──────────────────────────────────────────────────

bool AnnotationStore::remove(int id)
{
    auto it = std::find_if(m_annotations.begin(), m_annotations.end(),
                           [id](const Annotation& a) { return a.id == id; });
    if (it == m_annotations.end()) return false;
    m_annotations.erase(it);
    return true;
}

// ─── AnnotationStore::clear ───────────────────────────────────────────────────

void AnnotationStore::clear()
{
    m_annotations.clear();
}
