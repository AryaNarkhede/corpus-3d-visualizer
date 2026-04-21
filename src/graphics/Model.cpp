#define TINYOBJLOADER_IMPLEMENTATION
#include "Model.h"

#include <tiny_obj_loader.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cstdio>
#include <unordered_map>

// ─── Model::load ──────────────────────────────────────────────────────────────

bool Model::load(const std::string& path)
{
    m_loaded = false;
    m_meshes.clear();

    tinyobj::ObjReaderConfig cfg;
    cfg.mtl_search_path = "./";
    cfg.triangulate     = true;

    tinyobj::ObjReader reader;
    if (!reader.ParseFromFile(path, cfg)) {
        if (!reader.Error().empty())
            std::fprintf(stderr, "[Model] TinyObjReader error: %s\n", reader.Error().c_str());
        else
            std::fprintf(stderr, "[Model] Failed to load: %s\n", path.c_str());
        return false;
    }
    if (!reader.Warning().empty())
        std::fprintf(stderr, "[Model] TinyObjReader warning: %s\n", reader.Warning().c_str());

    const auto& attrib = reader.GetAttrib();
    const auto& shapes = reader.GetShapes();

    bool hasNormals = !attrib.normals.empty();
    if (!hasNormals)
        std::fprintf(stderr, "[Model] No normals in '%s' — computing flat normals.\n",
                     path.c_str());

    // Collect all vertices across all shapes into a single flat mesh.
    // (Per-shape splitting is straightforward to extend for Milestone 3+ ID encoding.)
    std::vector<Vertex>       vertices;
    std::vector<unsigned int> indices;

    // Key: (pos_idx, nrm_idx) → vertex buffer index
    std::unordered_map<size_t, unsigned int> indexMap;

    for (const auto& shape : shapes) {
        const auto& mesh = shape.mesh;
        size_t indexOffset = 0;

        for (size_t f = 0; f < mesh.num_face_vertices.size(); ++f) {
            int fv = mesh.num_face_vertices[f]; // always 3 after triangulation

            // Gather triangle positions (needed for flat-normal computation)
            glm::vec3 triPos[3];
            for (int v = 0; v < fv; ++v) {
                tinyobj::index_t idx = mesh.indices[indexOffset + v];
                triPos[v] = {
                    attrib.vertices[3 * idx.vertex_index + 0],
                    attrib.vertices[3 * idx.vertex_index + 1],
                    attrib.vertices[3 * idx.vertex_index + 2]
                };
            }

            // Flat normal (used when OBJ has none)
            glm::vec3 flatN = glm::normalize(
                glm::cross(triPos[1] - triPos[0], triPos[2] - triPos[0]));

            for (int v = 0; v < fv; ++v) {
                tinyobj::index_t idx = mesh.indices[indexOffset + v];

                glm::vec3 nrm = flatN;
                if (hasNormals && idx.normal_index >= 0) {
                    nrm = {
                        attrib.normals[3 * idx.normal_index + 0],
                        attrib.normals[3 * idx.normal_index + 1],
                        attrib.normals[3 * idx.normal_index + 2]
                    };
                    nrm = glm::normalize(nrm);
                }

                // Build a composite key so vertices sharing pos+nrm can be merged.
                size_t key = (static_cast<size_t>(idx.vertex_index) << 20)
                             ^ static_cast<size_t>(idx.normal_index < 0
                                                   ? 0xFFFFF
                                                   : idx.normal_index);

                auto it = indexMap.find(key);
                if (it != indexMap.end()) {
                    indices.push_back(it->second);
                } else {
                    unsigned int newIdx = static_cast<unsigned int>(vertices.size());
                    indexMap[key]       = newIdx;
                    indices.push_back(newIdx);
                    vertices.push_back({triPos[v], nrm});
                }
            }
            indexOffset += static_cast<size_t>(fv);
        }
    }

    if (vertices.empty()) {
        std::fprintf(stderr, "[Model] No geometry found in '%s'\n", path.c_str());
        return false;
    }

    computeBounds(vertices);

    Mesh mesh;
    mesh.upload(vertices, indices);
    m_meshes.push_back(std::move(mesh));

    m_loaded = true;
    std::printf("[Model] Loaded '%s': %zu vertices, %zu indices\n",
                path.c_str(), vertices.size(), indices.size());
    return true;
}

// ─── Model::draw ──────────────────────────────────────────────────────────────

void Model::draw() const
{
    for (const auto& m : m_meshes)
        m.draw();
}

// ─── Model::computeBounds ─────────────────────────────────────────────────────

void Model::computeBounds(const std::vector<Vertex>& verts)
{
    glm::vec3 mn = verts[0].position;
    glm::vec3 mx = verts[0].position;
    for (const auto& v : verts) {
        mn = glm::min(mn, v.position);
        mx = glm::max(mx, v.position);
    }
    m_center = (mn + mx) * 0.5f;
    m_radius = glm::length(mx - mn) * 0.5f;
}
