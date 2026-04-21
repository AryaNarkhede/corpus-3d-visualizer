cmake_minimum_required(VERSION 3.16)
project(corpus_3d_visualizer VERSION 0.1.0 LANGUAGES C CXX)

# ── C++ standard ──────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── Output directories ────────────────────────────────────────────────────────
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ─────────────────────────────────────────────────────────────────────────────
# Dependencies
# ─────────────────────────────────────────────────────────────────────────────

# ── OpenGL ────────────────────────────────────────────────────────────────────
find_package(OpenGL REQUIRED)

# ── GLFW ─────────────────────────────────────────────────────────────────────
find_package(glfw3 3.3 QUIET)
if(NOT glfw3_FOUND)
    # Fall back to pkg-config (common on Linux)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GLFW REQUIRED glfw3)
    add_library(glfw INTERFACE)
    target_include_directories(glfw INTERFACE ${GLFW_INCLUDE_DIRS})
    target_link_libraries(glfw INTERFACE ${GLFW_LIBRARIES})
    target_compile_options(glfw INTERFACE ${GLFW_CFLAGS_OTHER})
    add_library(glfw3::glfw3 ALIAS glfw)
endif()

# ── GLAD (bundled in external/glad) ──────────────────────────────────────────
add_library(glad STATIC
    external/glad/src/glad.c
)
target_include_directories(glad PUBLIC external/glad/include)

# ── Dear ImGui (bundled in external/imgui) ───────────────────────────────────
add_library(imgui STATIC
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/imgui_demo.cpp
    external/imgui/imgui_impl_glfw.cpp
    external/imgui/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC external/imgui)
# ImGui backends need GLFW and OpenGL headers
if(TARGET glfw3::glfw3)
    target_link_libraries(imgui PUBLIC glfw3::glfw3 OpenGL::GL)
else()
    target_link_libraries(imgui PUBLIC glfw OpenGL::GL)
endif()
target_include_directories(imgui PRIVATE external/glad/include)

# ─────────────────────────────────────────────────────────────────────────────
# Application target
# ─────────────────────────────────────────────────────────────────────────────
add_executable(corpus_visualizer
    src/main.cpp
    src/app/App.cpp
)

target_include_directories(corpus_visualizer PRIVATE src)

if(TARGET glfw3::glfw3)
    target_link_libraries(corpus_visualizer PRIVATE
        glfw3::glfw3
        OpenGL::GL
        glad
        imgui
    )
else()
    target_link_libraries(corpus_visualizer PRIVATE
        glfw
        OpenGL::GL
        glad
        imgui
    )
endif()

# On Linux we also need dl for GLAD's dlopen usage
if(UNIX AND NOT APPLE)
    target_link_libraries(corpus_visualizer PRIVATE ${CMAKE_DL_LIBS})
endif()

# ─────────────────────────────────────────────────────────────────────────────
# Compiler warnings
# ─────────────────────────────────────────────────────────────────────────────
if(MSVC)
    target_compile_options(corpus_visualizer PRIVATE /W4)
else()
    target_compile_options(corpus_visualizer PRIVATE -Wall -Wextra -Wpedantic)
endif()
