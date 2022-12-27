#ifndef __VDB_VOLUME_LOADER_HPP__
#define __VDB_VOLUME_LOADER_HPP__

#pragma  once
#include <memory>

#include "openvdb/openvdb.h"
#include <openvdb/io/Stream.h>
#include "common.h"


class VDBLoader {
public:
  explicit VDBLoader(const char* filename_);
    ~VDBLoader();

private:
    openvdb::GridPtrVecPtr grids;

std::string  filename;

};

#endif /* __VDB_VOLUME_LOADER_HPP__ */
