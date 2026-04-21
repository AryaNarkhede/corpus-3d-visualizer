#include "Mesh.h"

#include <glad/glad.h>

#include <utility>

// ─── Move semantics ───────────────────────────────────────────────────────────

Mesh::Mesh(Mesh&& o) noexcept
    : m_vao(o.m_vao), m_vbo(o.m_vbo), m_ebo(o.m_ebo),
      m_indexCount(o.m_indexCount), m_vertCount(o.m_vertCount),
      m_indexed(o.m_indexed)
{
    o.m_vao = o.m_vbo = o.m_ebo = 0;
}

Mesh& Mesh::operator=(Mesh&& o) noexcept
{
    if (this != &o) {
        destroy();
        m_vao        = o.m_vao;
        m_vbo        = o.m_vbo;
        m_ebo        = o.m_ebo;
        m_indexCount = o.m_indexCount;
        m_vertCount  = o.m_vertCount;
        m_indexed    = o.m_indexed;
        o.m_vao = o.m_vbo = o.m_ebo = 0;
    }
    return *this;
}

// ─── Mesh::upload ─────────────────────────────────────────────────────────────

void Mesh::upload(const std::vector<Vertex>&       vertices,
                  const std::vector<unsigned int>& indices)
{
    destroy();
    m_indexed    = true;
    m_indexCount = static_cast<GLsizei>(indices.size());
    m_vertCount  = static_cast<GLsizei>(vertices.size());

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)),
                 vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(indices.size() * sizeof(unsigned int)),
                 indices.data(), GL_STATIC_DRAW);

    // position (location = 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, position)));

    // normal (location = 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, normal)));

    glBindVertexArray(0);
}

// ─── Mesh::uploadNonIndexed ───────────────────────────────────────────────────

void Mesh::uploadNonIndexed(const std::vector<Vertex>& vertices)
{
    destroy();
    m_indexed   = false;
    m_vertCount = static_cast<GLsizei>(vertices.size());

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)),
                 vertices.data(), GL_STATIC_DRAW);

    // position (location = 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, position)));

    // normal (location = 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, normal)));

    glBindVertexArray(0);
}

// ─── Mesh::draw ───────────────────────────────────────────────────────────────

void Mesh::draw() const
{
    if (!m_vao) return;
    glBindVertexArray(m_vao);
    if (m_indexed) {
        glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, nullptr);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, m_vertCount);
    }
    glBindVertexArray(0);
}

// ─── Mesh::destroy ────────────────────────────────────────────────────────────

void Mesh::destroy()
{
    if (m_ebo) { glDeleteBuffers(1, &m_ebo);        m_ebo = 0; }
    if (m_vbo) { glDeleteBuffers(1, &m_vbo);        m_vbo = 0; }
    if (m_vao) { glDeleteVertexArrays(1, &m_vao);   m_vao = 0; }
    m_indexCount = m_vertCount = 0;
}
