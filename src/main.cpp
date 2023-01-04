#include "common.h"
#include "VDBLoader.h"
#include "VDBLoader.cpp"
#include "Cell.h"
#include "Brick.h"
#include "config_io.h"
#include "integrator.h"

//#define TEST
struct Local {
    static inline void diff(const Vec3f& a, const Vec3f& b, Vec3f& result) {
        result = a - b;
    }
};


int main(int argc, char *argv[]) {

    /// load config from json file
    Config config;
    std::ifstream fin;
    std::string file_path;
    if (argc == 1) {
        std::cout << "No json specified, use default path." << std::endl;
        file_path=GetFilePath("configs/single-small.json");
        fin.open(file_path);
    } else {
        file_path=argv[1];
        fin.open(GetFilePath(file_path));
    }
    if (!fin.is_open()) {
        std::cerr << "Can not open json file. Exit." << std::endl;
        exit(0);
    } else {
        std::cout << "Json file loaded from " << file_path << std::endl;
    }
    // parse json object to Config
    try {
        nlohmann::json j;
        fin >> j;
        nlohmann::from_json(j, config);
        fin.close();
    } catch (nlohmann::json::exception &ex) {
        fin.close();
        std::cerr << "Error:" << ex.what() << std::endl;
        exit(-1);
    }
    std::cout << "Parsed json to config. Start building scene..." << std::endl;
    // initialize all settings from config
    // set image resolution.
    std::shared_ptr<ImageRGB> rendered_img
            = std::make_shared<ImageRGB>(config.image_resolution[0], config.image_resolution[1]);
    std::cout << "Image resolution: "
              << config.image_resolution[0] << " x " << config.image_resolution[1] << std::endl;
    // set camera
    std::shared_ptr<Camera> camera = std::make_shared<Camera>(config.cam_config, rendered_img);
    // construct scene.
    auto scene = std::make_shared<Scene>();
//    initSceneFromConfig(config, scene);
//load vdb
    VDBLoader<Vec3sGrid> loader(GetFilePath(config.file_path));
#ifndef TEST
    // init integrator
    auto single_grid=loader.grids.grids[0];
    auto length=(loader.grids.wbbox.max()-loader.grids.wbbox.min())[loader.grids.wbbox.maxExtent()];
//    auto dim=single_grid->evalActiveVoxelBoundingBox().dim();
    std::unique_ptr<Integrator> integrator
            = std::make_unique<Integrator>(camera, scene, config.spp, single_grid,length+2,config.iso_value,config.var);
    std::cout << "Start Rendering..." << std::endl;
    auto start = std::chrono::steady_clock::now();
    // render scene
    integrator->render();
    auto end = std::chrono::steady_clock::now();
    auto time = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
    std::cout << "\nRender Finished in " << time << "s." << std::endl;
    rendered_img->writeImgToFile("../result.png");
    std::cout << "Image saved to disk." << std::endl;



#else
//    loader.SortBydx();
//Construct ABR
//    std::vector<Cell> cells;


//    float MIN_DX=loader.grids[0]->metaValue<double>("dx");
//    WORLD_ORIGIN=loader.grids[0]->metaValue<double>("origin");

    int grid_idx=0;
    for (auto &grid: loader.grids) {
        //get meta data
        for (auto iter = grid->beginMeta(); iter != grid->endMeta(); ++iter) {
            const std::string &name = iter->first;
            openvdb::Metadata::Ptr value = iter->second;
            std::string valueAsString = value->str();
            std::cout << name << " = " << valueAsString << std::endl;
        }

        //value iteration

        auto origin=grid->metaValue<openvdb::Vec3d>("origin");
        auto dx=grid->metaValue<double>("dx");
        for(auto iter=grid->beginValueOn();iter;++iter) {
            //check empty
            if (!iter.isValueOn()) continue;
            auto value = iter.getValue();
//            auto norm = iter->lengthSqr();
            auto coord = iter.getCoord();
            if(abs(value-0.066)>0.002)
            cout<<grid_idx<<" "<<value<<" "<<iter.getCoord()<<" "<<Grid_world_pos(grid,iter.getCoord())<<endl;

        }
        grid_idx++;
    }
#endif
    return 0;
}