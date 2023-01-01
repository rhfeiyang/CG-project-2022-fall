//
// Created by Renhui on 2022/12/31.
//

#ifndef CG_PROJECT_2022_FALL_CONSTRUCT_H
#define CG_PROJECT_2022_FALL_CONSTRUCT_H

#include "common.h"
#include <vector>

class KdTreeNode {
public:
//    explicit KdTreeNode(std::vector<std::shared_ptr<Cell>> cells, openvdb::CoordBBox bbox, int depth);
    explicit KdTreeNode(openvdb::CoordBBox bbox,float max_dx, int depth);
    ~KdTreeNode();
//    bool intersect(Interaction &interaction, const Ray &ray) const;
    [[nodiscard]] bool isLeaf() const { return !leftChild && !rightChild; }

private:
    KdTreeNode *leftChild, *rightChild;
//    openvdb::CoordBBox bbox;
//    std::vector<std::shared_ptr<Cell>> cells;
    int partition_axis;
    float partition_poi;
    std::vector<int> contribute_grids;
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









#endif //CG_PROJECT_2022_FALL_CONSTRUCT_H
