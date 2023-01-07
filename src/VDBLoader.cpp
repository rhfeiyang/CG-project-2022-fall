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
    float DX;


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
        auto dx = grid->metaValue<double>("dx");
        if (abs(DX-dx) > 1e-4) {
            DX = dx;
            cout << DX << endl;
        }
//        dx = 0.008;
        auto acc_grid = grid->getAccessor();

        for (auto ite = grid->beginValueAll(); ite; ++ite) {
            auto coord = ite.getCoord();
            //auto value = ite.getValue().length();
            auto max_coord = ite.getBoundingBox().max();
            Vec3f gx, gy, gz;
            // grad_x
            if (coord[0] == 0) {
                gx = (acc_grid.getValue(Coord(coord.x() + 1, coord.y(), coord.z())) -
                      acc_grid.getValue(Coord(coord.x(), coord.y(), coord.z()))) / dx;
            } else if (coord[0] == max_coord[0]) {
                gx = (acc_grid.getValue(Coord(coord.x(), coord.y(), coord.z())) -
                      acc_grid.getValue(Coord(coord.x() - 1, coord.y(), coord.z()))) / dx;
            } else {
                gx = (acc_grid.getValue(Coord(coord.x() + 1, coord.y(), coord.z())) -
                      acc_grid.getValue(Coord(coord.x() - 1, coord.y(), coord.z()))) / (2 * dx);
            }
            // grad_y
            if (coord[1] == 0) {
                gy = (acc_grid.getValue(Coord(coord.x(), coord.y() + 1, coord.z())) -
                      acc_grid.getValue(Coord(coord.x(), coord.y(), coord.z()))) / dx;
            } else if (coord[1] == max_coord[1]) {
                gy = (acc_grid.getValue(Coord(coord.x(), coord.y(), coord.z())) -
                      acc_grid.getValue(Coord(coord.x(), coord.y() - 1, coord.z()))) / dx;
            } else {
                gy = (acc_grid.getValue(Coord(coord.x(), coord.y() + 1, coord.z())) -
                      acc_grid.getValue(Coord(coord.x(), coord.y() - 1, coord.z()))) / (2 * dx);
            }
            // grad_z
            if (coord[2] == 0) {
                gz = (acc_grid.getValue(Coord(coord.x(), coord.y(), coord.z() + 1)) -
                      acc_grid.getValue(Coord(coord.x(), coord.y(), coord.z()))) / dx;
            } else if (coord[2] == max_coord[2]) {
                gz = (acc_grid.getValue(Coord(coord.x(), coord.y(), coord.z())) -
                      acc_grid.getValue(Coord(coord.x(), coord.y(), coord.z() - 1))) / dx;
            } else {
                gz = (acc_grid.getValue(Coord(coord.x(), coord.y(), coord.z() + 1)) -
                      acc_grid.getValue(Coord(coord.x(), coord.y(), coord.z() - 1))) / (2 * dx);
            }

            float q = -0.5f * (gx.x() * gx.x() + gy.y() * gy.y() + gz.z() * gz.z())
                    - gx.y() * gy.x() - gx.z() * gz.x() - gy.z() * gz.y();
            accessor.setValue(coord, q);
            accessor.setActiveState(coord, ite.isValueOn());
        }
        openvdb::tools::changeBackground(floatgrid->tree(), (float) 1e30);

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

//            if (ite.isValueOn()) {
//                auto value = ite.getValue().length();
//                accessor.setValue(coord, value);
//                accessor.setActiveState(coord, true);
//            }
//            else {
//                accessor.setValue(coord, std::numeric_limits<float>::max());
//                accessor.setActiveState(coord, false);
//            }
