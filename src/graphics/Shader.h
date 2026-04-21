#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <unordered_map>

// ─── Shader ───────────────────────────────────────────────────────────────────
// Compiles and links a GLSL vertex + fragment shader program.
// Provides typed uniform setters that cache location lookups.
// ─────────────────────────────────────────────────────────────────────────────
class Shader {
public:
    Shader()  = default;
    ~Shader() { destroy(); }

    // Not copyable
    Shader(const Shader&)            = delete;
    Shader& operator=(const Shader&) = delete;

    // Movable
    Shader(Shader&& o) noexcept : m_program(o.m_program) { o.m_program = 0; }
    Shader& operator=(Shader&& o) noexcept {
        if (this != &o) { destroy(); m_program = o.m_program; o.m_program = 0; }
        return *this;
    }

    // Load shader source from files and link.  Returns false on failure.
    bool load(const std::string& vertPath, const std::string& fragPath);

    // Bind this program for subsequent draw calls.
    void use() const;

    bool isValid() const { return m_program != 0; }

    // ── Typed uniform setters ─────────────────────────────────────────────────
    void setInt  (const std::string& name, int           v) const;
    void setFloat(const std::string& name, float         v) const;
    void setVec3 (const std::string& name, const glm::vec3& v) const;
    void setMat3 (const std::string& name, const glm::mat3& m) const;
    void setMat4 (const std::string& name, const glm::mat4& m) const;

private:
    GLuint m_program = 0;

    void destroy();
    GLint loc(const std::string& name) const;

    mutable std::unordered_map<std::string, GLint> m_locCache;
};
