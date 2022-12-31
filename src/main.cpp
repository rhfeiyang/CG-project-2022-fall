
#include "VDBLoader.hpp"
#include "VDBLoader.cpp"
#include "common.h"
#include <openvdb/openvdb.h>

int main() {
    VDBLoader<openvdb::Vec3SGrid > loader("../resource/multi-res small.vdb");
//Construct ABR

    for (auto &grid: loader.grids) {



        //get meta data
        for(auto iter=grid->beginMeta();iter!=grid->endMeta();++iter){
            const std::string& name = iter->first;
            openvdb::Metadata::Ptr value = iter->second;
            std::string valueAsString = value->str();
            std::cout << name << " = " << valueAsString << std::endl;
        }

        //value iteration
        for (auto iter = grid->cbeginValueOn(); iter.test(); ++iter) {
            auto dist = *iter;
            //cout << dist << endl;
        }
    }
//Draw


    return 0;
}