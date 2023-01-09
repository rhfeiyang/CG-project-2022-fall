//
// Created by Renhui on 2022/12/31.
//

#include "Construct.h"
#include <set>
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

KdTreeNode::KdTreeNode(Grids_data& gridsData,uint32_t bm,wBBox bbox, int depth):bitmap(bm),rightChild(nullptr),leftChild(nullptr) {
    ///For construct: find the longest axis by bbox, find a pos to create two children
    ///For leaf: create a brick, store cells
    ///Can depth has some effect?
    if(depth>8) return;
    if(bm==0) return;
    for (int i = 0; bm; i++, bm >>= 1) {
        if (bm & 1) {
            if (bm == 1) return;
            else break;
        }
    }

    partition_axis=bbox.maxExtent();

    float pos_range[2]={static_cast<float>(bbox.min()[partition_axis]),static_cast<float>(bbox.max()[partition_axis])};
    uint32_t temp_bm = bitmap;

    std::set<Pos,std::greater<>> min_pos;
    std::set<Pos> max_pos;
    std::vector<Pos> pos;
    for (int i = 0; temp_bm; i++, temp_bm >>= 1) {
        if (temp_bm & 1) {
            auto pos1=gridsData.wbboxes[i].min()[partition_axis];
            auto pos2=gridsData.wbboxes[i].max()[partition_axis];
            if(pos1>pos_range[0]+EPS && pos1<pos_range[1]-EPS){
                pos.push_back({pos1, (uint32_t) 1 << i});
                min_pos.insert({pos1, (uint32_t) 1 << i});
            }
            if(pos2>pos_range[0]+EPS && pos2<pos_range[1]-EPS){
//                cout<<pos_range[0]<<" "<<pos_range[1]<<endl;
                pos.push_back({pos2,(uint32_t)1 << i});
                max_pos.insert({pos2,(uint32_t)1 << i});
            }
        }
    }
//    for(auto& i : contribute_grids){
//        auto pos1=gridsData.wbboxes[i].min()[partition_axis];
//        auto pos2=gridsData.wbboxes[i].max()[partition_axis];
//        if(pos1>pos_range[0]+EPS && pos1<pos_range[1]-EPS) pos.push_back({pos1,i,true});
//        if(pos2>pos_range[0]+EPS && pos2<pos_range[1]-EPS) pos.push_back({pos2,i,false});
//    }

    if(!pos.empty()){
        std::sort(pos.begin(), pos.end());
        auto partition=pos[int((pos.size() - 1) / 2)];
        partition_pos=partition.pos;
        auto splitbbox= splitWbbox(bbox,partition_axis,partition_pos);
        auto leftbm = bitmap; auto rightbm=bitmap;
        for(auto min_po : min_pos){
            if(min_po.pos >= partition_pos-EPS){
                leftbm-=min_po.grid_idx_bm;
            }
            else break;
        }
        for(auto max_po:max_pos){
            if(max_po.pos<=partition_pos+EPS){
                rightbm-=max_po.grid_idx_bm;
            }
            else break;
        }
        leftChild=new KdTreeNode{gridsData,leftbm,splitbbox[0],depth+1};
        rightChild=new KdTreeNode{gridsData,rightbm,splitbbox[1],depth+1};
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
//    std::vector<int> contribute;
//    for(int i=0;i<gridsData.grids.size();i++){
//        contribute.emplace_back(i);
//    }
    uint32_t bitmap = (1 << gridsData.grids.size()) - 1;

    root=new KdTreeNode(gridsData,bitmap,gridsData.whole_wbbox,0);
}

uint32_t Kdtree::grid_contribute(const Vec3f &pos) const {
    auto node=root;
    while(!node->isLeaf()){
        if(pos[node->partition_axis]<node->partition_pos)
            node=node->leftChild;
        else node=node->rightChild;
    }
    return node->bitmap;
}





