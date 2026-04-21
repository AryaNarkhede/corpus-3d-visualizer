#include "App.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <cstdio>

// ─── GLFW callbacks ───────────────────────────────────────────────────────────

static void framebufferSizeCallback(GLFWwindow* /*window*/, int width, int height)
{
    glViewport(0, 0, width, height);
}

static void glfwErrorCallback(int error, const char* description)
{
    std::fprintf(stderr, "[GLFW error %d] %s\n", error, description);
}

// ─── App::init ────────────────────────────────────────────────────────────────

App::App() = default;

App::~App()
{
    shutdown();
}

bool App::init(int width, int height, const char* title)
{
    // ── GLFW ──────────────────────────────────────────────────────────────────
    glfwSetErrorCallback(glfwErrorCallback);
    if (!glfwInit()) {
        std::fprintf(stderr, "[App] Failed to initialise GLFW\n");
        return false;
    }

    // Request OpenGL 3.3 Core Profile
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

    // Resize callback – keeps viewport in sync with the framebuffer
    glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);

    // ── GLAD ──────────────────────────────────────────────────────────────────
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::fprintf(stderr, "[App] Failed to initialise GLAD\n");
        glfwDestroyWindow(m_window);
        glfwTerminate();
        return false;
    }
    std::printf("[App] OpenGL %s\n", reinterpret_cast<const char*>(glGetString(GL_VERSION)));

    // ── Dear ImGui ────────────────────────────────────────────────────────────
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    m_initialized = true;
    return true;
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

    // ── UI panels ─────────────────────────────────────────────────────────────
    // Milestone 1: simple placeholder panel
    ImGui::Begin("3D Corpus Visualiser");
    ImGui::Text("Milestone 1 — CMake + Window + ImGui boilerplate");
    ImGui::Separator();
    ImGui::TextDisabled("Milestone 2: Model loading + Blinn-Phong + Arcball camera");
    ImGui::TextDisabled("Milestone 3: FBO colour-picking pipeline");
    ImGui::TextDisabled("Milestone 4: Annotation workflow");
    ImGui::TextDisabled("Milestone 5: JSON persistence");
    ImGui::End();

    // ── Clear ─────────────────────────────────────────────────────────────────
    glClearColor(0.12f, 0.12f, 0.14f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // ── Render ImGui draw data ────────────────────────────────────────────────
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(m_window);
}

// ─── App::shutdown ────────────────────────────────────────────────────────────

void App::shutdown()
{
    if (!m_initialized) return;
    m_initialized = false;

    // Shutdown in reverse-init order
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(m_window);
    m_window = nullptr;
    glfwTerminate();
}
