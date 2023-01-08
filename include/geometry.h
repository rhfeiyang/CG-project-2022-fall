#ifndef GEOMETRY_H_
#define GEOMETRY_H_

#include "ray.h"
#include "interaction.h"
#include "accel.h"


class TriangleMesh {
 public:
  TriangleMesh() = default;
  TriangleMesh(std::vector<Vec3f> vertices,
               std::vector<Vec3f> normals,
               std::vector<int> v_index,
               std::vector<int> n_index);
  bool intersect(Ray &ray, Interaction &interaction) const;
//  void setMaterial(std::shared_ptr<BSDF> &new_bsdf);
    void setColor(Vec3f&);
  void buildBVH();
//  BVHNode* build_BVH_recur(std::vector<Triangle>& meshes,int start, int end);
//  int find_axis_ran(std::vector<Triangle>& meshes, int start, int end, float x_ran[2],float y_ran[2], float z_ran[2]);
//  int SAH_build(std::vector<Triangle> &meshes, int start, int end,int& min_index);

    ///from shy
    void bvhHit_linear(Interaction& interaction,
                       Ray& ray) const;
    // Expands a 10-bit integer into 30 bits
    // by inserting 2 zeros after each bit.
    uint32_t expand_bits(uint32_t v)
    {
        v = (v * 0x00010001u) & 0xFF0000FFu;
        v = (v * 0x00000101u) & 0x0F00F00Fu;
        v = (v * 0x00000011u) & 0xC30C30C3u;
        v = (v * 0x00000005u) & 0x49249249u;
        return v;
    }

    // Calculates a 30-bit Morton code for the
    // given 3D point located within the unit cube [0,1].
    uint32_t morton3D(float x, float y, float z)
    {
        x = std::min(std::max(x * 1024.0f, 0.0f), 1023.0f);
        y = std::min(std::max(y * 1024.0f, 0.0f), 1023.0f);
        z = std::min(std::max(z * 1024.0f, 0.0f), 1023.0f);
        uint32_t xx = expand_bits((uint32_t)x);
        uint32_t yy = expand_bits((uint32_t)y);
        uint32_t zz = expand_bits((uint32_t)z);
        return xx | (yy << 1) | (zz << 2);
    }

    uint32_t morton3D_invert(uint32_t x) {
        x = x & 0x49249249;
        x = (x | (x >> 2)) & 0xc30c30c3;
        x = (x | (x >> 4)) & 0x0f00f00f;
        x = (x | (x >> 8)) & 0xff0000ff;
        x = (x | (x >> 16)) & 0x0000ffff;
        return x;
    }

    int clz(unsigned int x) {
        if (x == 0) return 32;
        int n = 1;
        if ((x >> 16) == 0) { n = n + 16; x = x << 16; }
        if ((x >> 24) == 0) { n = n + 8; x = x << 8; }
        if ((x >> 28) == 0) { n = n + 4; x = x << 4; }
        if ((x >> 30) == 0) { n = n + 2; x = x << 2; }
        n = n - (x >> 31);
        return n;
    }
    //z ordering
    void MortonCodeSort(std::vector<Triangle_index>& triangles) {

        wBBox box;
        for (auto& tri : triangles) {
//            tri.aabb = wBBox(vertices[tri.v_idx[0]], vertices[tri.v_idx[1]], vertices[tri.v_idx[2]]);
            tri.aabb.expand(vertices[tri.v_idx[0]]);tri.aabb.expand(vertices[tri.v_idx[1]]);tri.aabb.expand(vertices[tri.v_idx[2]]);
//            box = wBBox(box, tri.aabb);
            box.expand(tri.aabb);
        }
        Vec3f dis = box.max() - box.min();
        dis = Vec3f(1.0f / dis.x(), 1.0f / dis.y(), 1.0f / dis.z());
        for (auto& tri : triangles) {
            Vec3f center = tri.aabb.getCenter();
            center = (center - box.min())*dis;
            tri.code = morton3D(center.x(), center.y(), center.z());
        }
        std::stable_sort(triangles.begin(), triangles.end(), [](auto& A, auto& B)->bool {
            return A.code < B.code;
        });


    }

    int findSplit(std::vector<Triangle_index>& triangles, int l, int r)
    {
        uint32_t firstCode = triangles[l].code;
        uint32_t lastCode = triangles[r].code;

        if (firstCode == lastCode)
            return (l + r) >> 1;

        int commonPrefix = clz(firstCode ^ lastCode);

        int split = l;
        int step = r - l;

        do
        {
            step = (step + 1) >> 1; // exponential decrease
            int newSplit = split + step; // proposed new position

            if (newSplit < r)
            {
                uint32_t splitCode = triangles[newSplit].code;
                int splitPrefix = clz(firstCode ^ splitCode);
                if (splitPrefix > commonPrefix)
                    split = newSplit; // accept proposal
            }
        } while (step > 1);

        return split;
    }
    void generateWithMortonCode(BVHNode*& node, std::vector<Triangle_index>& triangles, int l, int r) {
        if (l > r) return;
        node = new BVHNode();
        if ((r - l + 1) <= 5) {
            node->left = nullptr;
            node->right = nullptr;
            node->triangles.assign(triangles.begin() + l, triangles.begin() + r + 1);
            for (auto& tri : node->triangles) {
                node->aabb.expand(tri.aabb);
            }
            return;
        }
        int Split = findSplit(triangles, l, r);

        generateWithMortonCode(node->left, triangles, l, Split);
        generateWithMortonCode(node->right, triangles, Split + 1, r);
//        node->aabb = wBBox(node->left->aabb, node->right->aabb);
        node->aabb.expand(node->left->aabb);node->aabb.expand(node->right->aabb);

        return;
    }

    void generateHierarchy(BVHNode*& node, std::vector<Triangle_index>& triangles, int l, int r)
    {
        if (l > r) return;
        node = new BVHNode();
        if ((r - l + 1) <= 5) {
            node->left = nullptr;
            node->right = nullptr;
            node->triangles.assign(triangles.begin() + l, triangles.begin() + r + 1);
            for (auto& tri : node->triangles) {
//                node->aabb = wBBox(tri.aabb, node->aabb);
                node->aabb.expand(tri.aabb);
            }
            return;
        }


        //SAH
        float Cost = INT_MAX;
        int Axis = 0;
        int Split = (l + r) >> 1;
        std::vector<wBBox> aabb_l(r - l + 1), aabb_r(r - l + 1);
        for (int axis = 0; axis < 3; ++axis) {
            std::sort(triangles.begin() + l, triangles.begin() + r + 1, [&axis](const Triangle_index& A, const Triangle_index& B)->bool {
                return A.center[axis] < B.center[axis];
            });

            aabb_l[0] = triangles[l].aabb;
            aabb_r[r - l] = triangles[r].aabb;
            for (int i = l + 1; i <= r; ++i) {
//                aabb_l[i - l] = wBBox(aabb_l[i - l - 1], triangles[i].aabb);
                aabb_l[i - l].expand(aabb_l[i - l - 1]);aabb_l[i - l].expand(triangles[i].aabb);
            }
            for (int i = r - 1; i >= l; --i) {
//                aabb_r[i - l] = wBBox(aabb_r[i - l + 1], triangles[i].aabb);
                aabb_r[i - l].expand(aabb_r[i - l + 1]);aabb_r[i - l].expand(triangles[i].aabb);
            }
            float cost = INT_MAX;
            int split = l;
            auto Surface = [](const wBBox& node)->float {
                Vec3f dim=node.max()-node.min();
                float lenx = dim[0];
                float leny = dim[1];
                float lenz = dim[2];
                return 2.0f * ((lenx * leny) + (lenx * lenz) + (leny * lenz));
            };
            for (int i = l; i <= r - 1; ++i) {
                //[i,l]
                float surf_left = Surface(aabb_l[i - l]);
                float left_cost = surf_left * (i - l + 1);
                //[i+1,r]
                float surf_right = Surface(aabb_r[i + 1 - l]);
                float right_cost = surf_right * (r - i);
                float cost_ = right_cost + left_cost;
                if (cost_ < cost) {
                    cost = cost_;
                    split = i;
                }
            }

            if (cost < Cost) {
                Cost = cost;
                Split = split;
                Axis = axis;
            }
        }

        //for (int i = l; i <= r; ++i) {
        //	auto& tri = triangles[i];
        //	node->aabb = wBBox(tri.aabb, node->aabb);
        //}

        //float lenx = node->aabb.getDist(0);
        //float leny = node->aabb.getDist(1);
        //float lenz = node->aabb.getDist(2);

        // //x
        //if (lenx >= leny && lenx >= lenz) {
        //	Axis = 0;
        //}
        //// y
        //if (leny >= lenx && leny >= lenz) {
        //	Axis = 1;
        //}
        // //z
        //if (lenz >= lenx && lenz >= leny) {
        //	Axis = 2;
        //}
        std::sort(triangles.begin() + l, triangles.begin() + r + 1, [&Axis](const Triangle_index& A, const Triangle_index& B)->bool {
            return A.center[Axis] < B.center[Axis];
        });


        generateHierarchy(node->left, triangles, l, Split);
        generateHierarchy(node->right, triangles, Split + 1, r);
//        node->aabb = wBBox(node->left->aabb, node->right->aabb);
        node->aabb.expand(node->left->aabb);node->aabb.expand(node->right->aabb);
        return;
    }


    int BVHCompress(BVHNode* node, int* offset) {
        LinearBVHNode& linear_node = linearBVH[*offset];
        linear_node.aabb = node->aabb;
        int new_offset = (*offset)++;
        if (node->triangles.size() > 0) {
            linear_node.triangles = node->triangles;
            linear_node.right = -1;
            linear_node.left = -1;
        }
        else {
            linear_node.left=BVHCompress(node->left, offset);
            linear_node.right = BVHCompress(node->right, offset);
        }

        return new_offset;
    }


 private:
  bool intersectOneTriangle(Ray &ray, Interaction &interaction, const Vec3i& v_idx, const Vec3i& n_idx) const;
  void bvhHit(BVHNode *p, Interaction &interaction,
              Ray &ray) const;
//  std::shared_ptr<BSDF> bsdf;
  BVHNode* bvh;
    Vec3f color;
  std::vector<Vec3f> vertices;
  std::vector<Vec3f> normals;
  std::vector<int> v_indices;
  std::vector<int> n_indices;
  std::vector<LinearBVHNode> linearBVH;
};

#endif // GEOMETRY_H_
