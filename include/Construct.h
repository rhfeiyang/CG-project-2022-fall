//
// Created by Renhui on 2022/12/31.
//

#ifndef CG_PROJECT_2022_FALL_CONSTRUCT_H
#define CG_PROJECT_2022_FALL_CONSTRUCT_H

#include "common.h"
#include "Brick.h"
class KdTreeNode {
public:
    ///Is depth necessary?
    KdTreeNode(Grids_data& gridsData,wBBox bbox, int depth);
    ~KdTreeNode();
//    bool intersect(Interaction &interaction, const Ray &ray) const;
    [[nodiscard]] bool isLeaf() const { return !leftChild && !rightChild; }

private:
//    openvdb::CoordBBox bbox;
//    std::vector<std::shared_ptr<Cell>> cells;
    KdTreeNode *leftChild, *rightChild;
    int partition_axis;
    double partition_poi;
    std::vector<int> contribute_grids;

//    std::shared_ptr<Brick> brick;
};

KdTreeNode::~KdTreeNode() {
    if (leftChild) {
        delete leftChild;
        leftChild = nullptr;
    }
    if (rightChild) {
        delete rightChild;
        rightChild = nullptr;
    }
}


class Kdtree{
public:
    ///Notice: grids may have overlap!
    ///For a point in the world, find its contribute grids, then do the interpolation....
    Kdtree(Grids_data& gridsData);

    std::vector<int> grid_contribut0e(const Vec3f &xyz)const;
//    float Get_value(const Coord& ijk) const;
//    void setValue(const Coord& xyz, double value);
//    bool isValueOn(const Coord& xyz) const;

//    [[nodiscard]] Vec3f indexToWorld(const Vec3f& xyz) const;
//    [[nodiscard]] Vec3f indexToWorld(const Coord& ijk) const;
//    [[nodiscard]] Vec3f worldToIndex(const Vec3f& xyz) const;

//    float indexInterpolation(const Vec3f& xyz);
//    float worldInterpolation(const Vec3f& xyz);
private:
    KdTreeNode root;
    float dx;
};

#endif //CG_PROJECT_2022_FALL_CONSTRUCT_H
