# 3D Corpus Visualiser

An interactive platform for exploring 3D anatomical models. Students and educators can upload, label, and annotate complex anatomical structures in a real-time OpenGL viewport.

---

## Milestone 1 — CMake + Window + ImGui Boilerplate

The current state of the repository implements **Milestone 1**: a clean, terminal-runnable C++17 OpenGL application skeleton using CMake, GLFW, GLAD, and Dear ImGui.

### Project layout

```
corpus-3d-visualizer/
├── CMakeLists.txt          # C++17 build, GLFW + GLAD + ImGui
├── README.md
├── .gitignore
├── external/
│   ├── glad/               # GLAD GL loader (OpenGL 3.3 Core, bundled)
│   └── imgui/              # Dear ImGui v1.91.9 (bundled)
├── src/
│   ├── main.cpp            # Entry point
│   └── app/
│       ├── App.h           # Application class declaration
│       └── App.cpp         # GLFW window + GL context + ImGui lifecycle
├── assets/
│   ├── models/             # .obj + .mtl files (Milestone 2)
│   └── shaders/            # GLSL shaders (Milestone 2)
└── data/
    └── annotations.json    # Persisted labels (Milestone 5)
```

### System requirements

| Tool / Library | Minimum version |
|---|---|
| CMake | 3.16 |
| GCC / Clang / MSVC | C++17 support |
| GLFW | 3.3 (system package or bundled) |
| OpenGL | 3.3 Core (Mesa or GPU driver) |

#### Linux (Ubuntu / Debian)

```bash
sudo apt-get update
sudo apt-get install -y cmake build-essential libglfw3-dev libgl-dev
```

#### macOS

```bash
brew install cmake glfw
```

#### Windows

Install GLFW and add it to `CMAKE_PREFIX_PATH`, or let CMake's `find_package` locate a vcpkg installation.

---

### Build

```bash
# 1. Clone
git clone https://github.com/AryaNarkhede/corpus-3d-visualizer.git
cd corpus-3d-visualizer

# 2. Configure
cmake -S . -B build

# 3. Build
cmake --build build

# 4. Run
./build/bin/corpus_visualizer
```

On **Windows** with MSVC the binary will be at `build\bin\Debug\corpus_visualizer.exe`.

---

### Runtime behaviour (Milestone 1)

- Opens a 1280 × 720 window titled **"3D Corpus Visualiser"**.
- Clears to a dark background.
- Shows an ImGui panel listing the planned milestones.
- Closes cleanly (ESC or the window close button) with no crashes.

---

## Roadmap

| Milestone | Description |
|---|---|
| **1** ✅ | CMake + GLFW window + Dear ImGui boilerplate |
| 2 | OBJ model loading, Blinn-Phong shading, Arcball camera |
| 3 | FBO colour-picking pipeline (click → world coordinates) |
| 4 | ImGui annotation workflow (label + store) |
| 5 | JSON persistence (save / load annotations) |
