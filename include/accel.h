#ifndef ACCEL_H_
#define ACCEL_H_

#include "common.h"
#include "ray.h"

struct Triangle {
    Vec3f v1, v2, v3;
    Vec3f n1, n2, n3;
    wBBox aabb;
    Vec3f center;

    Triangle(Vec3f v1, Vec3f v2, Vec3f v3, Vec3f n1, Vec3f n2, Vec3f n3) : v1(v1), v2(v2), v3(v3), n1(n1), n2(n2),
                                                                           n3(n3) {
//		aabb = wBBox (v1, v2, v3);
        aabb.expand(v1);
        aabb.expand(v2);
        aabb.expand(v3);
        center = aabb.getCenter();
    }

    Triangle() {}
};

struct Triangle_index {
    Vec3i v_idx, n_idx;
    wBBox aabb;

    explicit Triangle_index(Vec3i v, Vec3i n) : v_idx(v), n_idx(n) {}

    Triangle_index() {}

    uint32_t code;
    Vec3f center;
};

struct BVHNode {
    BVHNode *left;
    BVHNode *right;
    // bounding box of current node.
    wBBox aabb;
    // index of triangles in current BVH leaf node.
    std::vector<Triangle_index> triangles;
};


struct LinearBVHNode {
    wBBox aabb;
    int right;
    int left;
    std::vector<Triangle_index> triangles;
};

// You may need to add your code for BVH construction here.

#endif //ACCEL_H_