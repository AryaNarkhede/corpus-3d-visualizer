#pragma once

#include <glm/glm.hpp>

// ─── Camera ───────────────────────────────────────────────────────────────────
// Arcball (orbit) camera.
//   • Left-mouse drag  – rotate around target
//   • Scroll wheel     – zoom (adjust distance)
// Uses spherical coordinates (yaw / pitch / distance) around a target point.
// ─────────────────────────────────────────────────────────────────────────────
class Camera {
public:
    explicit Camera(float distance = 3.0f,
                    float yawDeg   = 0.0f,
                    float pitchDeg = 20.0f);

    // ── Matrices ──────────────────────────────────────────────────────────────
    glm::mat4 viewMatrix()                  const;
    glm::mat4 projectionMatrix(float aspect) const;
    glm::vec3 position()                    const;

    // ── Input ─────────────────────────────────────────────────────────────────
    // Call with cursor delta in pixels.
    void processDrag(float dx, float dy);
    // Call with scroll wheel offset (positive = zoom in).
    void processScroll(float offset);

    // ── Target / distance ─────────────────────────────────────────────────────
    void  setTarget(const glm::vec3& target)   { m_target   = target;   }
    void  setDistance(float d)                  { m_distance = d;        }
    glm::vec3 target()   const { return m_target;   }
    float     distance() const { return m_distance; }
    float     fov()      const { return m_fov;       }

private:
    glm::vec3 m_target{0.0f};
    float m_distance          = 3.0f;
    float m_yaw               = 0.0f;   // degrees, horizontal rotation
    float m_pitch             = 20.0f;  // degrees, vertical rotation
    float m_fov               = 45.0f;  // vertical field of view

    float m_near              = 0.01f;
    float m_far               = 1000.0f;
    float m_mouseSensitivity  = 0.4f;
    float m_zoomSensitivity   = 0.15f;
    float m_minDistance       = 0.1f;
    float m_maxDistance       = 500.0f;
};
