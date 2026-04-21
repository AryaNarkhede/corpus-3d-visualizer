#include "Picker.h"

#include "graphics/Model.h"
#include "graphics/Camera.h"

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cstdio>

bool Picker::init(const std::string& shaderDir, int width, int height)
{
    const std::string vert = shaderDir + "/picking.vert";
    const std::string frag = shaderDir + "/picking.frag";

    if (!m_shader.load(vert, frag)) {
        std::fprintf(stderr, "[Picker] Failed to load picking shaders\n");
        return false;
    }
    if (!m_fbo.create(width, height)) {
        std::fprintf(stderr, "[Picker] Failed to create picking FBO\n");
        return false;
    }
    return true;
}

void Picker::resize(int width, int height)
{
    m_fbo.resize(width, height);
}

void Picker::renderPickingPass(const Model& model, const Camera& camera,
                                const glm::mat4& modelMat,
                                int viewportW, int viewportH)
{
    if (!model.isLoaded() || !m_shader.isValid()) return;

    m_fbo.resize(viewportW, viewportH);
    m_fbo.bind();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    float aspect = (viewportH > 0)
                       ? static_cast<float>(viewportW) / static_cast<float>(viewportH)
                       : 1.0f;
    glm::mat4 view = camera.viewMatrix();
    glm::mat4 proj = camera.projectionMatrix(aspect);

    m_shader.use();
    m_shader.setMat4("uModel",      modelMat);
    m_shader.setMat4("uView",       view);
    m_shader.setMat4("uProjection", proj);

    glm::vec4 pickColor = encodeId(1);
    m_shader.setVec4("uPickColor", pickColor);

    model.draw();

    m_fbo.unbind();
}

PickResult Picker::pick(int mouseX, int mouseY,
                         int screenH,
                         const glm::mat4& proj,
                         const glm::mat4& view) const
{
    PickResult result;

    int fbX = mouseX;
    int fbY = screenH - mouseY - 1;

    if (fbX < 0 || fbX >= m_fbo.width() ||
        fbY < 0 || fbY >= m_fbo.height()) {
        return result;
    }

    unsigned char rgba[4] = {};
    m_fbo.readPixel(fbX, fbY, rgba);

    int id = decodeId(rgba[0], rgba[1], rgba[2]);
    if (id == 0) return result;

    float depth = m_fbo.readDepth(fbX, fbY);
    if (depth >= 1.0f) return result;

    float ndcX = (2.0f * static_cast<float>(fbX) / static_cast<float>(m_fbo.width()))  - 1.0f;
    float ndcY = (2.0f * static_cast<float>(fbY) / static_cast<float>(m_fbo.height())) - 1.0f;
    float ndcZ = 2.0f * depth - 1.0f;

    glm::vec4 clipPos(ndcX, ndcY, ndcZ, 1.0f);
    glm::mat4 invProjView = glm::inverse(proj * view);
    glm::vec4 worldPos4 = invProjView * clipPos;

    if (std::abs(worldPos4.w) < 1e-6f) return result;

    result.valid    = true;
    result.worldPos = glm::vec3(worldPos4) / worldPos4.w;
    result.objectId = id;
    result.depth    = depth;
    return result;
}

void Picker::shutdown()
{
}

glm::vec4 Picker::encodeId(int id)
{
    int r = (id >>  0) & 0xFF;
    int g = (id >>  8) & 0xFF;
    int b = (id >> 16) & 0xFF;
    return glm::vec4(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
}

int Picker::decodeId(unsigned char r, unsigned char g, unsigned char b)
{
    return static_cast<int>(r) |
           (static_cast<int>(g) << 8) |
           (static_cast<int>(b) << 16);
}
