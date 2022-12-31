#ifndef __VDB_VOLUME_LOADER_HPP__
#define __VDB_VOLUME_LOADER_HPP__

#pragma  once

#include <memory>

#include "openvdb/openvdb.h"
#include <openvdb/io/Stream.h>
#include "common.h"

template<typename GridType>
class VDBLoader {
public:
    using TreeType = typename GridType::TreeType;

    explicit VDBLoader(std::string filename);

    ~VDBLoader();

    std::vector<std::string> &getGridNames();

    std::vector<openvdb::GridBase::Ptr> grids_base;
    std::vector<typename GridType::Ptr> grids;
    openvdb::io::File *file;
    std::vector<std::string> gridNames;
    std::string filename;

    std::string getGridType(openvdb::GridBase::Ptr grid);

private:


};

#endif /* __VDB_VOLUME_LOADER_HPP__ */
