#ifndef __VDB_VOLUME_LOADER_HPP__
#define __VDB_VOLUME_LOADER_HPP__

#pragma  once
#include <memory>

#include "openvdb/openvdb.h"
#include <openvdb/io/Stream.h>
#include "common.h"


class VDBLoader {
public:
  explicit VDBLoader(std::string filename);
    ~VDBLoader();
    std::vector<std::string> &getGridNames();
private:
    std::vector<openvdb::GridBase::Ptr> grids;
    openvdb::io::File *file;
    std::vector<std::string> gridNames;
    std::string  filename;

    std::string getGridType(openvdb::GridBase::Ptr grid);
};

#endif /* __VDB_VOLUME_LOADER_HPP__ */
