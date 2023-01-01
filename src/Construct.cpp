//
// Created by Renhui on 2022/12/31.
//

#include "Construct.h"

KdTreeNode::KdTreeNode(openvdb::CoordBBox bbox,float max_dx, int depth) {


    partition_axis=bbox.maxExtent();
    partition_poi = int((bbox.max()[partition_axis]-bbox.min()[partition_axis]) / max_dx / 2 + 0.5) * max_dx;


//    leftChild=new KdTreeNode(bbox.par)

}