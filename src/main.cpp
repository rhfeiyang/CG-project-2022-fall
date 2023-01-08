#include "common.h"
#include "VDBLoader.h"

#include "config_io.h"
#include "integrator.h"
#include "utils.h"
#include "scene.h"


const int WIDTH = 800;
const int HEIGHT = 300;

bool firstMouse = true;
float lastX = WIDTH / 2.f;
float lastY = HEIGHT / 2.f;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

GLFWwindow *window;

namespace GUI{


}

//#define TEST
struct Local {
    static inline void diff(const Vec3f& a, const Vec3f& b, Vec3f& result) {
        result = a - b;
    }
};

std::chrono::time_point<std::chrono::system_clock> start, end;
std::chrono::duration<double> elapsed_seconds;


void processInput(GLFWwindow *window,std::shared_ptr<Camera> camera)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera->ProcessKeyboard(Camera_Movement::FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera->ProcessKeyboard(Camera_Movement::BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera->ProcessKeyboard(Camera_Movement::LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera->ProcessKeyboard(Camera_Movement::RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera->ProcessKeyboard(Camera_Movement::UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera->ProcessKeyboard(Camera_Movement::DOWN, deltaTime);
}

void mouse_callback(GLFWwindow *window, double x, double y)
{
    if (firstMouse)
    {
        lastX = x;
        lastY = y;
        firstMouse = false;
    }

    float xoffset = x - lastX;
    float yoffset = lastY - y; // reversed since y-coordinates go from bottom to top

    lastX = x;
    lastY = y;


}

void scroll_callback(GLFWwindow* window, double x, double y)
{

}
void RenderOpenGL()
{
    /*auto data = GenerateRandomData(WIDTH * HEIGHT * 3);
    DrawContents(data);
    delete[] data;*/
    start = std::chrono::system_clock::now();


    end = std::chrono::system_clock::now();
    elapsed_seconds = end - start;
    // update_scale();


}
void DrawContents(uint8_t *data)
{
    glDrawPixels(WIDTH, HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, data);
}

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
    initSceneFromConfig(config, scene);
//load vdb
    VDBLoader loader(GetFilePath(config.file_path));

//    Kdtree kdtree(loader.grids);
//    WindowGuard windowGuard(window, WIDTH, HEIGHT, "CG");
//    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

//    while(!glfwWindowShouldClose(window)) {
//        float currentFrame = static_cast<float>(glfwGetTime());
//        deltaTime = currentFrame - lastFrame;
//        lastFrame = currentFrame;
//        processInput(window,camera);
//        std::unique_ptr<Integrator> integrator
//                = std::make_unique<Integrator>(camera, scene, config.spp, loader.grids,config.iso_value,config.var,config.step_scale);
//        integrator->render();
//        DrawContents(camera->getImage()->getdata());
//
//        glfwSwapBuffers(window);
//        glfwPollEvents();
//    }


#ifndef TEST
    // init integrator
//    auto single_grid=loader.grids.grids[0];
//    auto length=(loader.grids.whole_wbbox.max() - loader.grids.whole_wbbox.min())[loader.grids.whole_wbbox.maxExtent()];
//    auto dim=single_grid->evalActiveVoxelBoundingBox().dim();

    std::cout << "Start Rendering..." << std::endl;
    auto start = std::chrono::steady_clock::now();
    // render scene
    std::unique_ptr<Integrator> integrator
            = std::make_unique<Integrator>(camera, scene, config.spp, loader.grids,config.iso_value,config.var,config.step_scale);
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
    for (auto &grid: loader.grids.grids) {
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
            cout<<grid_idx<<" "<<value<<" "<<iter.getCoord()<<" "<<grid->indexToWorld(iter.getCoord())<<endl;

        }
        grid_idx++;
    }
#endif
    return 0;
}