#include "App.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <cstdio>
#include <cstring>
#include <string>

// ─── Default model and shader paths ──────────────────────────────────────────
// ASSETS_DIR is injected by CMake as an absolute path to the assets/ folder.
#ifndef ASSETS_DIR
#  define ASSETS_DIR "assets"
#endif

static const char* kDefaultModelPath  = ASSETS_DIR "/models/cube.obj";
static const char* kShaderDir         = ASSETS_DIR "/shaders";

// ─── GLFW error callback ─────────────────────────────────────────────────────

static void glfwErrorCallback(int error, const char* description)
{
    std::fprintf(stderr, "[GLFW error %d] %s\n", error, description);
}

// ─── App::App / App::~App ─────────────────────────────────────────────────────

App::App()
{
    std::strncpy(m_modelPathBuf, kDefaultModelPath, sizeof(m_modelPathBuf) - 1);
}

App::~App()
{
    shutdown();
}

// ─── App::init ────────────────────────────────────────────────────────────────

bool App::init(int width, int height, const char* title)
{
    // ── GLFW ──────────────────────────────────────────────────────────────────
    glfwSetErrorCallback(glfwErrorCallback);
    if (!glfwInit()) {
        std::fprintf(stderr, "[App] Failed to initialise GLFW\n");
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!m_window) {
        std::fprintf(stderr, "[App] Failed to create GLFW window\n");
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1); // vsync

    // Store pointer so static callbacks can reach this instance
    glfwSetWindowUserPointer(m_window, this);

    glfwGetFramebufferSize(m_window, &m_fbWidth, &m_fbHeight);
    glfwSetFramebufferSizeCallback(m_window, cbFramebufferSize);
    glfwSetScrollCallback        (m_window, cbScroll);
    glfwSetCursorPosCallback     (m_window, cbCursorPos);
    glfwSetMouseButtonCallback   (m_window, cbMouseButton);

    // ── GLAD ──────────────────────────────────────────────────────────────────
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::fprintf(stderr, "[App] Failed to initialise GLAD\n");
        glfwDestroyWindow(m_window);
        glfwTerminate();
        return false;
    }
    std::printf("[App] OpenGL %s\n",
                reinterpret_cast<const char*>(glGetString(GL_VERSION)));

    // ── Dear ImGui ────────────────────────────────────────────────────────────
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // ── Renderer ──────────────────────────────────────────────────────────────
    m_rendererReady = m_renderer.init(kShaderDir);
    if (!m_rendererReady) {
        m_statusMsg = "ERROR: Could not load shaders from: " + std::string(kShaderDir);
        std::fprintf(stderr, "[App] %s\n", m_statusMsg.c_str());
    }

    // ── Load default model ────────────────────────────────────────────────────
    loadModel(kDefaultModelPath);

    m_initialized = true;
    return true;
}

// ─── App::loadModel ───────────────────────────────────────────────────────────

void App::loadModel(const std::string& path)
{
    if (m_model.load(path)) {
        // Fit camera to bounding sphere
        float r = (m_model.radius() > 0.0f) ? m_model.radius() : 1.0f;
        m_camera.setTarget({0.0f, 0.0f, 0.0f}); // model is centred by renderer
        m_camera.setDistance(r * 3.0f);
        m_statusMsg = "Loaded: " + path;
    } else {
        m_statusMsg = "Could not load model: " + path
                      + "  Place an .obj file at that path and click Reload.";
    }
}

// ─── App::run ─────────────────────────────────────────────────────────────────

void App::run()
{
    if (!m_initialized) return;

    while (!glfwWindowShouldClose(m_window)) {
        glfwPollEvents();
        processFrame();
    }
}

// ─── App::processFrame ────────────────────────────────────────────────────────

void App::processFrame()
{
    // ── ImGui new frame ───────────────────────────────────────────────────────
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    buildUi();

    // ── 3-D render pass ───────────────────────────────────────────────────────
    if (m_rendererReady) {
        m_renderer.draw(m_model, m_camera, m_fbWidth, m_fbHeight);
    } else {
        glClearColor(0.12f, 0.12f, 0.14f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    // ── ImGui render (on top of scene) ────────────────────────────────────────
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(m_window);
}

// ─── App::buildUi ─────────────────────────────────────────────────────────────

void App::buildUi()
{
    ImGui::SetNextWindowPos({10.0f, 10.0f}, ImGuiCond_Once);
    ImGui::SetNextWindowSize({420.0f, 0.0f}, ImGuiCond_Once);
    ImGui::Begin("3D Corpus Visualiser – Milestone 2");

    // ── Model section ─────────────────────────────────────────────────────────
    ImGui::SeparatorText("Model");
    ImGui::InputText("##modelpath", m_modelPathBuf, sizeof(m_modelPathBuf));
    ImGui::SameLine();
    if (ImGui::Button("Load / Reload")) {
        loadModel(std::string(m_modelPathBuf));
    }

    if (!m_statusMsg.empty()) {
        ImGui::TextWrapped("%s", m_statusMsg.c_str());
    }

    // ── Camera section ────────────────────────────────────────────────────────
    ImGui::SeparatorText("Camera");
    ImGui::Text("Left-drag: rotate   Scroll: zoom");
    float dist = m_camera.distance();
    if (ImGui::SliderFloat("Distance", &dist, 0.1f, 50.0f))
        m_camera.setDistance(dist);

    // ── Lighting section ──────────────────────────────────────────────────────
    ImGui::SeparatorText("Lighting");
    auto& light = m_renderer.light();
    ImGui::DragFloat3("Light dir",  &light.direction.x, 0.01f, -1.0f, 1.0f);
    ImGui::ColorEdit3("Light color",&light.color.x);
    ImGui::SliderFloat("Ambient",   &light.ambientStrength, 0.0f, 1.0f);

    // ── Material section ──────────────────────────────────────────────────────
    ImGui::SeparatorText("Material");
    auto& mat = m_renderer.material();
    ImGui::ColorEdit3("Diffuse",  &mat.diffuse.x);
    ImGui::ColorEdit3("Specular", &mat.specular.x);
    ImGui::SliderFloat("Shininess", &mat.shininess, 1.0f, 256.0f);

    // ── Roadmap ───────────────────────────────────────────────────────────────
    ImGui::SeparatorText("Roadmap");
    ImGui::TextDisabled("Milestone 3: FBO colour-picking pipeline");
    ImGui::TextDisabled("Milestone 4: Annotation workflow");
    ImGui::TextDisabled("Milestone 5: JSON persistence");

    ImGui::End();
}

// ─── App::shutdown ────────────────────────────────────────────────────────────

void App::shutdown()
{
    if (!m_initialized) return;
    m_initialized = false;

    m_renderer.shutdown();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(m_window);
    m_window = nullptr;
    glfwTerminate();
}

// ─── Static GLFW callbacks ────────────────────────────────────────────────────

void App::cbFramebufferSize(GLFWwindow* w, int width, int height)
{
    auto* app = static_cast<App*>(glfwGetWindowUserPointer(w));
    if (app) {
        app->m_fbWidth  = width;
        app->m_fbHeight = height;
    }
    glViewport(0, 0, width, height);
}

void App::cbScroll(GLFWwindow* w, double /*xoffset*/, double yoffset)
{
    auto* app = static_cast<App*>(glfwGetWindowUserPointer(w));
    if (!app) return;

    // Don't consume scroll if ImGui wants it
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) return;

    app->m_camera.processScroll(static_cast<float>(yoffset));
}

void App::cbMouseButton(GLFWwindow* w, int button, int action, int /*mods*/)
{
    auto* app = static_cast<App*>(glfwGetWindowUserPointer(w));
    if (!app) return;

    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) {
        app->m_dragging = false;
        return;
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            app->m_dragging = true;
            double x, y;
            glfwGetCursorPos(w, &x, &y);
            app->m_lastMouseX = static_cast<float>(x);
            app->m_lastMouseY = static_cast<float>(y);
        } else if (action == GLFW_RELEASE) {
            app->m_dragging = false;
        }
    }
}

void App::cbCursorPos(GLFWwindow* w, double xpos, double ypos)
{
    auto* app = static_cast<App*>(glfwGetWindowUserPointer(w));
    if (!app || !app->m_dragging) return;

    // If ImGui just captured the mouse, stop dragging
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) {
        app->m_dragging = false;
        return;
    }

    float dx = static_cast<float>(xpos) - app->m_lastMouseX;
    float dy = static_cast<float>(ypos) - app->m_lastMouseY;

    // Invert dy so dragging up rotates up (standard orbit convention)
    app->m_camera.processDrag(dx, -dy);

    app->m_lastMouseX = static_cast<float>(xpos);
    app->m_lastMouseY = static_cast<float>(ypos);
}

