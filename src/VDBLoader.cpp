#include "VDBLoader.h"

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
VDBLoader<GridType>::VDBLoader(std::string filename) : filename(filename) {
    openvdb::initialize();
    file = new openvdb::io::File(filename);
    file->getSize();
    file->open();
    cout << filename << endl;
    for (openvdb::io::File::NameIterator nameIter = file->beginName();
         nameIter != file->endName(); ++nameIter) {
        std::string gridName = nameIter.gridName();
//        std::string gridName ="vel_2";
        openvdb::GridBase::Ptr baseGrid = file->readGrid(gridName);
        gridNames.push_back(gridName + " (" + getGridType(baseGrid) + ")");
//        grids_base.push_back(baseGrid);
        auto type = baseGrid->type();
        cout << gridName << endl;
        assert(type == GridType::gridType());
        //convert base to grid type
        auto grid = openvdb::gridPtrCast<Vec3sGrid>(baseGrid);

        FloatGrid::Ptr floatgrid = FloatGrid::create();
        auto accessor = floatgrid->getAccessor();
        for (auto ite = grid->beginValueAll(); ite; ++ite) {
            auto coord = ite.getCoord();
//            if (ite.isValueOn()) {
//                auto value = ite.getValue().length();
//                accessor.setValue(coord, value);
//                accessor.setActiveState(coord, true);
//            }
//            else {
//                accessor.setValue(coord, std::numeric_limits<float>::max());
//                accessor.setActiveState(coord, false);
//            }
            auto value = ite.getValue().length();
            accessor.setValue(coord, value);
            accessor.setActiveState(coord, ite.isValueOn());
        }
        openvdb::tools::changeBackground(floatgrid->tree(), std::numeric_limits<float>::max());

//        for(auto ite=floatgrid->beginValueAll();ite;++ite) {
//            if (!ite.isValueOn()) cout << ite.getValue() << endl;
//        }
        for (auto ite = grid->beginMeta(); ite != grid->endMeta(); ++ite) {
            auto name = ite->first;
            auto value = (ite->second)->copy();
            floatgrid->insertMeta(name, *value);
        }
        floatgrid->setTransform(grid->transformPtr());
        grids.addGrid(floatgrid);
//        for(auto ite=floatgrid->beginValueAll();ite;++ite){
//            if (!ite.isValueOn() && ite.getValue() > 1) cout << floatgrid->indexToWorld(ite.getCoord()) << "\t" << ite.getValue() << endl;
//        }
    }

//    openvdb::io::File floatfile("../testbig.vdb");
//    floatfile.write(grids.grids);
//    floatfile.close();
//    auto grid=openvdb::gridPtrCast<openvdb::Vec3fGrid>(grids[0]);
//    for(auto ite=grid->beginValueOn();ite;++ite){
//        cout<<ite.getValue()<<endl;
//    }
}


template<typename GridType>
VDBLoader<GridType>::~VDBLoader() {
    if (file) {
        file->close();
        delete[] file;
    }
}

//template<typename GridType>
//void VDBLoader<GridType>::SortBydx() {
//    std::sort(grids.begin(),grids.end(),[](auto &agrid,auto &bgrid){
//        return agrid->metaValue<double>("dx")< bgrid->metaValue<double>("dx");
//    });
//}


