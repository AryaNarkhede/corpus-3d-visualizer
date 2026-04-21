# 3D Corpus Visualiser

An interactive platform for exploring 3D anatomical models. Students and educators can upload, label, and annotate complex anatomical structures in a real-time OpenGL viewport.

---

## Milestone 5 — JSON Persistence + Final Hardening

The current state of the repository implements **Milestone 5**, building on top of Milestones 1–4.

### What's new in Milestone 5

| Feature | Details |
|---|---|
| JSON schema (v1) | `schema_version`, `id`, `label`, `position` (x/y/z), `object_id` |
| `JsonPersistence` module | `src/io/JsonPersistence.h/.cpp` — save/load with full validation |
| Save button | Serialises all annotations to the chosen file (default `data/annotations.json`) |
| Load button | Deserialises annotations; replaces current list; id counter updated to avoid collisions |
| Validation & error handling | Malformed JSON, wrong version, missing/invalid fields — all handled gracefully; app stays stable |
| UI status feedback | Green success / red error message shown after every save or load operation |
| Editable file path | Input field above Save/Load buttons lets the user point to any JSON file |
| Clean shutdown | No regressions to picking, rendering, or annotation markers from earlier milestones |

### JSON format

Annotations are stored in a versioned JSON file:

```json
{
  "schema_version": 1,
  "annotations": [
    {
      "id": 1,
      "label": "frontal lobe",
      "position": { "x": 0.123, "y": 0.456, "z": 0.789 },
      "object_id": 1
    },
    {
      "id": 2,
      "label": "temporal region",
      "position": { "x": -0.5, "y": 0.1, "z": 0.3 },
      "object_id": -1
    }
  ]
}
```

**Field reference**

| Field | Type | Required | Notes |
|---|---|---|---|
| `schema_version` | integer | ✅ | Must equal `1`; mismatches are rejected |
| `id` | integer | ✅ | Unique annotation identifier |
| `label` | string | ✅ | Human-readable name |
| `position` | object | ✅ | Must have numeric `x`, `y`, `z` |
| `object_id` | integer | optional | Defaults to `-1` if absent |

### How to save and load annotations

1. Create annotations via right-click + **Add Annotation** as before.
2. In the **Persistence** section, verify or edit the file path (default: `data/annotations.json`).
3. Click **Save** — a green status message confirms the number of annotations saved.
4. Close or restart the application.
5. Click **Load** — annotations are restored; yellow markers reappear in the 3-D scene.

### Behaviour on invalid files

| Scenario | Behaviour |
|---|---|
| File not found | Load fails; red error message; existing annotations unchanged |
| Malformed JSON | Parse error caught; red error message; app stays stable |
| Wrong `schema_version` | Rejected with descriptive message; no annotations loaded |
| Individual invalid entries | Skipped (logged to stderr); valid entries are still loaded; count shown in status |
| Cannot create `data/` directory | Save fails with clear error message |

### End-to-end demo

```bash
# 1. Build
cmake -S . -B build && cmake --build build -j

# 2. Run
./build/bin/corpus_visualizer

# 3. Right-click on the model → pick a world-space point
# 4. Enter a label in the Annotations panel → Add Annotation
# 5. Repeat for more annotations; yellow dots appear in the scene
# 6. Click Save  →  data/annotations.json is written
# 7. Close the app and re-run it
# 8. Click Load  →  annotations are restored with their original IDs and positions
```

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
| Save | Write annotations to the JSON file shown in the path field |
| Load | Read annotations from the JSON file shown in the path field |

### Annotation list actions

- **Add** — Creates a new annotation from the latest valid pick plus the typed label.
  - Blocked if no valid pick exists ("Pick a point first" feedback).
  - Blocked if the label is empty or whitespace-only ("Label cannot be empty" feedback).
- **Select** — Click any row to highlight it; click again to deselect.
- **Delete** — The **Delete** button on each row removes the annotation immediately; its marker disappears from the 3-D scene on the next frame.
- **Clear on reload** — Loading a new model resets all annotations.
- **Save** — Serialises all current annotations to the JSON file path.
- **Load** — Deserialises annotations from the JSON file path, replacing the current list.

### Project layout

```
corpus-3d-visualizer/
├── CMakeLists.txt              # C++17 build (v0.5)
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
├── data/
│   └── annotations.json        # Persisted labels (written by Save, read by Load)
├── src/
│   ├── main.cpp
│   ├── app/
│   │   ├── App.h / .cpp        # Application class (Milestone 5 additions)
│   │   ├── AnnotationStore.h / .cpp  # In-memory annotation store (add/remove/list/replaceAll)
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
│   ├── interaction/
│   │   └── Picker.h / .cpp     # Colour-ID encode/decode, picking pipeline
│   └── io/
│       └── JsonPersistence.h / .cpp  # JSON save/load with schema validation
```

### System requirements

| Tool / Library | Minimum version |
|---|---|
| CMake | 3.16 |
| GCC / Clang / MSVC | C++17 support |
| GLFW | 3.3 (system package or bundled) |
| OpenGL | 3.3 Core (Mesa or GPU driver) |
| GLM | 0.9.9 (system package or auto-fetched via FetchContent) |
| nlohmann/json | 3.10 (system package or auto-fetched via FetchContent) |

#### Linux (Ubuntu / Debian)

```bash
sudo apt-get update
sudo apt-get install -y cmake build-essential libglfw3-dev libgl-dev libglm-dev
```

#### macOS

```bash
brew install cmake glfw glm nlohmann-json
```

#### Windows (MSYS2 MinGW64)

```bash
pacman -S --needed \
  mingw-w64-x86_64-cmake \
  mingw-w64-x86_64-ninja \
  mingw-w64-x86_64-toolchain \
  mingw-w64-x86_64-glfw \
  mingw-w64-x86_64-glm \
  mingw-w64-x86_64-nlohmann-json
```

> **Note:** If GLM or nlohmann/json are not found by `find_package`, CMake automatically
> fetches them from GitHub via `FetchContent` (requires internet access during the first
> configure).

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

On **Windows** with MSVC the binary will be at `build\bin\corpus_visualizer.exe`.

---

### Loading your own OBJ model

1. Copy your `.obj` (and optional `.mtl`) file into `assets/models/`.
2. Launch the app.
3. In the **"3D Corpus Visualiser – Milestone 5"** ImGui panel, update the path in the
   text field (e.g. `/absolute/path/to/my_model.obj` or a relative path) and click
   **Load / Reload**.

The renderer auto-centres and normalises the model to fit a unit bounding sphere, so
any scale of OBJ file will display correctly.

---

### Runtime behaviour (Milestone 5)

- Opens a 1280 × 720 window.
- Loads `assets/models/cube.obj` by default (a lit unit cube).
- ImGui panel lets you change light direction, light colour, ambient strength, and
  material diffuse/specular/shininess in real time.
- Arcball camera: left-drag to orbit, scroll to zoom.
- **Right-click** on the model to pick a 3-D world-space point.
- **Annotations section**: type a label and click **Add Annotation** to store the point.
  Yellow circular markers (now larger, 20 px diameter) appear in the 3-D view at each
  annotation position.  The scrollable list supports row selection and per-row deletion.
- **Bidirectional selection**: clicking a label in the ImGui list highlights the
  corresponding marker in the scene (shown in white, 30 px).  Conversely, left-clicking
  a marker in the scene selects its entry in the list.  Clicking the same item again
  deselects it.
- **Persistence section**: edit the file path, click **Save** or **Load**.
  Status messages (green = success, red = error) appear immediately after each operation.
- Camera drag and picking are fully isolated from ImGui widget interaction.

---

## Roadmap

| Milestone | Description |
|---|---|
| **1** ✅ | CMake + GLFW window + Dear ImGui boilerplate |
| **2** ✅ | OBJ model loading, Blinn-Phong shading, Arcball camera |
| **3** ✅ | FBO colour-picking pipeline (click → world coordinates) |
| **4** ✅ | ImGui annotation workflow (label + store + scene markers) |
| **5** ✅ | JSON persistence (save / load annotations) + final hardening |
