#include "Shader.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <sstream>
#include <cstdio>

// ─── Helpers ──────────────────────────────────────────────────────────────────

static std::string readFile(const std::string& path)
{
    std::ifstream f(path);
    if (!f.is_open()) {
        std::fprintf(stderr, "[Shader] Cannot open file: %s\n", path.c_str());
        return {};
    }
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

static GLuint compileStage(GLenum type, const std::string& src, const std::string& path)
{
    const char* srcPtr = src.c_str();
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &srcPtr, nullptr);
    glCompileShader(shader);

    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        std::fprintf(stderr, "[Shader] Compile error in %s:\n%s\n", path.c_str(), log);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

// ─── Shader::load ─────────────────────────────────────────────────────────────

bool Shader::load(const std::string& vertPath, const std::string& fragPath)
{
    destroy();

    std::string vertSrc = readFile(vertPath);
    std::string fragSrc = readFile(fragPath);
    if (vertSrc.empty() || fragSrc.empty()) return false;

    GLuint vert = compileStage(GL_VERTEX_SHADER,   vertSrc, vertPath);
    GLuint frag = compileStage(GL_FRAGMENT_SHADER, fragSrc, fragPath);
    if (!vert || !frag) {
        glDeleteShader(vert);
        glDeleteShader(frag);
        return false;
    }

    m_program = glCreateProgram();
    glAttachShader(m_program, vert);
    glAttachShader(m_program, frag);
    glLinkProgram(m_program);
    glDeleteShader(vert);
    glDeleteShader(frag);

    GLint ok = 0;
    glGetProgramiv(m_program, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetProgramInfoLog(m_program, sizeof(log), nullptr, log);
        std::fprintf(stderr, "[Shader] Link error:\n%s\n", log);
        glDeleteProgram(m_program);
        m_program = 0;
        return false;
    }

    m_locCache.clear();
    std::printf("[Shader] Loaded: %s + %s\n", vertPath.c_str(), fragPath.c_str());
    return true;
}

// ─── Shader::use ──────────────────────────────────────────────────────────────

void Shader::use() const
{
    glUseProgram(m_program);
}

// ─── Shader::destroy ──────────────────────────────────────────────────────────

void Shader::destroy()
{
    if (m_program) {
        glDeleteProgram(m_program);
        m_program = 0;
        m_locCache.clear();
    }
}

// ─── Shader::loc ──────────────────────────────────────────────────────────────

GLint Shader::loc(const std::string& name) const
{
    auto it = m_locCache.find(name);
    if (it != m_locCache.end()) return it->second;
    GLint l = glGetUniformLocation(m_program, name.c_str());
    m_locCache[name] = l;
    return l;
}

// ─── Shader::set* ─────────────────────────────────────────────────────────────

void Shader::setInt(const std::string& name, int v) const
{
    glUniform1i(loc(name), v);
}

void Shader::setFloat(const std::string& name, float v) const
{
    glUniform1f(loc(name), v);
}

void Shader::setVec3(const std::string& name, const glm::vec3& v) const
{
    glUniform3fv(loc(name), 1, glm::value_ptr(v));
}

void Shader::setVec4(const std::string& name, const glm::vec4& v) const
{
    glUniform4fv(loc(name), 1, glm::value_ptr(v));
}

void Shader::setMat3(const std::string& name, const glm::mat3& m) const
{
    glUniformMatrix3fv(loc(name), 1, GL_FALSE, glm::value_ptr(m));
}

void Shader::setMat4(const std::string& name, const glm::mat4& m) const
{
    glUniformMatrix4fv(loc(name), 1, GL_FALSE, glm::value_ptr(m));
}
