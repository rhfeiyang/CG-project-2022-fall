//
// Created by Renhui on 2022/12/31.
//
#include "common.h"
#include "Cell.h"
#ifndef CG_PROJECT_2022_FALL_BRICK_H
#define CG_PROJECT_2022_FALL_BRICK_H

class Brick {
public:
    ///may need modify?
    Brick(Coord coord, int l, Vec3i dim, Grids_data& gridsData, wBBox bbox);
    bool Get_value(Coord coord, double& result);//first test whether is in
    [[nodiscard]] Vec3i Get_dim() const {return cell_dim;}
    [[nodiscard]] int Get_level() const{return level;}
private:
    Coord coord; //lower-left corner coord
    int level;
    Vec3i cell_dim;
    std::vector<Cell> cells;
};
#endif //CG_PROJECT_2022_FALL_BRICK_H
