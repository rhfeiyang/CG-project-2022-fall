//
// Created by Renhui on 2022/12/31.
//

#ifndef CG_PROJECT_2022_FALL_CONSTRUCT_H
#define CG_PROJECT_2022_FALL_CONSTRUCT_H

#include "common.h"

struct Pos {
    double pos;
//    int grid_idx;
    uint32_t grid_idx_bm;

//    bool isMin;
    bool operator<(const Pos &other) const {
        return pos < other.pos;
    }

    bool operator>(const Pos &other) const {
        return !(*this < other);
    }
};

class Kdtree;

class KdTreeNode {
public:
    ///Is depth necessary?
    KdTreeNode(Grids_data &gridsData, uint32_t bm, wBBox bbox, int depth);

    ~KdTreeNode();

//    bool intersect(Interaction &interaction, const Ray &ray) const;
    [[nodiscard]] bool isLeaf() const { return leftChild == nullptr && rightChild == nullptr; }

    static std::vector<wBBox> splitWbbox(const wBBox &bbox, int axis, double pos);

//    static int bm_grid_count(uint32_t i);

    friend Kdtree;

private:
    int partition_axis;
    double partition_pos;

    uint32_t bitmap;
    KdTreeNode *leftChild, *rightChild;
};

class Kdtree {
public:
    ///Notice: grids may have overlap!
    ///For a point in the world, find its contribute grids, then do the interpolation....
    explicit Kdtree(Grids_data &gridsData);

    [[nodiscard]] uint32_t grid_contribute(const Vec3f &xyz) const;

private:
    KdTreeNode *root;
};

#endif //CG_PROJECT_2022_FALL_CONSTRUCT_H
