//
// Created by Renhui on 2022/12/31.
//
#include "common.h"
#ifndef CG_PROJECT_2022_FALL_CELL_H
#define CG_PROJECT_2022_FALL_CELL_H

//float MIN_DX;
//openvdb::Vec3f WORLD_ORIGIN;
class Cell{
public:
    Cell(){}
    ~Cell(){}
    Cell(openvdb::Coord p_idx, uint32_t l,float value):coord(p_idx),level(l),value(value){}
    Cell(int i, int j, int k , uint32_t l,float v): Cell(openvdb::Coord(i,j,k),l,v){}
    void update_level(int l){
        level=l;
    }

private:
    openvdb::Coord coord;
    uint32_t level;
    float value;

};
#endif //CG_PROJECT_2022_FALL_CELL_H
