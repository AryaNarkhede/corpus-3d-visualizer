#pragma once

// Forward declarations
struct GLFWwindow;

// ─── App ──────────────────────────────────────────────────────────────────────
// Application orchestrator for Milestone 1.
// Owns the GLFW window, OpenGL context, and ImGui lifecycle.
//
// Future milestones will extend this class with:
//   - Milestone 2: Model loading, Blinn-Phong renderer, Arcball camera
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

    GLFWwindow* m_window = nullptr;
    bool        m_initialized = false;

    // ── Placeholder state for future milestones ──
    // Milestone 2: Renderer, Model, Camera will be added here
    // Milestone 3: Framebuffer (FBO) and Picker will be added here
    // Milestone 4: AnnotationStore and UiLayer will be added here
    // Milestone 5: JsonPersistence will be added here
};
