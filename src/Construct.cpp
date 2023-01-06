//
// Created by Renhui on 2022/12/31.
//

#include "Construct.h"

std::vector<wBBox> KdTreeNode::splitWbbox(const wBBox &bbox, int axis, double pos) {
    std::vector<wBBox> splitbbox;
    Vec3d lp=bbox.max();
    lp[axis]=pos;
    Vec3d rp=bbox.min();
    rp[axis]=pos;
    splitbbox.emplace_back(bbox.min(),lp);
    splitbbox.emplace_back(rp,bbox.max());
    return splitbbox;
}

KdTreeNode::KdTreeNode(Grids_data& gridsData,std::vector<int> contri_grids,wBBox bbox, int depth):contribute_grids(std::move(contri_grids)) {
    ///For construct: find the longest axis by bbox, find a pos to create two children
    ///For leaf: create a brick, store cells
    ///Can depth has some effect?
    if(depth>50) return;
    partition_axis=bbox.maxExtent();

    float pos_range[2]={static_cast<float>(bbox.min()[partition_axis]),static_cast<float>(bbox.max()[partition_axis])};
    std::vector<Pos> pos;
    for(auto& i : contribute_grids){
        auto pos1=gridsData.wbboxes[i].min()[partition_axis];
        auto pos2=gridsData.wbboxes[i].max()[partition_axis];
        if(pos1>pos_range[0]+EPS && pos1<pos_range[1]-EPS) pos.push_back({pos1,i,true});
        if(pos2>pos_range[0]+EPS && pos2<pos_range[1]-EPS) pos.push_back({pos2,i,false});
    }

    if(!pos.empty()){
        std::sort(pos.begin(), pos.end());
        auto partition=pos[int((pos.size() - 1) / 2)];
        partition_pos=partition.pos;
        auto splitbbox= splitWbbox(bbox,partition_axis,partition_pos);
        auto contribute= contribute_grids;
        std::erase(contribute,partition.grid_idx);
        if(partition.isMin){
            leftChild=new KdTreeNode{gridsData,contribute,splitbbox[0],depth+1};
            rightChild=new KdTreeNode{gridsData,contribute_grids,splitbbox[1],depth+1};
        }
        else{
            leftChild=new KdTreeNode{gridsData,contribute_grids,splitbbox[0],depth+1};
            rightChild=new KdTreeNode{gridsData,contribute,splitbbox[1],depth+1};
        }
    }
}

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

Kdtree::Kdtree(Grids_data &gridsData) {
    std::vector<int> contribute;
    for(int i=0;i<gridsData.grids.size();i++){
        contribute.emplace_back(i);
    }
    root=new KdTreeNode(gridsData,contribute,gridsData.whole_wbbox,0);
}

std::vector<int> Kdtree::grid_contribute(const Vec3f &pos) const {
    auto node=root;
    while(!node->isLeaf()){
        if(pos[node->partition_axis]<node->partition_pos)
            node=node->leftChild;
        else node=node->rightChild;
    }
    return node->contribute_grids;
}
