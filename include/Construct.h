//
// Created by Renhui on 2022/12/31.
//

#ifndef CG_PROJECT_2022_FALL_CONSTRUCT_H
#define CG_PROJECT_2022_FALL_CONSTRUCT_H

#include "common.h"
//#include "Brick.h"

struct Pos{
    double pos;
    int grid_idx;
    bool isMin;
    bool operator< (const Pos& other) const{
        return pos<other.pos;
    }
};

class Kdtree;
class KdTreeNode {
public:
    ///Is depth necessary?
    KdTreeNode(Grids_data& gridsData,std::vector<int> contribute_grids,wBBox bbox, int depth);

    ~KdTreeNode();
//    bool intersect(Interaction &interaction, const Ray &ray) const;
    [[nodiscard]] bool isLeaf() const { return !leftChild && !rightChild; }

    static std::vector<wBBox> splitWbbox(const wBBox& bbox, int axis, double pos);

    friend Kdtree;

private:
    int partition_axis=-1;
    double partition_pos;
    std::vector<int> contribute_grids;
    KdTreeNode *leftChild, *rightChild;
//    openvdb::CoordBBox bbox;
//    std::vector<std::shared_ptr<Cell>> cells;

//    std::shared_ptr<Brick> brick;
};




class Kdtree{
public:
    ///Notice: grids may have overlap!
    ///For a point in the world, find its contribute grids, then do the interpolation....
    explicit Kdtree(Grids_data& gridsData);

    [[nodiscard]] std::vector<int> grid_contribute(const Vec3f &xyz)const;
//    float Get_value(const Coord& ijk) const;
//    void setValue(const Coord& xyz, double value);
//    bool isValueOn(const Coord& xyz) const;

//    [[nodiscard]] Vec3f indexToWorld(const Vec3f& xyz) const;
//    [[nodiscard]] Vec3f indexToWorld(const Coord& ijk) const;
//    [[nodiscard]] Vec3f worldToIndex(const Vec3f& xyz) const;

//    float indexInterpolation(const Vec3f& xyz);
//    float worldInterpolation(const Vec3f& xyz);
private:
    KdTreeNode* root;
};

#endif //CG_PROJECT_2022_FALL_CONSTRUCT_H
