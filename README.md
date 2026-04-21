# 3D Corpus Visualiser

An interactive platform for exploring 3D anatomical models. Students and educators can upload, label, and annotate complex anatomical structures in a real-time OpenGL viewport.

---

## Milestone 3 — FBO Colour-Picking Pipeline (click → world coordinates)

The current state of the repository implements **Milestone 3**, building on top of Milestones 1 & 2.

### What's new in Milestone 3

| Feature | Details |
|---|---|
| Picking FBO | Offscreen framebuffer with RGBA8 colour + depth24 attachments |
| Picking shaders | Flat-colour `picking.vert` / `picking.frag` (no lighting) |
| Colour-ID encoding | Integer object ID packed into RGB bytes (24-bit range) |
| Depth readback | Per-pixel depth read from the picking FBO |
| World coordinate recovery | Inverse(proj × view) reconstruction from NDC + depth |
| Right-click to pick | Right-click on the model returns a stable 3D world position |
| ImGui pick display | "Picking" panel shows world coords, object ID, and depth |
| Resize-safe FBO | Picking framebuffer auto-resizes with the window |

### Controls

| Input | Action |
|---|---|
| Left mouse drag | Orbit / rotate camera around model |
| Scroll wheel | Zoom in / out |
| Right-click | Pick a 3D point on the model surface |
| Distance slider | Fine-grained zoom control in the UI panel |

### Project layout

```
corpus-3d-visualizer/
├── CMakeLists.txt              # C++17 build (v0.3)
├── README.md
├── .gitignore
├── external/
│   ├── glad/                   # GLAD GL loader (OpenGL 3.3 Core, bundled)
│   ├── imgui/                  # Dear ImGui v1.91.9 (bundled)
│   └── tinyobjloader/          # tinyobjloader v2.0.0-rc13 single-header (bundled)
├── assets/
│   ├── models/
│   │   └── cube.obj            # Default test model (unit cube with normals)
│   └── shaders/
│       ├── mesh.vert           # Vertex shader (position, normal transform)
│       ├── mesh.frag           # Fragment shader (Blinn-Phong lighting)
│       ├── picking.vert        # Picking vertex shader (position only)
│       └── picking.frag        # Picking fragment shader (flat colour ID)
├── src/
│   ├── main.cpp
│   ├── app/
│   │   ├── App.h / .cpp        # Application class (Milestone 3 additions)
│   │   └── Types.h             # PickResult struct
│   ├── graphics/
│   │   ├── Lighting.h          # DirectionalLight + Material structs
│   │   ├── Shader.h / .cpp     # Compile, link, uniform API (+ setVec4)
│   │   ├── Mesh.h / .cpp       # VAO/VBO/EBO, indexed + non-indexed draw
│   │   ├── Model.h / .cpp      # tinyobjloader parsing, bounding sphere
│   │   ├── Camera.h / .cpp     # Arcball orbit camera
│   │   ├── Renderer.h / .cpp   # Blinn-Phong forward pass
│   │   └── Framebuffer.h / .cpp # FBO creation/resize/readback
│   └── interaction/
│       └── Picker.h / .cpp     # Colour-ID encode/decode, picking pipeline
└── data/
    └── annotations.json        # Persisted labels (Milestone 5)
```

### System requirements

| Tool / Library | Minimum version |
|---|---|
| CMake | 3.16 |
| GCC / Clang / MSVC | C++17 support |
| GLFW | 3.3 (system package or bundled) |
| OpenGL | 3.3 Core (Mesa or GPU driver) |
| GLM | 0.9.9 (system package or auto-fetched via FetchContent) |

#### Linux (Ubuntu / Debian)

```bash
sudo apt-get update
sudo apt-get install -y cmake build-essential libglfw3-dev libgl-dev libglm-dev
```

#### macOS

```bash
brew install cmake glfw glm
```

#### Windows (MSYS2 MinGW64)

```bash
pacman -S --needed \
  mingw-w64-x86_64-cmake \
  mingw-w64-x86_64-ninja \
  mingw-w64-x86_64-toolchain \
  mingw-w64-x86_64-glfw \
  mingw-w64-x86_64-glm
```

> **Note:** If GLM is not found by `find_package`, CMake automatically fetches it
> from GitHub via `FetchContent` (requires internet access during the first configure).

---

### Build

```bash
# 1. Clone
git clone https://github.com/AryaNarkhede/corpus-3d-visualizer.git
cd corpus-3d-visualizer

# 2. Configure
cmake -S . -B build

# 3. Build
cmake --build build -j

# 4. Run
./build/bin/corpus_visualizer
```

On **Windows** with MSVC the binary will be at `build\bin\Debug\corpus_visualizer.exe`.

---

### Loading your own OBJ model

1. Copy your `.obj` (and optional `.mtl`) file into `assets/models/`.
2. Launch the app.
3. In the **"3D Corpus Visualiser – Milestone 3"** ImGui panel, update the path in the
   text field (e.g. `/absolute/path/to/my_model.obj` or a relative path) and click
   **Load / Reload**.

The renderer auto-centres and normalises the model to fit a unit bounding sphere, so
any scale of OBJ file will display correctly.

---

### Runtime behaviour (Milestone 3)

- Opens a 1280 × 720 window.
- Loads `assets/models/cube.obj` by default (a lit unit cube).
- ImGui panel lets you change light direction, light colour, ambient strength, and
  material diffuse/specular/shininess in real time.
- Arcball camera: left-drag to orbit, scroll to zoom.
- **Right-click** on the model to pick a 3D world-space point. The "Picking" section
  of the panel displays the world coordinates, object ID, and depth value.
- Clicking the background returns no pick (displayed as "No pick result yet").
- Picking FBO automatically resizes when the window is resized.

---

## Roadmap

| Milestone | Description |
|---|---|
| **1** ✅ | CMake + GLFW window + Dear ImGui boilerplate |
| **2** ✅ | OBJ model loading, Blinn-Phong shading, Arcball camera |
| **3** ✅ | FBO colour-picking pipeline (click → world coordinates) |
| 4 | ImGui annotation workflow (label + store) |
| 5 | JSON persistence (save / load annotations) |
