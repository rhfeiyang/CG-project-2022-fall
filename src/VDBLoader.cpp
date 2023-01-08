
#include "VDBLoader.h"
float VDBLoader::q_criterion(const Vec3sGrid& grid,const Coord& coord,const iBBox& ibbox){
    auto dx = grid.metaValue<double>("dx");
    dx=0.008;
    auto max_coord=ibbox.max();
    auto acc_grid=grid.getAccessor();
    Coord dx_coord{1, 0, 0};Coord dy_coord{0, 1, 0};Coord dz_coord{0, 0, 1};

    Vec3f gx, gy, gz;
    // grad_x
    if (coord[0] == 0) {
        gx = (acc_grid.getValue(coord + dx_coord) -
              acc_grid.getValue(coord)) / dx;
    } else if (coord[0] == max_coord[0]) {
        gx = (acc_grid.getValue(coord) -
              acc_grid.getValue(coord - dx_coord)) / dx;
    } else {
        gx = (acc_grid.getValue(coord + dx_coord) -
              acc_grid.getValue(coord - dx_coord)) / (2 * dx);
    }
    // grad_y
    if (coord[1] == 0) {
        gy = (acc_grid.getValue(coord + dy_coord) -
              acc_grid.getValue(coord)) / dx;
    } else if (coord[1] == max_coord[1]) {
        gy = (acc_grid.getValue(coord) -
              acc_grid.getValue(coord - dy_coord)) / dx;
    } else {
        gy = (acc_grid.getValue(coord + dy_coord) -
              acc_grid.getValue(coord - dy_coord)) / (2 * dx);
    }
    // grad_z
    if (coord[2] == 0) {
        gz = (acc_grid.getValue(coord + dz_coord) -
              acc_grid.getValue(coord)) / dx;
    } else if (coord[2] == max_coord[2]) {
        gz = (acc_grid.getValue(coord) -
              acc_grid.getValue(coord - dz_coord)) / dx;
    } else {
        gz = (acc_grid.getValue(coord + dz_coord) -
              acc_grid.getValue(coord - dz_coord)) / (2 * dx);
    }
    return -0.5f * (gx.x() * gx.x() + gy.y() * gy.y() + gz.z() * gz.z())
           - gx.y() * gy.x() - gx.z() * gz.x() - gy.z() * gz.y();
}
//template<typename GridType>
std::string VDBLoader::getGridType(const openvdb::GridBase::Ptr& grid) {
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

//template<typename GridType>
std::vector<std::string> &VDBLoader::getGridNames() {
    return gridNames;
}


//template<typename GridType>
VDBLoader::VDBLoader(const std::string& filename) : filename(filename) {
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
        openvdb::tools::changeBackground(grid->tree(), (Vec3f) 1e30);
        FloatGrid::Ptr floatgrid = FloatGrid::create(1e30);

        auto accessor = floatgrid->getAccessor();

        auto grid_ibbox = grid->evalActiveVoxelBoundingBox();
        for (auto ite = grid->beginValueOn(); ite; ++ite) {
            auto coord = ite.getCoord();
            accessor.setValue(coord, q_criterion(*grid,coord,grid_ibbox));
            accessor.setActiveState(coord, ite.isValueOn());
        }
//        for(auto ite=floatgrid->beginValueAll();ite;++ite) {
//            if (!ite.isValueOn()) cout << ite.getValue() << endl;
//        }
        for (auto ite = grid->beginMeta(); ite != grid->endMeta(); ++ite) {
            auto name = ite->first;
            auto value = (ite->second)->copy();
            floatgrid->insertMeta(name, *value);
        }
        floatgrid->setTransform(grid->transformPtr());
        grids.addGrid(floatgrid,grid_ibbox);
//        for(auto ite=floatgrid->beginValueAll();ite;++ite){
//            if (!ite.isValueOn() && ite.getValue() > 1) cout << floatgrid->indexToWorld(ite.getCoord()) << "\t" << ite.getValue() << endl;
//        }
    }
}


//template<typename GridType>
VDBLoader::~VDBLoader() {
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
