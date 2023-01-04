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
    grids.push_back(grid);
    auto dx_temp=grid->metaValue<double>("dx");
//    auto O=grid->metaValue<openvdb::Vec3d>("origin");
    dx.push_back(dx_temp);
    if(dx_temp<min_dx) min_dx=dx_temp;
    if(dx_temp>max_dx) max_dx=dx_temp;
    auto ibbox=grid->evalActiveVoxelBoundingBox();
    wbbox.expand(grid->indexToWorld(ibbox.min()));
    wbbox.expand(grid->indexToWorld(ibbox.max()));
}