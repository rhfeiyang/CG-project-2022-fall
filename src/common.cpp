//
// Created by Renhui on 2023/1/2.
//
#include "common.h"
Vec3f Grid_world_pos(openvdb::GridBase::Ptr grid, Coord coord){
    auto O=grid->metaValue<openvdb::Vec3d>("origin");
    auto dx=grid->metaValue<double>("dx");
    return O+dx*coord.asVec3i();
}

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