#ifndef __VDB_VOLUME_LOADER_H__
#define __VDB_VOLUME_LOADER_H__



#include <memory>
#include "common.h"
//#include <openvdb/io/Stream.h>
#include <algorithm>

template<typename GridType>
class VDBLoader {
public:
    using TreeType = typename GridType::TreeType;

    explicit VDBLoader(std::string filename);

    ~VDBLoader();

    std::vector<std::string> &getGridNames();

//    std::vector<openvdb::GridBase::Ptr> grids_base;
//    std::vector<typename GridType::Ptr> grids;
    std::vector<FloatGrid::Ptr> grids;
    std::vector<double> dx;
    openvdb::io::File *file;
    std::vector<std::string> gridNames;
    std::string filename;

    std::string getGridType(openvdb::GridBase::Ptr grid);
    void SortBydx();

private:
};

#endif /* __VDB_VOLUME_LOADER_H__ */
