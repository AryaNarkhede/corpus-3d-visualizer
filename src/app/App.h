#pragma once

#include "Types.h"
#include "AnnotationStore.h"
#include "graphics/Camera.h"
#include "graphics/LineRenderer.h"
#include "graphics/MarkerRenderer.h"
#include "graphics/Model.h"
#include "graphics/Renderer.h"
#include "interaction/Picker.h"

#include <array>
#include <string>

// Forward declarations
struct GLFWwindow;

// ─── App ──────────────────────────────────────────────────────────────────────
// Application orchestrator.
// Milestone 5 adds: JSON persistence (save / load annotations).
// Milestone 6 adds: screenshot export, measurement tool, search/filter,
//                   keyboard shortcuts.
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
    void buildAnnotationUi();          // Annotation panel sub-section
    void buildMeasureUi();             // Measurement panel sub-section
    void loadModel(const std::string& path);

    // ── Persistence helpers (Milestone 5) ─────────────────────────────────────
    void saveAnnotations();
    void loadAnnotations();

    // ── Screenshot helper (Milestone 6) ───────────────────────────────────────
    void takeScreenshot();

    // ── GLFW callbacks (routed through the window user pointer) ───────────────
    static void cbFramebufferSize(GLFWwindow* w, int width, int height);
    static void cbScroll(GLFWwindow* w, double xoffset, double yoffset);
    static void cbCursorPos(GLFWwindow* w, double xpos, double ypos);
    static void cbMouseButton(GLFWwindow* w, int button, int action, int mods);
    static void cbKey(GLFWwindow* w, int key, int scancode, int action, int mods);

    // ── Window / GL state ─────────────────────────────────────────────────────
    GLFWwindow* m_window      = nullptr;
    bool        m_initialized = false;
    int         m_fbWidth     = 1280;
    int         m_fbHeight    = 720;

    // ── Graphics systems ──────────────────────────────────────────────────────
    Camera         m_camera;
    Model          m_model;
    Renderer       m_renderer;

    // ── Arcball mouse state ────────────────────────────────────────────────────
    bool  m_dragging       = false;
    float m_lastMouseX     = 0.0f;
    float m_lastMouseY     = 0.0f;

    // ── UI state ──────────────────────────────────────────────────────────────
    char        m_modelPathBuf[512] = {};
    bool        m_rendererReady     = false;
    std::string m_statusMsg;

    // ── Milestone 3: FBO picking pipeline ─────────────────────────────────────
    Picker     m_picker;
    bool       m_pickerReady    = false;
    bool       m_pickRequested  = false;
    int        m_pickMouseX     = 0;
    int        m_pickMouseY     = 0;
    PickResult m_lastPick;

    glm::mat4 computeModelMatrix() const;

    // ── Milestone 4: Annotation workflow ──────────────────────────────────────
    AnnotationStore m_annotStore;
    MarkerRenderer  m_markerRenderer;
    bool            m_markerReady     = false;
    char            m_annotLabelBuf[256] = {};
    std::string     m_annotErrorMsg;
    int             m_selectedAnnotId = -1;

    // ── Milestone 5: JSON persistence ─────────────────────────────────────────
    char        m_jsonPathBuf[512] = {};   // editable file path for save/load
    std::string m_persistMsg;              // save/load status message shown in UI
    bool        m_persistSuccess = true;   // colours the status message

    // ── Milestone 6a: Screenshot ──────────────────────────────────────────────
    std::string m_screenshotMsg;           // result of last screenshot
    bool        m_screenshotSuccess = true;

    // ── Milestone 6b: Measurement tool ────────────────────────────────────────
    InteractionMode m_mode = InteractionMode::Annotate;

    // Up to 3 measurement points (indices 0-2).
    // m_measureCount tracks how many have been picked.
    static constexpr int kMaxMeasPts = 3;
    std::array<glm::vec3, kMaxMeasPts> m_measPts{};
    int  m_measCount = 0;      // 0 = no points, 1 = A picked, 2 = A+B, 3 = A+B+C
    bool m_measValid = false;  // true when at least 2 points are available

    LineRenderer m_lineRenderer;
    bool         m_lineRendererReady = false;

    // ── Milestone 6c: Search / filter annotations ─────────────────────────────
    char m_annotFilterBuf[128] = {};   // substring filter for annotation list
};

