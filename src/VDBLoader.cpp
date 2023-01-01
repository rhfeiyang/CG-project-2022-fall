#include "VDBLoader.hpp"

template<typename GridType>
std::string VDBLoader<GridType>::getGridType(openvdb::GridBase::Ptr grid) {
    if (grid->isType<openvdb::BoolGrid>())
        return "BoolGrid";
    else if (grid->isType<openvdb::FloatGrid>())
        return "FloatGrid";
    else if (grid->isType<openvdb::DoubleGrid>())
        return "DoubleGrid";
    else if (grid->isType<openvdb::Int32Grid>())
        return "Int32Grid";
    else if (grid->isType<openvdb::Int64Grid>())
        return "Int64Grid";
    else if (grid->isType<openvdb::Vec3IGrid>())
        return "Vec3IGrid";
    else if (grid->isType<openvdb::Vec3SGrid>())
        return "Vec3SGrid";
    else if (grid->isType<openvdb::Vec3DGrid>())
        return "Vec3DGrid";

    return "Type not found";
}
template<typename GridType>
std::vector<std::string> &VDBLoader<GridType>::getGridNames() {
    return gridNames;
}

template<typename GridType>
VDBLoader<GridType>::VDBLoader(std::string filename) : filename(filename){
    openvdb::initialize();
    file = new openvdb::io::File(filename);
    file->getSize();
    file->open();
    cout << filename << endl;
    for (openvdb::io::File::NameIterator nameIter = file->beginName();
         nameIter != file->endName(); ++nameIter) {
        std::string gridName = nameIter.gridName();
        openvdb::GridBase::Ptr baseGrid = file->readGrid(gridName);
        gridNames.push_back(gridName + " (" + getGridType(baseGrid) + ")");
        grids_base.push_back(baseGrid);
        auto type=baseGrid->type();
        cout<<type<<endl;
        assert(type==GridType::gridType());
        //convert base to grid type
        auto grid=openvdb::gridPtrCast<GridType>(baseGrid);
        grids.push_back(grid);

    }
//    auto grid=openvdb::gridPtrCast<openvdb::Vec3fGrid>(grids[0]);
//    for(auto ite=grid->beginValueOn();ite;++ite){
//        cout<<ite.getValue()<<endl;
//    }
}



template<typename GridType>
VDBLoader<GridType>::~VDBLoader() {
    if(file){
        file->close();
        delete[] file;
    }
}

template<typename GridType>
void VDBLoader<GridType>::SortBydx() {
    std::sort(grids.begin(),grids.end(),[](auto &agrid,auto &bgrid){
        return agrid->metaValue<double>("dx")< bgrid->metaValue<double>("dx");
    });
}
