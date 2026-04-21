#pragma once

#include "graphics/Camera.h"
#include "graphics/Model.h"
#include "graphics/Renderer.h"

#include <string>

// Forward declarations
struct GLFWwindow;

// ─── App ──────────────────────────────────────────────────────────────────────
// Application orchestrator.
// Milestone 2 adds: OBJ model loading, Blinn-Phong renderer, Arcball camera.
//
// Future milestones will extend this class with:
//   - Milestone 3: FBO colour-picking pipeline
//   - Milestone 4: Annotation workflow (ImGui panels + AnnotationStore)
//   - Milestone 5: JSON persistence (save / load)
// ─────────────────────────────────────────────────────────────────────────────
class App {
public:
    App();
    ~App();

    // Initialize window, GL context, and ImGui. Returns false on failure.
    bool init(int width = 1280, int height = 720,
              const char* title = "3D Corpus Visualiser");

    // Run the main event/render loop until the window is closed.
    void run();

    // Release all resources (called automatically by destructor).
    void shutdown();

private:
    void processFrame();
    void buildUi();
    void loadModel(const std::string& path);

    // ── GLFW callbacks (routed through the window user pointer) ───────────────
    static void cbFramebufferSize(GLFWwindow* w, int width, int height);
    static void cbScroll(GLFWwindow* w, double xoffset, double yoffset);
    static void cbCursorPos(GLFWwindow* w, double xpos, double ypos);
    static void cbMouseButton(GLFWwindow* w, int button, int action, int mods);

    // ── Window / GL state ─────────────────────────────────────────────────────
    GLFWwindow* m_window      = nullptr;
    bool        m_initialized = false;
    int         m_fbWidth     = 1280;
    int         m_fbHeight    = 720;

    // ── Milestone 2: graphics systems ─────────────────────────────────────────
    Camera   m_camera;
    Model    m_model;
    Renderer m_renderer;

    // ── Arcball mouse state ────────────────────────────────────────────────────
    bool  m_dragging       = false;
    float m_lastMouseX     = 0.0f;
    float m_lastMouseY     = 0.0f;

    // ── UI state ──────────────────────────────────────────────────────────────
    char        m_modelPathBuf[512] = {};
    bool        m_rendererReady     = false;
    std::string m_statusMsg;

    // ── Milestone 3: Framebuffer (FBO) and Picker will be added here ──────────
    // ── Milestone 4: AnnotationStore and UiLayer will be added here ───────────
    // ── Milestone 5: JsonPersistence will be added here ───────────────────────
};
