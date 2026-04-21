# 3D Corpus Visualiser

An interactive platform for exploring 3D anatomical models. Students and educators can upload, label, and annotate complex anatomical structures in a real-time OpenGL viewport.

---

## Milestone 4 — ImGui Annotation Workflow

The current state of the repository implements **Milestone 4**, building on top of Milestones 1–3.

### What's new in Milestone 4

| Feature | Details |
|---|---|
| `Annotation` data model | `id`, `label`, `worldPos` (`glm::vec3`), `objectId` reference from pick result |
| `AnnotationStore` | In-memory add / remove / list with auto-incrementing IDs |
| Annotation panel | ImGui UI to enter a label, add annotation from the latest pick, and manage the list |
| Validation | Add button disabled when pick is invalid or label is empty/whitespace; inline feedback shown |
| Annotation list | Scrollable list with per-row delete button and world-position tooltip on hover |
| Row selection | Click a list row to highlight/select it (click again to deselect) |
| World-space markers | Bright yellow circular point sprites rendered in the 3-D scene at each annotation position |
| Input isolation | ImGui `WantCaptureMouse` prevents camera drag / picking when interacting with UI widgets |

### How to create annotations from picks

1. Right-click on the model surface to pick a 3-D point.
2. The **Picking** section confirms "Hit!" and shows the world coordinates.
3. In the **Annotations** section, type a label into the **Label** text box.
4. Click **Add Annotation**. A yellow dot appears at the picked world position.
5. The new annotation appears in the scrollable list below.

### Controls summary

| Input | Action |
|---|---|
| Left mouse drag | Orbit / rotate camera around model |
| Scroll wheel | Zoom in / out |
| Right-click on model | Pick a 3-D world-space point |
| Distance slider | Fine-grained zoom control in the UI panel |
| Label field + Add Annotation | Create an annotation from the current pick |
| Click list row | Select / deselect an annotation entry |
| Delete (per row) | Remove that annotation from the store and scene |
| Load / Reload | Load a new OBJ model (clears all annotations) |

### Annotation list actions

- **Add** — Creates a new annotation from the latest valid pick plus the typed label.
  - Blocked if no valid pick exists ("Pick a point first" feedback).
  - Blocked if the label is empty or whitespace-only ("Label cannot be empty" feedback).
- **Select** — Click any row to highlight it; click again to deselect.
- **Delete** — The **Delete** button on each row removes the annotation immediately; its marker disappears from the 3-D scene on the next frame.
- **Clear on reload** — Loading a new model resets all annotations.

### Project layout

```
corpus-3d-visualizer/
├── CMakeLists.txt              # C++17 build (v0.4)
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
│       ├── picking.frag        # Picking fragment shader (flat colour ID)
│       ├── marker.vert         # Marker vertex shader (point sprite sizing)
│       └── marker.frag         # Marker fragment shader (circular point discard)
├── src/
│   ├── main.cpp
│   ├── app/
│   │   ├── App.h / .cpp        # Application class (Milestone 4 additions)
│   │   ├── AnnotationStore.h / .cpp  # In-memory annotation store (add/remove/list)
│   │   └── Types.h             # PickResult + Annotation structs
│   ├── graphics/
│   │   ├── Lighting.h          # DirectionalLight + Material structs
│   │   ├── Shader.h / .cpp     # Compile, link, uniform API
│   │   ├── Mesh.h / .cpp       # VAO/VBO/EBO, indexed + non-indexed draw
│   │   ├── Model.h / .cpp      # tinyobjloader parsing, bounding sphere
│   │   ├── Camera.h / .cpp     # Arcball orbit camera
│   │   ├── Renderer.h / .cpp   # Blinn-Phong forward pass
│   │   ├── Framebuffer.h / .cpp # FBO creation/resize/readback
│   │   └── MarkerRenderer.h / .cpp  # Annotation point-sprite markers
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
3. In the **"3D Corpus Visualiser – Milestone 4"** ImGui panel, update the path in the
   text field (e.g. `/absolute/path/to/my_model.obj` or a relative path) and click
   **Load / Reload**.

The renderer auto-centres and normalises the model to fit a unit bounding sphere, so
any scale of OBJ file will display correctly.

---

### Runtime behaviour (Milestone 4)

- Opens a 1280 × 720 window.
- Loads `assets/models/cube.obj` by default (a lit unit cube).
- ImGui panel lets you change light direction, light colour, ambient strength, and
  material diffuse/specular/shininess in real time.
- Arcball camera: left-drag to orbit, scroll to zoom.
- **Right-click** on the model to pick a 3-D world-space point.
- **Annotations section**: type a label and click **Add Annotation** to store the point.
  Yellow circular markers appear in the 3-D view at each annotation position.
  The scrollable list supports row selection and per-row deletion.
- Camera drag and picking are fully isolated from ImGui widget interaction.

---

## Roadmap

| Milestone | Description |
|---|---|
| **1** ✅ | CMake + GLFW window + Dear ImGui boilerplate |
| **2** ✅ | OBJ model loading, Blinn-Phong shading, Arcball camera |
| **3** ✅ | FBO colour-picking pipeline (click → world coordinates) |
| **4** ✅ | ImGui annotation workflow (label + store + scene markers) |
| 5 | JSON persistence (save / load annotations) |
