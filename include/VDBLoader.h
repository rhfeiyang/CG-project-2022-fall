#ifndef VDB_VOLUME_LOADER_H_
#define VDB_VOLUME_LOADER_H_

#include <memory>
#include "common.h"
//#include <openvdb/io/Stream.h>
#include <algorithm>


//template<typename GridType>
class VDBLoader {
public:
//    using TreeType = typename GridType::TreeType;

    explicit VDBLoader(const std::string& filename);

    ~VDBLoader();

    std::vector<std::string> &getGridNames();

//    std::vector<openvdb::GridBase::Ptr> grids_base;
//    std::vector<typename GridType::Ptr> grids;

//    std::vector<FloatGrid::Ptr> grids;
//    std::vector<double> dx;
    Grids_data grids;
    openvdb::io::File *file;
    std::vector<std::string> gridNames;
    std::string filename;

    static std::string getGridType(const openvdb::GridBase::Ptr& grid);
    static float q_criterion(const Vec3sGrid& grid,const Coord& coord,const iBBox& ibbox);
//    void SortBydx();
private:
};


#endif /* VDB_VOLUME_LOADER_H_ */
