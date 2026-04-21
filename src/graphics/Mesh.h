#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <vector>

// ─── Vertex ───────────────────────────────────────────────────────────────────
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
};

// ─── Mesh ─────────────────────────────────────────────────────────────────────
// Owns one VAO/VBO/EBO on the GPU.
// ─────────────────────────────────────────────────────────────────────────────
class Mesh {
public:
    Mesh()  = default;
    ~Mesh() { destroy(); }

    // Not copyable
    Mesh(const Mesh&)            = delete;
    Mesh& operator=(const Mesh&) = delete;

    // Movable
    Mesh(Mesh&& o) noexcept;
    Mesh& operator=(Mesh&& o) noexcept;

    // Upload geometry to the GPU.
    void upload(const std::vector<Vertex>&       vertices,
                const std::vector<unsigned int>& indices);

    // Upload geometry without explicit indices (indexed by draw order).
    void uploadNonIndexed(const std::vector<Vertex>& vertices);

    // Issue a draw call (uses the bound shader program).
    void draw() const;

    bool isValid() const { return m_vao != 0; }

private:
    GLuint  m_vao        = 0;
    GLuint  m_vbo        = 0;
    GLuint  m_ebo        = 0;
    GLsizei m_indexCount = 0;
    GLsizei m_vertCount  = 0;
    bool    m_indexed    = false;

    void destroy();
};
