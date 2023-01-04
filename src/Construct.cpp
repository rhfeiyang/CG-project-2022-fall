//
// Created by Renhui on 2022/12/31.
//

#include "Construct.h"

KdTreeNode::KdTreeNode(Grids_data& gridsData,wBBox bbox, int depth) {
    ///For construct: find the longest axis by bbox, find a pos to create two children
    ///For leaf: create a brick, store cells
    ///Can depth has some effect?
    partition_axis=bbox.maxExtent();
//    partition_poi = int((bbox.max()[partition_axis]-bbox.min()[partition_axis]) / max_dx / 2 + 0.5) * max_dx;


//    leftChild=new KdTreeNode(bbox.par)

}

//Vec3f Kdtree::worldToIndex(const Vec3f &xyz) const {
//    return xyz/dx;
//}
//
//Vec3f Kdtree::indexToWorld(const Vec3f &xyz) const {
//    return xyz*dx;
//}
//
//Vec3f Kdtree::indexToWorld(const Coord &ijk) const {
//    return ijk.asVec3d()*dx;
//}
