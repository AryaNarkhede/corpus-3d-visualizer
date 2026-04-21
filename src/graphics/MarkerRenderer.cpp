#include "MarkerRenderer.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include <cstdio>

// ─── MarkerRenderer::init ─────────────────────────────────────────────────────

bool MarkerRenderer::init(const std::string& shaderDir)
{
    const std::string vert = shaderDir + "/marker.vert";
    const std::string frag = shaderDir + "/marker.frag";

    if (!m_shader.load(vert, frag)) {
        std::fprintf(stderr, "[MarkerRenderer] Failed to load marker shaders\n");
        return false;
    }

    // Allocate a VAO + VBO for streaming point positions.
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    // Attribute 0: vec3 position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          sizeof(glm::vec3),
                          reinterpret_cast<void*>(0));
    glBindVertexArray(0);

    return true;
}

// ─── MarkerRenderer::draw ─────────────────────────────────────────────────────

void MarkerRenderer::draw(const std::vector<glm::vec3>& positions,
                           const glm::mat4&              viewProj) const
{
    if (positions.empty() || !m_shader.isValid()) return;

    // Stream positions to GPU.
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(positions.size() * sizeof(glm::vec3)),
                 positions.data(),
                 GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Enable point-sprite sizing from the vertex shader.
    glEnable(GL_PROGRAM_POINT_SIZE);

    m_shader.use();
    m_shader.setMat4("uViewProjection", viewProj);
    m_shader.setVec4("uColor", {1.0f, 0.9f, 0.2f, 1.0f}); // bright yellow

    glBindVertexArray(m_vao);
    glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(positions.size()));
    glBindVertexArray(0);

    glDisable(GL_PROGRAM_POINT_SIZE);
}

// ─── MarkerRenderer::shutdown ─────────────────────────────────────────────────

void MarkerRenderer::shutdown()
{
    if (m_vbo) { glDeleteBuffers(1, &m_vbo);       m_vbo = 0; }
    if (m_vao) { glDeleteVertexArrays(1, &m_vao);  m_vao = 0; }
}
