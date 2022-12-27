#include "VDBLoader.hpp"


VDBLoader::VDBLoader(const char *filename_):filename(filename_) {
    cout<<filename.data()<<endl;
    openvdb::initialize();
    openvdb::io::File file(filename.c_str());
    file.open();
    grids=file.getGrids();
    for(auto & iter : *grids){
        auto grid=openvdb::gridPtrCast<openvdb::FloatGrid> (iter);
        grid->getAccessor();
    }

    file.close();
}


VDBLoader::~VDBLoader(){

}
