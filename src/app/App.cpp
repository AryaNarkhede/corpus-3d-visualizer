#include "App.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <glm/gtc/matrix_transform.hpp>

#include "io/JsonPersistence.h"

#include <cstdio>
#include <cstring>
#include <cmath>
#include <string>

// ─── Default model and shader paths ──────────────────────────────────────────
// ASSETS_DIR is injected by CMake as an absolute path to the assets/ folder.
#ifndef ASSETS_DIR
#  define ASSETS_DIR "assets"
#endif

// DATA_DIR is injected by CMake as an absolute path to the data/ folder.
#ifndef DATA_DIR
#  define DATA_DIR "data"
#endif

static const char* kDefaultModelPath    = ASSETS_DIR "/models/cube.obj";
static const char* kShaderDir           = ASSETS_DIR "/shaders";
static const char* kDefaultAnnotPath    = DATA_DIR   "/annotations.json";

// ─── trimString ──────────────────────────────────────────────────────────────
// Returns a copy of s with leading and trailing whitespace removed.
// An empty or whitespace-only string returns "".

static std::string trimString(const std::string& s)
{
    auto first = s.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    auto last = s.find_last_not_of(" \t\r\n");
    return s.substr(first, last - first + 1);
}

// ─── GLFW error callback ─────────────────────────────────────────────────────

static void glfwErrorCallback(int error, const char* description)
{
    std::fprintf(stderr, "[GLFW error %d] %s\n", error, description);
}

// ─── App::App / App::~App ─────────────────────────────────────────────────────

App::App()
{
    std::strncpy(m_modelPathBuf, kDefaultModelPath, sizeof(m_modelPathBuf) - 1);
    std::strncpy(m_jsonPathBuf,  kDefaultAnnotPath, sizeof(m_jsonPathBuf)  - 1);
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

    // ── Picker (Milestone 3) ──────────────────────────────────────────────────
    m_pickerReady = m_picker.init(kShaderDir, m_fbWidth, m_fbHeight);
    if (!m_pickerReady) {
        std::fprintf(stderr, "[App] Picker initialisation failed\n");
    }

    // ── Marker renderer (Milestone 4) ─────────────────────────────────────────
    m_markerReady = m_markerRenderer.init(kShaderDir);
    if (!m_markerReady) {
        std::fprintf(stderr, "[App] MarkerRenderer initialisation failed\n");
    }

    // ── Load default model ────────────────────────────────────────────────────
    loadModel(kDefaultModelPath);

    m_initialized = true;
    return true;
}

// ─── App::computeModelMatrix ──────────────────────────────────────────────────

glm::mat4 App::computeModelMatrix() const
{
    float scale = (m_model.radius() > 0.0f) ? (1.0f / m_model.radius()) : 1.0f;
    return glm::scale(
        glm::translate(glm::mat4(1.0f), -m_model.center()),
        glm::vec3(scale));
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
        m_lastPick = {};
        m_annotStore.clear();           // reset annotations for the new model
        m_selectedAnnotId = -1;
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

    // ── Picking pass (offscreen FBO, before scene render) ─────────────────────
    if (m_rendererReady && m_pickerReady && m_model.isLoaded()) {
        glm::mat4 modelMat = computeModelMatrix();
        m_picker.renderPickingPass(m_model, m_camera, modelMat,
                                   m_fbWidth, m_fbHeight);

        if (m_pickRequested) {
            m_pickRequested = false;

            float aspect = (m_fbHeight > 0)
                               ? static_cast<float>(m_fbWidth) / static_cast<float>(m_fbHeight)
                               : 1.0f;
            glm::mat4 proj = m_camera.projectionMatrix(aspect);
            glm::mat4 view = m_camera.viewMatrix();

            m_lastPick = m_picker.pick(m_pickMouseX, m_pickMouseY,
                                        m_fbHeight, proj, view);

            if (m_lastPick.valid) {
                std::printf("[App] Pick hit: world(%.3f, %.3f, %.3f) id=%d depth=%.4f\n",
                            m_lastPick.worldPos.x, m_lastPick.worldPos.y,
                            m_lastPick.worldPos.z, m_lastPick.objectId,
                            m_lastPick.depth);
            }
        }
    }

    // ── 3-D render pass (default framebuffer) ─────────────────────────────────
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, m_fbWidth, m_fbHeight);

    if (m_rendererReady) {
        m_renderer.draw(m_model, m_camera, m_fbWidth, m_fbHeight);
    } else {
        glClearColor(0.12f, 0.12f, 0.14f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    // ── Annotation markers (Milestone 4) ──────────────────────────────────────
    if (m_markerReady && !m_annotStore.all().empty()) {
        float aspect = (m_fbHeight > 0)
                           ? static_cast<float>(m_fbWidth) / static_cast<float>(m_fbHeight)
                           : 1.0f;
        glm::mat4 viewProj = m_camera.projectionMatrix(aspect) * m_camera.viewMatrix();

        const auto& annotations = m_annotStore.all();

        // Resolve the selected annotation index within the positions array.
        int selectedIndex = -1;
        if (m_selectedAnnotId >= 0) {
            for (int i = 0; i < static_cast<int>(annotations.size()); ++i) {
                if (annotations[i].id == m_selectedAnnotId) {
                    selectedIndex = i;
                    break;
                }
            }
        }

        // Handle a pending left-click: project markers to screen and pick nearest.
        if (m_markerSelectRequested) {
            m_markerSelectRequested = false;
            constexpr float kPickRadius = 20.0f; // pixels
            int    bestId   = -1;
            float  bestDist = kPickRadius;

            for (const auto& a : annotations) {
                glm::vec4 clip = viewProj * glm::vec4(a.worldPos, 1.0f);
                if (clip.w <= 0.0f) continue; // behind camera
                glm::vec3 ndc = glm::vec3(clip) / clip.w;
                float sx = ( ndc.x * 0.5f + 0.5f) * static_cast<float>(m_fbWidth);
                float sy = (1.0f - (ndc.y * 0.5f + 0.5f)) * static_cast<float>(m_fbHeight);
                float dx = sx - static_cast<float>(m_markerSelectMX);
                float dy = sy - static_cast<float>(m_markerSelectMY);
                float dist = std::sqrt(dx * dx + dy * dy);
                if (dist < bestDist) {
                    bestDist = dist;
                    bestId   = a.id;
                }
            }

            // Toggle: clicking the already-selected marker deselects it.
            m_selectedAnnotId = (bestId != -1 && bestId != m_selectedAnnotId)
                                    ? bestId : -1;
        }

        std::vector<glm::vec3> positions;
        positions.reserve(annotations.size());
        for (const auto& a : annotations)
            positions.push_back(a.worldPos);

        m_markerRenderer.draw(positions, viewProj, selectedIndex);
    } else if (m_markerSelectRequested) {
        // No annotations: consume the pending request without doing anything.
        m_markerSelectRequested = false;
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
    ImGui::Begin("3D Corpus Visualiser – Milestone 5");

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

    // ── Picking section (Milestone 3) ─────────────────────────────────────────
    ImGui::SeparatorText("Picking");
    ImGui::Text("Right-click on the model to pick a 3D point.");

    if (m_lastPick.valid) {
        ImGui::TextColored({0.4f, 1.0f, 0.4f, 1.0f}, "Hit!");
        ImGui::Text("World: (%.3f, %.3f, %.3f)",
                    m_lastPick.worldPos.x, m_lastPick.worldPos.y,
                    m_lastPick.worldPos.z);
        ImGui::Text("Object ID: %d", m_lastPick.objectId);
        ImGui::Text("Depth: %.4f", m_lastPick.depth);
    } else {
        ImGui::TextDisabled("No pick result yet.");
    }

    // ── Annotations section (Milestone 4 + 5) ────────────────────────────────
    buildAnnotationUi();

    ImGui::End();
}

// ─── App::buildAnnotationUi ───────────────────────────────────────────────────
// Sub-section rendered inside the main ImGui window.
// Allows the user to create an annotation from the latest valid pick result,
// see all annotations in a scrollable list, and delete individual entries.

void App::buildAnnotationUi()
{
    ImGui::SeparatorText("Annotations");

    // ── Label input ───────────────────────────────────────────────────────────
    ImGui::Text("Label:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-1.0f);
    ImGui::InputText("##annotlabel", m_annotLabelBuf, sizeof(m_annotLabelBuf));

    // ── Validation & Add button ───────────────────────────────────────────────
    // Determine whether creation is currently possible and why not.
    bool pickInvalid = !m_lastPick.valid;
    std::string trimmedLabel = trimString(m_annotLabelBuf);
    bool labelEmpty  = trimmedLabel.empty();
    bool canAdd = !pickInvalid && !labelEmpty;

    if (!canAdd) ImGui::BeginDisabled();
    if (ImGui::Button("Add Annotation")) {
        m_annotStore.add(trimmedLabel, m_lastPick.worldPos, m_lastPick.objectId);
        m_annotLabelBuf[0] = '\0';
        m_annotErrorMsg.clear();
    }
    if (!canAdd) ImGui::EndDisabled();

    // Show inline validation feedback.
    if (pickInvalid) {
        ImGui::SameLine();
        ImGui::TextColored({1.0f, 0.5f, 0.2f, 1.0f}, "Pick a point first");
    } else if (labelEmpty) {
        ImGui::SameLine();
        ImGui::TextColored({1.0f, 0.5f, 0.2f, 1.0f}, "Label cannot be empty");
    }

    if (!m_annotErrorMsg.empty()) {
        ImGui::TextColored({1.0f, 0.3f, 0.3f, 1.0f}, "%s", m_annotErrorMsg.c_str());
    }

    // ── Annotation list ───────────────────────────────────────────────────────
    const auto& annotations = m_annotStore.all();
    ImGui::Text("Annotations: %d", static_cast<int>(annotations.size()));

    // Fixed-height scrollable child region for the list.
    ImGui::BeginChild("##annotlist", {0.0f, 160.0f}, true);

    if (annotations.empty()) {
        ImGui::TextDisabled("No annotations yet.");
    } else {
        int toDelete = -1; // collect the ID to delete (avoids modifying while iterating)
        for (const auto& a : annotations) {
            bool selected = (a.id == m_selectedAnnotId);

            // Selectable row — clicking highlights the entry.
            char rowLabel[64];
            std::snprintf(rowLabel, sizeof(rowLabel), "[%d] %s", a.id, a.label.c_str());
            if (ImGui::Selectable(rowLabel, selected,
                                  ImGuiSelectableFlags_None, {0.0f, 0.0f})) {
                m_selectedAnnotId = selected ? -1 : a.id; // toggle
            }

            // World-position tooltip on hover.
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("World: (%.3f, %.3f, %.3f)  ObjID: %d",
                                  a.worldPos.x, a.worldPos.y, a.worldPos.z,
                                  a.objectId);
            }

            // Delete button on the same row.
            ImGui::SameLine();
            char btnId[32];
            std::snprintf(btnId, sizeof(btnId), "Delete##%d", a.id);
            if (ImGui::SmallButton(btnId)) {
                toDelete = a.id;
            }
        }

        if (toDelete != -1) {
            m_annotStore.remove(toDelete);
            if (m_selectedAnnotId == toDelete) m_selectedAnnotId = -1;
        }
    }

    ImGui::EndChild();

    // ── Persistence section (Milestone 5) ────────────────────────────────────
    ImGui::SeparatorText("Persistence");

    // Editable file path.
    ImGui::Text("File:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-1.0f);
    ImGui::InputText("##jsonpath", m_jsonPathBuf, sizeof(m_jsonPathBuf));

    // Save / Load buttons on the same row.
    if (ImGui::Button("Save")) {
        saveAnnotations();
    }
    ImGui::SameLine();
    if (ImGui::Button("Load")) {
        loadAnnotations();
    }

    // Status message with colour coding.
    if (!m_persistMsg.empty()) {
        if (m_persistSuccess)
            ImGui::TextColored({0.4f, 1.0f, 0.4f, 1.0f}, "%s", m_persistMsg.c_str());
        else
            ImGui::TextColored({1.0f, 0.3f, 0.3f, 1.0f}, "%s", m_persistMsg.c_str());
    }
}

// ─── App::saveAnnotations ─────────────────────────────────────────────────────

void App::saveAnnotations()
{
    std::string path = trimString(m_jsonPathBuf);
    if (path.empty()) path = kDefaultAnnotPath;

    auto result = JsonPersistence::saveAnnotations(m_annotStore.all(), path);
    m_persistMsg     = result.message;
    m_persistSuccess = result.success;
}

// ─── App::loadAnnotations ─────────────────────────────────────────────────────

void App::loadAnnotations()
{
    std::string path = trimString(m_jsonPathBuf);
    if (path.empty()) path = kDefaultAnnotPath;

    std::vector<Annotation> loaded;
    auto result = JsonPersistence::loadAnnotations(path, loaded);
    m_persistMsg     = result.message;
    m_persistSuccess = result.success;

    if (result.success) {
        m_annotStore.replaceAll(std::move(loaded));
        m_selectedAnnotId = -1;
    }
}

// ─── App::shutdown ────────────────────────────────────────────────────────────

void App::shutdown()
{
    if (!m_initialized) return;
    m_initialized = false;

    m_picker.shutdown();
    m_markerRenderer.shutdown();
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
        if (app->m_pickerReady)
            app->m_picker.resize(width, height);
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
            app->m_lastMouseX  = static_cast<float>(x);
            app->m_lastMouseY  = static_cast<float>(y);
            app->m_leftPressMX = static_cast<float>(x);
            app->m_leftPressMY = static_cast<float>(y);
        } else if (action == GLFW_RELEASE) {
            app->m_dragging = false;

            // If the mouse barely moved this is a click, not a drag.
            // Schedule a marker proximity test for the current cursor position.
            double x, y;
            glfwGetCursorPos(w, &x, &y);
            float dx = static_cast<float>(x) - app->m_leftPressMX;
            float dy = static_cast<float>(y) - app->m_leftPressMY;
            if (dx * dx + dy * dy < 25.0f) { // within 5-pixel radius
                app->m_markerSelectRequested = true;
                app->m_markerSelectMX = static_cast<int>(x);
                app->m_markerSelectMY = static_cast<int>(y);
            }
        }
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        double x, y;
        glfwGetCursorPos(w, &x, &y);
        app->m_pickRequested = true;
        app->m_pickMouseX    = static_cast<int>(x);
        app->m_pickMouseY    = static_cast<int>(y);
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
