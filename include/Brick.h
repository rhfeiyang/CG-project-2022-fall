//
// Created by Renhui on 2022/12/31.
//
#include "common.h"

#ifndef CG_PROJECT_2022_FALL_BRICK_H
#define CG_PROJECT_2022_FALL_BRICK_H

class Brick {
public:
    Brick(Vec3f p, int l, Vec3i c):poi(p),level(l),cell_num(c){}
private:
    Vec3f poi; //lower-left corner i,j,k
    int level;
    Vec3i cell_num;
};
#endif //CG_PROJECT_2022_FALL_BRICK_H
