#pragma once

#include <string>

#include "Object.h"
#include "Obj_loader.h"

class AABoxTreeNode {
   public:
    AABox bounding_box;

    // For leafs
    int leaf_triangle_count;
    int leaf_first_triangle;

    // For inner nodes
    int left_index;
    int right_index;

   public:
    static AABoxTreeNode make_leaf(AABox bbox, int l, int n) {
        //cout << "make_leaf " << l << ", " << n << endl;
        AABoxTreeNode result;
        result.bounding_box = bbox;
        result.leaf_triangle_count = n;
        result.leaf_first_triangle = l;
        result.left_index = 0;
        result.right_index = 0;
        return result;
    }

    static AABoxTreeNode make_inner(AABox bbox, int l, int r) {
        //cout << "make_inner " << l << ", " << r << endl;
        AABoxTreeNode result;
        result.bounding_box = bbox;
        result.leaf_triangle_count = 0;
        result.leaf_first_triangle = 0;
        result.left_index = l;
        result.right_index = r;
        return result;
    }
};


class TriangleSoup : public Object {
    vector<Material> materials;
    vector<Triangle> triangles;
    vector<AABoxTreeNode> aabox_tree_nodes;
    int aabox_tree_root_index;

   protected:  
    int aabox_tree_build_progress_leaf_count;
    int aabox_tree_build_progress_leaf_tricount;
    int aabox_tree_build_progress_last_message_tricount;

    // Called at the beginnging of tree building
    void aabox_tree_build_progress_init() {
       aabox_tree_build_progress_leaf_count = 0;
       aabox_tree_build_progress_leaf_tricount = 0;
       aabox_tree_build_progress_last_message_tricount = 0;
    }

    // Called after emitting each leaf, to print progress if at the end or we have emitted more than 5% of triangles since last message
    void aabox_tree_build_progress_out(int count_in_leaf) {
        aabox_tree_build_progress_leaf_count++;
        aabox_tree_build_progress_leaf_tricount += count_in_leaf;
        float fraction_done = float(aabox_tree_build_progress_leaf_tricount) / float(triangles.size());
        if ((aabox_tree_build_progress_leaf_tricount == triangles.size()) ||
            (aabox_tree_build_progress_leaf_tricount - aabox_tree_build_progress_last_message_tricount) > 0.05*triangles.size()) {
            aabox_tree_build_progress_last_message_tricount = aabox_tree_build_progress_leaf_tricount;
            cout << "AABOX TREE:" << aabox_tree_build_progress_leaf_tricount << "/" << triangles.size() << " triangles processed: " <<
            " Tree size = " << aabox_tree_build_progress_leaf_count << " leaves " << aabox_tree_nodes.size() - aabox_tree_build_progress_leaf_count << " inner" <<
            " (" << float(aabox_tree_build_progress_leaf_tricount)/float(aabox_tree_build_progress_leaf_count) << " trianges/leaf)" << std::endl;
        }
    }

   public:
    TriangleSoup() : aabox_tree_root_index(-1) {}

    void clear() {
        materials.clear();
        triangles.clear();
        aabox_tree_nodes.clear();
        aabox_tree_root_index = -1;
    }

    bool loadObj(const string &fname) {
        clear();
        ObjLoader loader;
        bool result = loader.load(fname, triangles, materials);
        if (result) {
            // start time
            clock_t start = clock();
            buildAABoxTree();
            // end time
            clock_t end = clock();
            // time elapsed
            double time_elapsed = double(end - start) / CLOCKS_PER_SEC;
            cout << "buildAABoxTree time: " << time_elapsed << endl;
        }
        return result;
    }

    void buildAABoxTree() {
        aabox_tree_nodes.clear();
        aabox_tree_root_index = -1;
        if (!triangles.empty()) {
            cout << "---- buildAABoxTree" << endl;
            aabox_tree_build_progress_init();
            aabox_tree_root_index = buildAABoxTreeNode(0, triangles.size());
            cout << "---- buildAABoxTree done " << aabox_tree_root_index << ", " << aabox_tree_nodes.size() << endl;
        }
    }

    int buildAABoxTreeNode(int mintri, int tricount) {
        // Compute bbox of triangle range
        AABox box;
        AABox centroid_box;
        vector<glm::vec3> centroids(tricount);

        for (int i = 0; i < tricount; ++i) {
            // triangles[mintri+i]
            const Triangle &t = triangles[mintri + i];
            box.extend(t.p[0]);
            box.extend(t.p[1]);
            box.extend(t.p[2]);
            centroids[i] = (t.p[0] + t.p[1] + t.p[2]) / 3.0f;
            centroid_box.extend(centroids[i]);
            // if (tricount < 10) {
            //     cout << "centroid " << i << ": " << centroids[i][0] << ", " << centroids[i][1] << ", " << centroids[i][2] << endl;
            // }
        }
        // expand boxes to avoid degenerate cases in ray intersection
        box.expand();
        if (tricount < 8) {
            aabox_tree_nodes.push_back(AABoxTreeNode::make_leaf(box, mintri, tricount));
            aabox_tree_build_progress_out(tricount);
            return aabox_tree_nodes.size() - 1;
        } else {
            glm::vec3 d = centroid_box.diagonal();
            int split_index = 0;
            if (d[1] > d[split_index]) split_index = 1;
            if (d[2] > d[split_index]) split_index = 2;

            float mid = centroid_box.center()[split_index];
            // cout << "split_index: " << split_index << ", mid: " << mid << endl;

            int first_right = 0;  // index of the first element in the right partition
            while ((first_right < tricount) && (centroids[first_right][split_index] < mid)) {
                ++first_right;
            }
            // cout << "first_right: " << first_right << endl;
            if (first_right == tricount) {
                // Something bad happened - we have no spatial split
                // Just partition triangles in current order
                first_right = tricount / 2;
            } else {
                for (int i = first_right + 1; i < tricount; ++i) {
                    // Move all elements that are on the left side of the split plane to the left partition
                    if (centroids[i][split_index] < mid) {
                        swap(centroids[i], centroids[first_right]);
                        swap(triangles[mintri + i], triangles[mintri + first_right]);
                        ++first_right;
                    }
                }
                if (first_right == tricount || first_right == 0) {
                    // Something bad happened - we have no spatial split
                    // Just partition triangles in current order
                    for (int i = 0; i < tricount; ++i) {
                        if (i == first_right) cout << "___" << endl;
                        cout << "centroid " << i << ": " << centroids[i][0] << ", " << centroids[i][1] << ", " << centroids[i][2] << endl;
                    }
                    first_right = tricount / 2;
                }
            }

            int left_child = buildAABoxTreeNode(mintri, first_right);
            int right_child = buildAABoxTreeNode(mintri + first_right, tricount - first_right);
            aabox_tree_nodes.push_back(AABoxTreeNode::make_inner(box, left_child, right_child));

            //cout << "EXIT buildAABoxTreeNode(" << mintri << ", " << tricount << std::endl;
            return aabox_tree_nodes.size() - 1;
        }
    }

    Hit intersect(Ray ray) {
        Hit hit;

        Ray localRay = toLocalRay(ray);
        
        pair<int, float> intersection = intersectTree(localRay, numeric_limits<float>::max(), aabox_tree_root_index);
        if (intersection.first >= 0) {
            const Triangle &tri = triangles[intersection.first];
            float t, u, v;
            hit.hit = tri.rayIntersect(localRay, t, u, v);
            if (hit.hit) {
                hit.normal = tri.getGeometricNormal();
                hit.normal_shading = tri.getShadingNormal(u, v);
                hit.uv = tri.getTextureCoords(u, v);
                hit.intersection = localRay.origin + t * localRay.direction;
                hit.distance = t;
                hit.material = &materials[tri.material_index];
            }
        }
        return toGlobalHit(hit, ray);
    }

    pair<int, float> intersectTree(Ray ray, float tmax, int node_index) const {
        pair<int, float> result = make_pair(-1, tmax);

        bool is_intersecting = aabox_tree_nodes[node_index].bounding_box.is_intersecting(ray, tmax);
        if (is_intersecting) {
            const int n = aabox_tree_nodes[node_index].leaf_triangle_count;
            if (n == 0) {
                // Inner node
                pair<int, float> result_left = intersectTree(ray, tmax, aabox_tree_nodes[node_index].left_index);
                pair<int, float> result_right = intersectTree(ray, min(tmax, result.second), aabox_tree_nodes[node_index].right_index);
                result = (result_left.second < result_right.second) ? result_left : result_right;
            } else {
                // leaf node
                const int i0 = aabox_tree_nodes[node_index].leaf_first_triangle;
                for (int i = i0; i < i0 + n; ++i) {
                    float t, u, v;
                    bool is_intersecting = triangles[i].rayIntersect(ray, t, u, v);
                    if (is_intersecting && t < result.second) {
                        result.first = i;
                        result.second = t;
                    }
                }
            }
        }

        return result;
    }
};