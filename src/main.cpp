
#include "VDBLoader.hpp"
#include "VDBLoader.cpp"
#include "common.h"
#include <openvdb/openvdb.h>
#include "Cell.h"
#include "Brick.h"
#include "vector"

using GridType=openvdb::Vec3SGrid;
using GridValue=openvdb::Vec3s;
struct Local {
    static inline void diff(const GridValue& a, const GridValue& b, GridValue& result) {
        result = a - b;
    }
};

struct vdbcell{
    Vec3f world_poi;
    int grid_idx;
    Coord coord;
};
std::vector<vdbcell> cells;

int main() {
    //VDBLoader<openvdb::FloatGrid > loader("../resource/bunny.vdb");
    VDBLoader<GridType> loader("../resource/single-res small.vdb");

    loader.SortBydx();
//Construct ABR
//    std::vector<Cell> cells;


//    float MIN_DX=loader.grids[0]->metaValue<double>("dx");
//    WORLD_ORIGIN=loader.grids[0]->metaValue<double>("origin");

    int grid_idx=0;
    for (auto &grid: loader.grids) {
        //get meta data
        for (auto iter = grid->beginMeta(); iter != grid->endMeta(); ++iter) {
            const std::string &name = iter->first;
            openvdb::Metadata::Ptr value = iter->second;
            std::string valueAsString = value->str();
            std::cout << name << " = " << valueAsString << std::endl;
        }

        //value iteration

        auto origin=grid->metaValue<openvdb::Vec3d>("origin");
        auto dx=grid->metaValue<double>("dx");
        for(auto iter=grid->beginValueOn();iter;++iter) {
            //check empty
            if (!iter.isValueOn()) continue;
            auto value = iter.getValue();
            auto norm = iter->lengthSqr();
            auto coord = iter.getCoord();
            cout<<value<<" "<<iter.getCoord()<<" "<<grid_idx<<endl;

            //world position
            auto world_pos = origin + coord.asVec3d() * dx;
            //    cout<<"world position:"<<world_pos<<endl;
//            cells.push_back(Cell{coord, iter.getLevel(), norm});
            cells.push_back({world_pos,grid_idx,coord});
        }
        grid_idx++;
    }


//Draw

    //::vector<Cell> cells;
//    Brick(cells);

    return 0;
}