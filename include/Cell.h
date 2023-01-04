//
// Created by Renhui on 2022/12/31.
//
#include "common.h"
#ifndef CG_PROJECT_2022_FALL_CELL_H
#define CG_PROJECT_2022_FALL_CELL_H

class Cell{
public:
    Cell(){}
    ~Cell(){}
    Cell(openvdb::Coord p_idx, int l,float value):coord(p_idx),level(l),value(value){}
    Cell(int i, int j, int k , int l,float v): Cell(openvdb::Coord(i,j,k),l,v){}

private:
    ///Does coord necessary?
    openvdb::Coord coord;
    int level;
    float value;
};
#endif //CG_PROJECT_2022_FALL_CELL_H
