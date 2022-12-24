#include "Obj_loader.h"

#define TINYOBJLOADER_IMPLEMENTATION
// TINYOBJLOADER_USE_MAPBOX_EARCUT: Enable better triangulation. Requires C++11
// #define TINYOBJLOADER_USE_MAPBOX_EARCUT
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "tiny_obj_loader.h"

// =========================================================================================
// Code adapted from tiny obj loader viewer
// https://github.com/tinyobjloader/tinyobjloader/tree/release/examples/viewer/viewer.cc
// =========================================================================================

static std::string GetBaseDir(const std::string& filepath) {
    if (filepath.find_last_of("/\\") != std::string::npos)
        return filepath.substr(0, filepath.find_last_of("/\\"));
    return "";
}

static bool FileExists(const std::string& abs_filename) {
    bool ret;
    FILE* fp = fopen(abs_filename.c_str(), "rb");
    if (fp) {
        ret = true;
        fclose(fp);
    } else {
        ret = false;
    }

    return ret;
}

static void CalcNormal(float N[3], float v0[3], float v1[3], float v2[3]) {
    float v10[3];
    v10[0] = v1[0] - v0[0];
    v10[1] = v1[1] - v0[1];
    v10[2] = v1[2] - v0[2];

    float v20[3];
    v20[0] = v2[0] - v0[0];
    v20[1] = v2[1] - v0[1];
    v20[2] = v2[2] - v0[2];

    N[0] = v10[1] * v20[2] - v10[2] * v20[1];
    N[1] = v10[2] * v20[0] - v10[0] * v20[2];
    N[2] = v10[0] * v20[1] - v10[1] * v20[0];

    float len2 = N[0] * N[0] + N[1] * N[1] + N[2] * N[2];
    if (len2 > 0.0f) {
        float len = sqrtf(len2);

        N[0] /= len;
        N[1] /= len;
        N[2] /= len;
    }
}

namespace  // Local utility functions
{
struct vec3 {
    float v[3];
    vec3() {
        v[0] = 0.0f;
        v[1] = 0.0f;
        v[2] = 0.0f;
    }
};

void normalizeVector(vec3& v) {
    float len2 = v.v[0] * v.v[0] + v.v[1] * v.v[1] + v.v[2] * v.v[2];
    if (len2 > 0.0f) {
        float len = sqrtf(len2);

        v.v[0] /= len;
        v.v[1] /= len;
        v.v[2] /= len;
    }
}

/*
  There are 2 approaches here to automatically generating vertex normals. The
  old approach (computeSmoothingNormals) doesn't handle multiple smoothing
  groups properly, as it effectively merges all smoothing groups present in the
  OBJ file into a single group. However, it can be useful when the OBJ file
  contains vertex normals which you want to use, but is missing some, as it
  will attempt to fill in the missing normals without generating new shapes.

  The new approach (computeSmoothingShapes, computeAllSmoothingNormals) handles
  multiple smoothing groups but is a bit more complicated, as handling this
  correctly requires potentially generating new vertices (and hence shapes).
  In general, the new approach is most useful if your OBJ file is missing
  vertex normals entirely, and instead relies on smoothing groups to correctly
  generate them as a pre-process. That said, it can be used to reliably
  generate vertex normals in the general case. If you want to always generate
  normals in this way, simply force set regen_all_normals to true below. By
  default, it's only true when there are no vertex normals present. One other
  thing to keep in mind is that the statistics printed apply to the model
  *prior* to shape regeneration, so you'd need to print them again if you want
  to see the new statistics.

  TODO(syoyo): import computeSmoothingShapes and computeAllSmoothingNormals to
  tinyobjloader as utility functions.
*/

// Check if `mesh_t` contains smoothing group id.
bool hasSmoothingGroup(const tinyobj::shape_t& shape) {
    for (size_t i = 0; i < shape.mesh.smoothing_group_ids.size(); i++) {
        if (shape.mesh.smoothing_group_ids[i] > 0) {
            return true;
        }
    }
    return false;
}

void computeSmoothingNormals(const tinyobj::attrib_t& attrib, const tinyobj::shape_t& shape,
                             std::map<int, vec3>& smoothVertexNormals) {
    smoothVertexNormals.clear();
    std::map<int, vec3>::iterator iter;

    for (size_t f = 0; f < shape.mesh.indices.size() / 3; f++) {
        // Get the three indexes of the face (all faces are triangular)
        tinyobj::index_t idx0 = shape.mesh.indices[3 * f + 0];
        tinyobj::index_t idx1 = shape.mesh.indices[3 * f + 1];
        tinyobj::index_t idx2 = shape.mesh.indices[3 * f + 2];

        // Get the three vertex indexes and coordinates
        int vi[3];      // indexes
        float v[3][3];  // coordinates

        for (int k = 0; k < 3; k++) {
            vi[0] = idx0.vertex_index;
            vi[1] = idx1.vertex_index;
            vi[2] = idx2.vertex_index;
            assert(vi[0] >= 0);
            assert(vi[1] >= 0);
            assert(vi[2] >= 0);

            v[0][k] = attrib.vertices[3 * vi[0] + k];
            v[1][k] = attrib.vertices[3 * vi[1] + k];
            v[2][k] = attrib.vertices[3 * vi[2] + k];
        }

        // Compute the normal of the face
        float normal[3];
        CalcNormal(normal, v[0], v[1], v[2]);

        // Add the normal to the three vertexes
        for (size_t i = 0; i < 3; ++i) {
            iter = smoothVertexNormals.find(vi[i]);
            if (iter != smoothVertexNormals.end()) {
                // add
                iter->second.v[0] += normal[0];
                iter->second.v[1] += normal[1];
                iter->second.v[2] += normal[2];
            } else {
                smoothVertexNormals[vi[i]].v[0] = normal[0];
                smoothVertexNormals[vi[i]].v[1] = normal[1];
                smoothVertexNormals[vi[i]].v[2] = normal[2];
            }
        }

    }  // f

    // Normalize the normals, that is, make them unit vectors
    for (iter = smoothVertexNormals.begin(); iter != smoothVertexNormals.end();
         iter++) {
        normalizeVector(iter->second);
    }

}  // computeSmoothingNormals

static void computeAllSmoothingNormals(tinyobj::attrib_t& attrib,
                                       std::vector<tinyobj::shape_t>& shapes) {
    vec3 p[3];
    for (size_t s = 0, slen = shapes.size(); s < slen; ++s) {
        const tinyobj::shape_t& shape(shapes[s]);
        size_t facecount = shape.mesh.num_face_vertices.size();
        assert(shape.mesh.smoothing_group_ids.size());

        for (size_t f = 0, flen = facecount; f < flen; ++f) {
            for (unsigned int v = 0; v < 3; ++v) {
                tinyobj::index_t idx = shape.mesh.indices[3 * f + v];
                assert(idx.vertex_index != -1);
                p[v].v[0] = attrib.vertices[3 * idx.vertex_index];
                p[v].v[1] = attrib.vertices[3 * idx.vertex_index + 1];
                p[v].v[2] = attrib.vertices[3 * idx.vertex_index + 2];
            }

            // cross(p[1] - p[0], p[2] - p[0])
            float nx = (p[1].v[1] - p[0].v[1]) * (p[2].v[2] - p[0].v[2]) -
                       (p[1].v[2] - p[0].v[2]) * (p[2].v[1] - p[0].v[1]);
            float ny = (p[1].v[2] - p[0].v[2]) * (p[2].v[0] - p[0].v[0]) -
                       (p[1].v[0] - p[0].v[0]) * (p[2].v[2] - p[0].v[2]);
            float nz = (p[1].v[0] - p[0].v[0]) * (p[2].v[1] - p[0].v[1]) -
                       (p[1].v[1] - p[0].v[1]) * (p[2].v[0] - p[0].v[0]);

            // Don't normalize here.
            for (unsigned int v = 0; v < 3; ++v) {
                tinyobj::index_t idx = shape.mesh.indices[3 * f + v];
                attrib.normals[3 * idx.normal_index] += nx;
                attrib.normals[3 * idx.normal_index + 1] += ny;
                attrib.normals[3 * idx.normal_index + 2] += nz;
            }
        }
    }

    assert(attrib.normals.size() % 3 == 0);
    for (size_t i = 0, nlen = attrib.normals.size() / 3; i < nlen; ++i) {
        tinyobj::real_t& nx = attrib.normals[3 * i];
        tinyobj::real_t& ny = attrib.normals[3 * i + 1];
        tinyobj::real_t& nz = attrib.normals[3 * i + 2];
        tinyobj::real_t len = sqrtf(nx * nx + ny * ny + nz * nz);
        tinyobj::real_t scale = len == 0 ? 0 : 1 / len;
        nx *= scale;
        ny *= scale;
        nz *= scale;
    }
}

static void computeSmoothingShape(tinyobj::attrib_t& inattrib, tinyobj::shape_t& inshape,
                                  std::vector<std::pair<unsigned int, unsigned int>>& sortedids,
                                  unsigned int idbegin, unsigned int idend,
                                  std::vector<tinyobj::shape_t>& outshapes,
                                  tinyobj::attrib_t& outattrib) {
    unsigned int sgroupid = sortedids[idbegin].first;
    bool hasmaterials = inshape.mesh.material_ids.size();
    // Make a new shape from the set of faces in the range [idbegin, idend).
    outshapes.emplace_back();
    tinyobj::shape_t& outshape = outshapes.back();
    outshape.name = inshape.name;
    // Skip lines and points.

    std::unordered_map<unsigned int, unsigned int> remap;
    for (unsigned int id = idbegin; id < idend; ++id) {
        unsigned int face = sortedids[id].second;

        outshape.mesh.num_face_vertices.push_back(3);  // always triangles
        if (hasmaterials)
            outshape.mesh.material_ids.push_back(inshape.mesh.material_ids[face]);
        outshape.mesh.smoothing_group_ids.push_back(sgroupid);
        // Skip tags.

        for (unsigned int v = 0; v < 3; ++v) {
            tinyobj::index_t inidx = inshape.mesh.indices[3 * face + v], outidx;
            assert(inidx.vertex_index != -1);
            auto iter = remap.find(inidx.vertex_index);
            // Smooth group 0 disables smoothing so no shared vertices in that case.
            if (sgroupid && iter != remap.end()) {
                outidx.vertex_index = (*iter).second;
                outidx.normal_index = outidx.vertex_index;
                outidx.texcoord_index = (inidx.texcoord_index == -1) ? -1 : outidx.vertex_index;
            } else {
                assert(outattrib.vertices.size() % 3 == 0);
                unsigned int offset = static_cast<unsigned int>(outattrib.vertices.size() / 3);
                outidx.vertex_index = outidx.normal_index = offset;
                outidx.texcoord_index = (inidx.texcoord_index == -1) ? -1 : offset;
                outattrib.vertices.push_back(inattrib.vertices[3 * inidx.vertex_index]);
                outattrib.vertices.push_back(inattrib.vertices[3 * inidx.vertex_index + 1]);
                outattrib.vertices.push_back(inattrib.vertices[3 * inidx.vertex_index + 2]);
                outattrib.normals.push_back(0.0f);
                outattrib.normals.push_back(0.0f);
                outattrib.normals.push_back(0.0f);
                if (inidx.texcoord_index != -1) {
                    outattrib.texcoords.push_back(inattrib.texcoords[2 * inidx.texcoord_index]);
                    outattrib.texcoords.push_back(inattrib.texcoords[2 * inidx.texcoord_index + 1]);
                }
                remap[inidx.vertex_index] = offset;
            }
            outshape.mesh.indices.push_back(outidx);
        }
    }
}

static void computeSmoothingShapes(tinyobj::attrib_t& inattrib,
                                   std::vector<tinyobj::shape_t>& inshapes,
                                   std::vector<tinyobj::shape_t>& outshapes,
                                   tinyobj::attrib_t& outattrib) {
    for (size_t s = 0, slen = inshapes.size(); s < slen; ++s) {
        tinyobj::shape_t& inshape = inshapes[s];

        unsigned int numfaces = static_cast<unsigned int>(inshape.mesh.smoothing_group_ids.size());
        assert(numfaces);
        std::vector<std::pair<unsigned int, unsigned int>> sortedids(numfaces);
        for (unsigned int i = 0; i < numfaces; ++i)
            sortedids[i] = std::make_pair(inshape.mesh.smoothing_group_ids[i], i);
        sort(sortedids.begin(), sortedids.end());

        unsigned int activeid = sortedids[0].first;
        unsigned int id = activeid, idbegin = 0, idend = 0;
        // Faces are now bundled by smoothing group id, create shapes from these.
        while (idbegin < numfaces) {
            while (activeid == id && ++idend < numfaces)
                id = sortedids[idend].first;
            computeSmoothingShape(inattrib, inshape, sortedids, idbegin, idend,
                                  outshapes, outattrib);
            activeid = id;
            idbegin = idend;
        }
    }
}

}  // namespace

std::string texture_filename(std::string fname, std::string base_dir) {
    std::string result;
    if (FileExists(fname.c_str())) {
        result = fname;
    } else {
        std::string texture_fname = base_dir + fname;
        if (FileExists(texture_fname.c_str())) {
            result = texture_fname;
        } else {
            std::cerr << "Unable to find texture file: " << fname << std::endl;
        }
    }
    return result;
}

// =========================================================================================

static bool load_obj_scene(string fname,
                           vector<Triangle>& outtriangles,
                           vector<Material>& outmaterials) {
    tinyobj::attrib_t inattrib;
    std::vector<tinyobj::material_t> inmaterials;
    std::vector<tinyobj::shape_t> inshapes;

    std::string base_dir = GetBaseDir(fname.c_str());
    if (base_dir.empty()) {
        base_dir = ".";
    }
#ifdef _WIN32
    base_dir += "\\";
#else
    base_dir += "/";
#endif

    std::string warn;
    std::string err;
    bool ret = tinyobj::LoadObj(&inattrib, &inshapes, &inmaterials, &warn, &err,
                                fname.c_str(),
                                base_dir.c_str());
    if (!warn.empty()) {
        std::cout << "WARN: " << warn << std::endl;
    }
    if (!err.empty()) {
        std::cerr << err << std::endl;
    }

    if (!ret) {
        std::cerr << "Failed to load " << fname << std::endl;
        return false;
    }

    printf("# of vertices  = %d\n", (int)(inattrib.vertices.size()) / 3);
    printf("# of normals   = %d\n", (int)(inattrib.normals.size()) / 3);
    printf("# of texcoords = %d\n", (int)(inattrib.texcoords.size()) / 2);
    printf("# of materials = %d\n", (int)inmaterials.size());
    printf("# of shapes    = %d\n", (int)inshapes.size());

    // ----------------------------------------------------------------------
    // Convert tinyobjloader materials into our material class
    // ----------------------------------------------------------------------

    for (size_t i = 0; i < inmaterials.size(); i++) {
        printf("material[%d].diffuse_texname = %s\n", int(i),
               inmaterials[i].diffuse_texname.c_str());
        Material m;
        m.setAmbient(glm::vec3(inmaterials[i].ambient[0],
                               inmaterials[i].ambient[1],
                               inmaterials[i].ambient[2]));
        m.setDiffuse(glm::vec3(inmaterials[i].diffuse[0],
                               inmaterials[i].diffuse[1],
                               inmaterials[i].diffuse[2]));
        m.setSpecular(glm::vec3(inmaterials[i].specular[0],
                                inmaterials[i].specular[1],
                                inmaterials[i].specular[2]));
        m.setShininess(inmaterials[i].shininess);
        m.setRefractionIndex(inmaterials[i].ior);
        // TODO: parse illum mode and set accordingly
        // m.setRefractivity(1.0f - (inmaterials[i].transmittance[0] + inmaterials[i].transmittance[1] + inmaterials[i].transmittance[2]) / 3.0f);
        m.setReflectivity(0.0);  // FIXME: guess from shininess?

        if (inmaterials[i].diffuse_texname.length() > 0) {
            std::string fname = texture_filename(inmaterials[i].diffuse_texname, base_dir);
            if (!fname.empty()) {
                m.setDiffuse(fname);
            }
        }
        // if (inmaterials[i].specular_texname.length() > 0) {
        //     std::string fname = texture_filename(inmaterials[i].specular_texname, base_dir);
        //     if (!fname.empty()) {
        //         m.setSpecular(fname);
        //     }
        // }
        if (inmaterials[i].ambient_texname.length() > 0) {
            std::string fname = texture_filename(inmaterials[i].ambient_texname, base_dir);
            if (!fname.empty()) {
                m.setAmbient(fname);
            }
        }
        if (inmaterials[i].bump_texname.length() > 0) {
            std::string fname = texture_filename(inmaterials[i].bump_texname, base_dir);
            if (!fname.empty()) {
                m.setNormal(fname);
            }
        }
        if (inmaterials[i].displacement_texname.length() > 0) {
            std::string fname = texture_filename(inmaterials[i].displacement_texname, base_dir);
            if (!fname.empty()) {
                m.setDisplacement(fname);
            }
        }

        outmaterials.push_back(m);
    }

    // Append `default` material at end
    Material default_material;
    default_material.setAmbient(glm::vec3(0.2f, 0.2f, 0.2f));
    default_material.setDiffuse(glm::vec3(1, 0, 1));
    outmaterials.push_back(default_material);

    // ----------------------------------------------------------------------
    // Recompute normals
    // ----------------------------------------------------------------------

    // recalculate normals if needed
    bool regen_all_normals = inattrib.normals.size() == 0;
    tinyobj::attrib_t outattrib;
    std::vector<tinyobj::shape_t> outshapes;
    if (regen_all_normals) {
        computeSmoothingShapes(inattrib, inshapes, outshapes, outattrib);
        computeAllSmoothingNormals(outattrib, outshapes);
    }

    std::vector<tinyobj::shape_t>& shapes = regen_all_normals ? outshapes : inshapes;
    tinyobj::attrib_t& attrib = regen_all_normals ? outattrib : inattrib;

    // ----------------------------------------------------------------------
    // Convert tinyobjloader shapes into our list of triangles
    // ----------------------------------------------------------------------

    // bounding box of the scene
    float bmin[3];
    float bmax[3];
    bmin[0] = bmin[1] = bmin[2] = std::numeric_limits<float>::max();
    bmax[0] = bmax[1] = bmax[2] = -std::numeric_limits<float>::max();

    {
        for (size_t s = 0; s < shapes.size(); s++) {
            // Check for smoothing group and compute smoothing normals
            std::map<int, vec3> smoothVertexNormals;
            if (!regen_all_normals && (hasSmoothingGroup(shapes[s]) > 0)) {
                std::cout << "Compute smoothingNormal for shape [" << s << "]" << std::endl;
                computeSmoothingNormals(attrib, shapes[s], smoothVertexNormals);
            }

            for (size_t f = 0; f < shapes[s].mesh.indices.size() / 3; f++) {
                // vertex indices
                tinyobj::index_t idx0 = shapes[s].mesh.indices[3 * f + 0];
                tinyobj::index_t idx1 = shapes[s].mesh.indices[3 * f + 1];
                tinyobj::index_t idx2 = shapes[s].mesh.indices[3 * f + 2];

                // material index
                int current_material_id = shapes[s].mesh.material_ids[f];

                if ((current_material_id < 0) ||
                    (current_material_id >= static_cast<int>(outmaterials.size()))) {
                    // Invaid material ID. Use default material.
                    current_material_id =
                        outmaterials.size() -
                        1;  // Default material is added to the last item in `materials`.
                }

                // texture coordinates
                float tc[3][2];
                if (attrib.texcoords.size() > 0) {
                    if ((idx0.texcoord_index < 0) || (idx1.texcoord_index < 0) ||
                        (idx2.texcoord_index < 0)) {
                        // face does not contain valid uv index.
                        tc[0][0] = 0.0f;
                        tc[0][1] = 0.0f;
                        tc[1][0] = 0.0f;
                        tc[1][1] = 0.0f;
                        tc[2][0] = 0.0f;
                        tc[2][1] = 0.0f;
                    } else {
                        assert(attrib.texcoords.size() >
                               size_t(2 * idx0.texcoord_index + 1));
                        assert(attrib.texcoords.size() >
                               size_t(2 * idx1.texcoord_index + 1));
                        assert(attrib.texcoords.size() >
                               size_t(2 * idx2.texcoord_index + 1));

                        // Flip Y coord. // FIXME: not sure if filp y is correct.
                        tc[0][0] = attrib.texcoords[2 * idx0.texcoord_index];
                        tc[0][1] = 1.0f - attrib.texcoords[2 * idx0.texcoord_index + 1];
                        tc[1][0] = attrib.texcoords[2 * idx1.texcoord_index];
                        tc[1][1] = 1.0f - attrib.texcoords[2 * idx1.texcoord_index + 1];
                        tc[2][0] = attrib.texcoords[2 * idx2.texcoord_index];
                        tc[2][1] = 1.0f - attrib.texcoords[2 * idx2.texcoord_index + 1];
                    }
                } else {
                    tc[0][0] = 0.0f;
                    tc[0][1] = 0.0f;
                    tc[1][0] = 0.0f;
                    tc[1][1] = 0.0f;
                    tc[2][0] = 0.0f;
                    tc[2][1] = 0.0f;
                }

                // vertex positions
                float v[3][3];
                for (int k = 0; k < 3; k++) {
                    int f0 = idx0.vertex_index;
                    int f1 = idx1.vertex_index;
                    int f2 = idx2.vertex_index;
                    assert(f0 >= 0);
                    assert(f1 >= 0);
                    assert(f2 >= 0);

                    v[0][k] = attrib.vertices[3 * f0 + k];
                    v[1][k] = attrib.vertices[3 * f1 + k];
                    v[2][k] = attrib.vertices[3 * f2 + k];
                    bmin[k] = std::min(v[0][k], bmin[k]);
                    bmin[k] = std::min(v[1][k], bmin[k]);
                    bmin[k] = std::min(v[2][k], bmin[k]);
                    bmax[k] = std::max(v[0][k], bmax[k]);
                    bmax[k] = std::max(v[1][k], bmax[k]);
                    bmax[k] = std::max(v[2][k], bmax[k]);
                }

                // normals
                float n[3][3];
                {
                    bool invalid_normal_index = false;
                    if (attrib.normals.size() > 0) {
                        int nf0 = idx0.normal_index;
                        int nf1 = idx1.normal_index;
                        int nf2 = idx2.normal_index;

                        if ((nf0 < 0) || (nf1 < 0) || (nf2 < 0)) {
                            // normal index is missing from this face.
                            invalid_normal_index = true;
                        } else {
                            for (int k = 0; k < 3; k++) {
                                assert(size_t(3 * nf0 + k) < attrib.normals.size());
                                assert(size_t(3 * nf1 + k) < attrib.normals.size());
                                assert(size_t(3 * nf2 + k) < attrib.normals.size());
                                n[0][k] = attrib.normals[3 * nf0 + k];
                                n[1][k] = attrib.normals[3 * nf1 + k];
                                n[2][k] = attrib.normals[3 * nf2 + k];
                            }
                        }
                    } else {
                        invalid_normal_index = true;
                    }

                    if (invalid_normal_index && !smoothVertexNormals.empty()) {
                        // Use smoothing normals
                        int f0 = idx0.vertex_index;
                        int f1 = idx1.vertex_index;
                        int f2 = idx2.vertex_index;

                        if (f0 >= 0 && f1 >= 0 && f2 >= 0) {
                            n[0][0] = smoothVertexNormals[f0].v[0];
                            n[0][1] = smoothVertexNormals[f0].v[1];
                            n[0][2] = smoothVertexNormals[f0].v[2];

                            n[1][0] = smoothVertexNormals[f1].v[0];
                            n[1][1] = smoothVertexNormals[f1].v[1];
                            n[1][2] = smoothVertexNormals[f1].v[2];

                            n[2][0] = smoothVertexNormals[f2].v[0];
                            n[2][1] = smoothVertexNormals[f2].v[1];
                            n[2][2] = smoothVertexNormals[f2].v[2];

                            invalid_normal_index = false;
                        }
                    }

                    if (invalid_normal_index) {
                        // compute geometric normal
                        CalcNormal(n[0], v[0], v[1], v[2]);
                        n[1][0] = n[0][0];
                        n[1][1] = n[0][1];
                        n[1][2] = n[0][2];
                        n[2][0] = n[0][0];
                        n[2][1] = n[0][1];
                        n[2][2] = n[0][2];
                    }
                }

                Triangle tri;
                tri.p[0] = glm::vec3(v[0][0], v[0][1], v[0][2]);
                tri.p[1] = glm::vec3(v[1][0], v[1][1], v[1][2]);
                tri.p[2] = glm::vec3(v[2][0], v[2][1], v[2][2]);
                tri.n[0] = glm::vec3(n[0][0], n[0][1], n[0][2]);
                tri.n[1] = glm::vec3(n[1][0], n[1][1], n[1][2]);
                tri.n[2] = glm::vec3(n[2][0], n[2][1], n[2][2]);
                tri.uv[0] = glm::vec2(tc[0][0], tc[0][1]);
                tri.uv[1] = glm::vec2(tc[1][0], tc[1][1]);
                tri.uv[2] = glm::vec2(tc[2][0], tc[2][1]);
                tri.material_index = current_material_id;
                // tri.material_index = outmaterials.size() - 1;


                outtriangles.push_back(tri);
            }
        }
    }

    cout << "num triangles = " << outtriangles.size() << endl;
    cout << "num materials = " << outmaterials.size() << endl;

    printf("bmin = %f, %f, %f\n", bmin[0], bmin[1], bmin[2]);
    printf("bmax = %f, %f, %f\n", bmax[0], bmax[1], bmax[2]);

    return true;
}

// =========================================================================================
// Default scene
// =========================================================================================

static bool load_default_scene(vector<Triangle>& triangles,
                               vector<Material>& materials) {
    // create random triangle soup
    triangles.clear();
    materials.clear();

    // create a single material
    // Material red;
    // red.setAmbient(glm::vec3(1.0f, 0.3f, 0.3f));
    // red.setDiffuse(glm::vec3(1.0f, 0.3f, 0.3f));
    // red.setSpecular(glm::vec3(0.5));
    // red.setRoughness(0.5);
    // red.setReflectivity(0.5);

    Material red;
    red.setAmbient(glm::vec3(0.01f, 0.03f, 0.03f));
    red.setDiffuse(glm::vec3(1.0f, 0.3f, 0.3f));
    red.setSpecular(glm::vec3(0.5));
    red.setRoughness(0.5);
    materials.push_back(red);

    Material yellow;
    yellow.setAmbient(glm::vec3(0.01f, 0.01f, 0.01f));
    yellow.setDiffuse(glm::vec3(1.0f, 1.0f, 0.0f));
    yellow.setSpecular(glm::vec3(1.0));
    yellow.setRoughness(0.0);
    yellow.setReflectivity(0.5f);
    materials.push_back(yellow);

    float xmin = -10.0f;
    float xmax = 10.0f;
    float ymin = 0.0f;
    float ymax = 10.0f;
    float z = 10.0f;
    const int N = 10;
    for (int i = 0; i < N - 1; ++i) {
        float x0 = xmin + float(i + 0) / float(N - 1) * (xmax - xmin);
        float x1 = xmin + float(i + 1) / float(N - 1) * (xmax - xmin);
        for (int j = 0; j < N - 1; ++j) {
            float y0 = ymin + float(j + 0) / float(N - 1) * (ymax - ymin);
            float y1 = ymin + float(j + 1) / float(N - 1) * (ymax - ymin);

            glm::vec3 p00 = glm::vec3(x0, y0, z);
            glm::vec3 p10 = glm::vec3(x1, y0, z);
            glm::vec3 p01 = glm::vec3(x0, y1, z);
            glm::vec3 p11 = glm::vec3(x1, y1, z);

            triangles.push_back(Triangle(p10, p00, p01, 0));
            triangles.push_back(Triangle(p11, p10, p01, 1));
        }
    }

    return true;
}

// =========================================================================================
// Class implementation
// =========================================================================================

bool ObjLoader::load(const string& fname,
                     vector<Triangle>& triangles,
                     vector<Material>& materials) {
    if (fname == "") {
        return load_default_scene(triangles, materials);
    } else {
        return load_obj_scene(fname, triangles, materials);
    }
}