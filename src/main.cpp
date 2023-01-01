
#include "VDBLoader.hpp"
#include "VDBLoader.cpp"
#include "common.h"
#include <openvdb/openvdb.h>
#include "Cell.h"

int main() {
    //VDBLoader<openvdb::FloatGrid > loader("../resource/bunny.vdb");
    VDBLoader<openvdb::Vec3SGrid > loader("../resource/multi-res small.vdb");
//Construct ABR

    for (auto &grid: loader.grids) {

        //get meta data
        for (auto iter = grid->beginMeta(); iter != grid->endMeta(); ++iter) {
            const std::string &name = iter->first;
            openvdb::Metadata::Ptr value = iter->second;
            std::string valueAsString = value->str();
            std::cout << name << " = " << valueAsString << std::endl;
        }

        //value iteration
        for (auto iter = grid->cbeginValueOn(); iter.test(); ++iter) {
            auto dist = *iter;
            auto bbox = iter.getBoundingBox();
            //cout << dist << bbox << endl;
            //Cell cell;

        }

    }
//Draw


    return 0;
}