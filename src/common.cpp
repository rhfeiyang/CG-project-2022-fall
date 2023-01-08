//
// Created by Renhui on 2023/1/2.
//
#include "common.h"
//Vec3f Grid_world_pos(openvdb::GridBase::Ptr grid, Coord coord){
//    auto O=grid->metaValue<openvdb::Vec3d>("origin");
//    auto dx=grid->metaValue<double>("dx");
//    return O+dx*coord.asVec3i();
//}

std::string GetFilePath(const std::string& target, int depth/* = 5*/) {
    std::string path = target;
    for (int i = 0; i < depth; ++i) {
        FILE *file = fopen(path.c_str(), "r");
        if (file) {
            fclose(file);
            return path;
        }
        path = "../" + path;
    }
    std::cerr << "GetFilePath(): Failed to get file: " + target << std::endl;
    abort();
}

void Grids_data::addGrid(const FloatGrid::Ptr& grid) {
    addGrid(grid,grid->evalActiveVoxelBoundingBox());
}

void Grids_data::addGrid(const FloatGrid::Ptr& grid,const iBBox &ibbox) {
    grids.push_back(grid);
    auto dx_temp=grid->metaValue<double>("dx");

    dx.push_back(dx_temp);
    if(dx_temp<min_dx) min_dx=dx_temp;
    if(dx_temp>max_dx) max_dx=dx_temp;
    auto wbbox= ibbTowbb(grid,ibbox);
    whole_wbbox.expand(wbbox);
    wbboxes.emplace_back(wbbox);
}

bool floatEqual(const float& first,const float& second){
    return abs(first-second)<EPS;
}

wBBox ibbTowbb(const FloatGrid::Ptr & grid,const iBBox& ibb){
    auto lb=grid->indexToWorld(ibb.min());
    auto rt=grid->indexToWorld(ibb.max());
    return {lb,rt};
}
