//
// Created by Renhui on 2022/12/31.
//
#include "common.h"
#ifndef CG_PROJECT_2022_FALL_CELL_H
#define CG_PROJECT_2022_FALL_CELL_H
class Cell{
public:
    Cell(Vec3i p_idx, int l);
    Cell(int i, int j, int k, int l): Cell(Vec3i(i,j,k),l){}
    void update_level(int l);
    [[nodiscard]] size_t size() const;
private:
    Vec3i poi_idx;
    int level;
    size_t Size;
};
#endif //CG_PROJECT_2022_FALL_CELL_H
