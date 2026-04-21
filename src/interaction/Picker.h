#pragma once

#include "app/Types.h"
#include "graphics/Framebuffer.h"
#include "graphics/Shader.h"

#include <glm/glm.hpp>

class Model;
class Camera;

class Picker {
public:
    bool init(const std::string& shaderDir, int width, int height);
    void resize(int width, int height);

    void renderPickingPass(const Model& model, const Camera& camera,
                           const glm::mat4& modelMat,
                           int viewportW, int viewportH);

    PickResult pick(int mouseX, int mouseY,
                    int screenH,
                    const glm::mat4& proj,
                    const glm::mat4& view) const;

    void shutdown();

    static glm::vec4 encodeId(int id);
    static int       decodeId(unsigned char r, unsigned char g, unsigned char b);

private:
    Framebuffer m_fbo;
    Shader      m_shader;
};
