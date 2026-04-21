#include "Renderer.h"
#include "Model.h"
#include "Camera.h"

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>

// ─── Renderer::init ───────────────────────────────────────────────────────────

bool Renderer::init(const std::string& shaderDir)
{
    const std::string vert = shaderDir + "/mesh.vert";
    const std::string frag = shaderDir + "/mesh.frag";

    if (!m_shader.load(vert, frag)) {
        return false;
    }

    // Enable depth testing for correct occlusion
    glEnable(GL_DEPTH_TEST);
    return true;
}

// ─── Renderer::draw ───────────────────────────────────────────────────────────

void Renderer::draw(const Model& model, const Camera& camera,
                    int viewportW, int viewportH) const
{
    glViewport(0, 0, viewportW, viewportH);
    glClearColor(0.12f, 0.12f, 0.14f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (!model.isLoaded() || !m_shader.isValid()) return;

    // ── Matrices ──────────────────────────────────────────────────────────────
    // Model matrix: centre and normalise the object so it fits in a unit sphere
    float scale = (model.radius() > 0.0f) ? (1.0f / model.radius()) : 1.0f;
    glm::mat4 modelMat = glm::scale(
        glm::translate(glm::mat4(1.0f), -model.center()),
        glm::vec3(scale));

    float aspect = (viewportH > 0)
                       ? static_cast<float>(viewportW) / static_cast<float>(viewportH)
                       : 1.0f;
    glm::mat4 view = camera.viewMatrix();
    glm::mat4 proj = camera.projectionMatrix(aspect);

    // Normal matrix: inverse-transpose of the upper-left 3×3 of model matrix
    glm::mat3 normalMat = glm::mat3(glm::transpose(glm::inverse(modelMat)));

    // ── Shader uniforms ───────────────────────────────────────────────────────
    m_shader.use();
    m_shader.setMat4("uModel",        modelMat);
    m_shader.setMat4("uView",         view);
    m_shader.setMat4("uProjection",   proj);
    m_shader.setMat3("uNormalMatrix", normalMat);

    m_shader.setVec3 ("uLightDir",         m_light.direction);
    m_shader.setVec3 ("uLightColor",       m_light.color);
    m_shader.setFloat("uAmbientStrength",  m_light.ambientStrength);

    m_shader.setVec3 ("uViewPos",          camera.position());
    m_shader.setVec3 ("uMatDiffuse",       m_material.diffuse);
    m_shader.setVec3 ("uMatSpecular",      m_material.specular);
    m_shader.setFloat("uMatShininess",     m_material.shininess);

    model.draw();
}

// ─── Renderer::shutdown ───────────────────────────────────────────────────────

void Renderer::shutdown()
{
    // Shader RAII destructor handles cleanup.
}
