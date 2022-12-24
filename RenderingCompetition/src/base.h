#pragma once

#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtx/vector_angle.hpp"

using namespace std;

/**
 Class representing a single ray.
 */
class Ray {
   public:
    glm::vec3 origin;     ///< Origin of the ray
    glm::vec3 direction;  ///< Direction of the ray
                          /**
                           Constructor of the ray
                           @param origin Origin of the ray
                           @param direction Direction of the ray
                           */
    Ray(glm::vec3 origin, glm::vec3 direction) : origin(origin), direction(direction) {
    }
};

/**
 * Generates a 4x4 transformation matrix given Translation (t*), Rotation (r*), Scaling (s*) parameters.
 * @return
 */
inline glm::mat4 genTRMat(glm::vec3 t,  // Translation
                          glm::vec3 r,  // Rotation
                          glm::vec3 s   // Scaling
) {
    glm::mat4 trans = glm::translate(glm::mat4(1.0f), t);
    glm::mat4 rotat = glm::rotate(trans, glm::radians(r.x), glm::vec3(1, 0, 0));
    rotat = glm::rotate(rotat, glm::radians(r.y), glm::vec3(0, 1, 0));
    rotat = glm::rotate(rotat, glm::radians(r.z), glm::vec3(0, 0, 1));
    glm::mat4 scale = glm::scale(rotat, s);
    return scale;
}

class Material;

/**
 Structure representing the even of hitting an object
 */
struct Hit {
    bool hit;                  ///< Boolean indicating whether there was or there was no intersection with an object
    glm::vec3 normal;          ///< Geometric Normal vector of the intersected object at the intersection point
    glm::vec3 normal_shading;  //<  Shading Normal ( = transformed geometric normal) vector of the intersected object at the intersection point
    glm::vec3 intersection;    ///< Point of Intersection
    float distance;            ///< Distance from the origin of the ray to the intersection point
    const Material *material;  ///< A pointer to the intersected object
    glm::vec2 uv;              ///< Coordinates for computing the texture (texture coordinates)

    Hit() {  // Default constructor
        hit = false;
        material = nullptr;
    }
};

// ------------------------------------------------------------------------------------------------
//
// Axis-Aligned Bounding Box
//
// ------------------------------------------------------------------------------------------------

class AABox {
   protected:
    glm::vec3 lb;
    glm::vec3 ub;

   public:
    AABox() {
        make_empty();
    }

    glm::vec3 lower_bound() const { return lb; }
    glm::vec3 upper_bound() const { return ub; }
    glm::vec3 diagonal() const { return ub - lb; }
    glm::vec3 center() const { return 0.5f * (ub + lb); }

    void make_empty() {
        const float HUGE = numeric_limits<float>::max();
        lb = glm::vec3(HUGE, HUGE, HUGE);
        ub = glm::vec3(-HUGE, -HUGE, -HUGE);
    }

    bool is_empty() const {
        return lb[0] > ub[0];
    }

    void extend(glm::vec3 p) {
        lb[0] = min(lb[0], p[0]);
        lb[1] = min(lb[1], p[1]);
        lb[2] = min(lb[2], p[2]);
        ub[0] = max(ub[0], p[0]);
        ub[1] = max(ub[1], p[1]);
        ub[2] = max(ub[2], p[2]);
    }

    void extend(const AABox &b) {
        extend(b.lb);
        extend(b.ub);
    }

    void expand(const float eps = 1e-6f) {
        if (!is_empty()) {
            lb -= glm::vec3(eps, eps, eps);
            ub += glm::vec3(eps, eps, eps);
        }
    }

    /**
     * Clip a linear component with this axis aligned box.
     * on entry, tmin and tmax determine the extent of
     * the linear component. On exit, off is true if
     * the linear component is outside of the box, and
     * tmin, tmax contain the portion of the component
     * inside the box.
     */
    void clip(const glm::vec3 &org,
              const glm::vec3 &dir,
              bool &off,
              float &tmin,
              float &tmax) const {
        off = (tmin > tmax);
        for (size_t i = 0; (i < 3) && (!off); ++i) {
            const float min_i = lb[i];
            const float max_i = ub[i];
            const float org_i = org[i];
            const float dir_i = dir[i];

            if (std::abs(dir_i) < 1e-10f) {
                off = (org_i < min_i || org_i > max_i);
            } else {
                const float inv_dir_i = 1.0f/(dir_i);
                const float t1 = (min_i - org_i) * inv_dir_i;
                const float t2 = (max_i - org_i) * inv_dir_i;
                if (t1 < t2) {
                    if (t1 > tmin) tmin = t1;
                    if (t2 < tmax) tmax = t2;
                } else {
                    if (t2 > tmin) tmin = t2;
                    if (t1 < tmax) tmax = t1;
                }
                off = (tmin > tmax);
            }
        }
    }

    /**
     * ray - volume intersection. The routine returns the
     * pair <false,???> if there is no intersection, and the pair
     * <true, t> if an intersection occurs. t is the value of the
     * smallest ray parameter for an intersection point.
     */
    inline std::pair<bool, float> bounds_intersection(const Ray &r, float t_min, float t_max) const {
        bool off = false;
        clip(r.origin, r.direction, off, t_min, t_max);
        return std::pair<bool, float>(!off, t_min);
    }

    bool is_intersecting(Ray ray, float tmax) const {
        return bounds_intersection(ray, 0.0, tmax).first;
    }
};

// ------------------------------------------------------------------------------------------------
//
// Base Triangle
//
// ------------------------------------------------------------------------------------------------

class Triangle {
   public:
    glm::vec3 p[3];
    glm::vec3 n[3];
    glm::vec2 uv[3];
    int material_index;

    Triangle() {
        material_index = -1;
    }

    Triangle(const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec3 &p2, int mat_idx = -1) {
        material_index = mat_idx;
        p[0] = p0;
        p[1] = p1;
        p[2] = p2;
        glm::vec3 gn = getGeometricNormal();
        n[0] = gn;
        n[1] = gn;
        n[2] = gn;
        uv[0] = glm::vec2(0.0f, 0.0f);
        uv[1] = glm::vec2(1.0f, 0.0f);
        uv[2] = glm::vec2(0.0f, 1.0f);
    }

    glm::vec3 getGeometricNormal() const {
        return glm::normalize(glm::cross(p[1] - p[0], p[2] - p[0]));
    }

    glm::vec3 getShadingNormal(float u, float v) const {
        return glm::normalize(n[0] + u * (n[1] - n[0]) + v * (n[2] - n[0]));
    }

    glm::vec2 getTextureCoords(float u, float v) const {
        return uv[0] + u * (uv[1] - uv[0]) + v * (uv[2] - uv[0]);
    }

    // Moller-Trumbore intersect ...
    bool rayIntersect(const Ray &ray,
                      float &t, float &u, float &v,
                      bool cull_backfaces = false) const {
        // This constant defines the tolerance with which we compute intersection
        // In practice, it considers rays nearly parallel to the triangle plane not intersecting
        // (otherwise, error is too big to trust the intersection)

        const float kEpsilon = 1e-6f;

        glm::vec3 p0p1 = p[1] - p[0];
        glm::vec3 p0p2 = p[2] - p[0];
        glm::vec3 pvec = glm::cross(ray.direction, p0p2);
        float det = glm::dot(p0p1, pvec);

        if (cull_backfaces) {
            // if the determinant is negative the triangle is 'back facing'
            // if the determinant is close to 0, the ray misses the triangle
            if (det < kEpsilon) return false;
        } else {
            // ray and triangle are parallel if det is close to 0
            if (abs(det) < kEpsilon) return false;
        }
        float invDet = 1.0f / det;

        // Barycentric coordinates
        // If not in range, the ray does not intersect the triangle
        glm::vec3 tvec = ray.origin - p[0];
        u = glm::dot(tvec, pvec) * invDet;
        if (u < 0.0f || u > 1.0f) return false;

        glm::vec3 qvec = glm::cross(tvec, p0p1);
        v = glm::dot(ray.direction, qvec) * invDet;
        if (v < 0.0f || u + v > 1.0f) return false;

        // Distance from ray origin
        t = glm::dot(p0p2, qvec) * invDet;

        // No intersection if in wrong direction
        return t > 0.0f;
    }
};
