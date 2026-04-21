#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cmath>

// ─────────────────────────────────────────────────────────────────────────────

Camera::Camera(float distance, float yawDeg, float pitchDeg)
    : m_distance(distance), m_yaw(yawDeg), m_pitch(pitchDeg)
{}

// ─── Camera::position ─────────────────────────────────────────────────────────
// Convert spherical (yaw, pitch, distance) to Cartesian offset from target.

glm::vec3 Camera::position() const
{
    const float yRad = glm::radians(m_yaw);
    const float pRad = glm::radians(m_pitch);

    glm::vec3 offset;
    offset.x = m_distance * std::cos(pRad) * std::sin(yRad);
    offset.y = m_distance * std::sin(pRad);
    offset.z = m_distance * std::cos(pRad) * std::cos(yRad);

    return m_target + offset;
}

// ─── Camera::viewMatrix ───────────────────────────────────────────────────────

glm::mat4 Camera::viewMatrix() const
{
    glm::vec3 pos = position();
    glm::vec3 up{0.0f, 1.0f, 0.0f};

    // Flip up vector near poles to avoid flip artifact
    if (std::abs(m_pitch) > 89.0f) {
        up = (m_pitch > 0.0f) ? glm::vec3(0.0f, 1.0f, 0.0f)
                               : glm::vec3(0.0f, -1.0f, 0.0f);
    }
    return glm::lookAt(pos, m_target, up);
}

// ─── Camera::projectionMatrix ─────────────────────────────────────────────────

glm::mat4 Camera::projectionMatrix(float aspect) const
{
    return glm::perspective(glm::radians(m_fov), aspect, m_near, m_far);
}

// ─── Camera::processDrag ──────────────────────────────────────────────────────

void Camera::processDrag(float dx, float dy)
{
    m_yaw   += dx * m_mouseSensitivity;
    m_pitch += dy * m_mouseSensitivity;

    // Clamp pitch to avoid flipping over the poles
    m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);
}

// ─── Camera::processScroll ────────────────────────────────────────────────────

void Camera::processScroll(float offset)
{
    m_distance -= offset * m_zoomSensitivity * m_distance;
    m_distance  = std::clamp(m_distance, m_minDistance, m_maxDistance);
}
