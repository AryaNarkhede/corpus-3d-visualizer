#include "LineRenderer.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include <cstdio>

// ─── LineRenderer::init ───────────────────────────────────────────────────────

bool LineRenderer::init(const std::string& shaderDir)
{
    const std::string vert = shaderDir + "/line.vert";
    const std::string frag = shaderDir + "/line.frag";

    if (!m_shader.load(vert, frag)) {
        std::fprintf(stderr, "[LineRenderer] Failed to load line shaders\n");
        return false;
    }

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

// ─── LineRenderer::begin ──────────────────────────────────────────────────────

void LineRenderer::begin()
{
    m_vertices.clear();
}

// ─── LineRenderer::addSegment ─────────────────────────────────────────────────

void LineRenderer::addSegment(const glm::vec3& a, const glm::vec3& b)
{
    m_vertices.push_back(a);
    m_vertices.push_back(b);
}

// ─── LineRenderer::draw ───────────────────────────────────────────────────────

void LineRenderer::draw(const glm::mat4& viewProj, const glm::vec4& color) const
{
    if (m_vertices.empty() || !m_shader.isValid()) return;

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(m_vertices.size() * sizeof(glm::vec3)),
                 m_vertices.data(),
                 GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glLineWidth(2.0f); // wider line for visibility (clamped by driver on some hw)

    m_shader.use();
    m_shader.setMat4("uViewProjection", viewProj);
    m_shader.setVec4("uColor", color);

    glBindVertexArray(m_vao);
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(m_vertices.size()));
    glBindVertexArray(0);
}

// ─── LineRenderer::shutdown ───────────────────────────────────────────────────

void LineRenderer::shutdown()
{
    if (m_vbo) { glDeleteBuffers(1, &m_vbo);       m_vbo = 0; }
    if (m_vao) { glDeleteVertexArrays(1, &m_vao);  m_vao = 0; }
}
