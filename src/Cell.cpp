//
// Created by Renhui on 2022/12/31.
//

#include "Cell.h"
Cell::Cell(Vec3i p_idx, int l):poi_idx(p_idx),level(l){
    Size= pow(2,l);
}
void Cell::update_level(int l) {
    level=l;
    Size= pow(2,l);
}
size_t Cell::size() const {
    return Size;
}