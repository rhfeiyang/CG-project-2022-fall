#include "common.h"
#include "VDBLoader.h"

#include "config_io.h"
#include "integrator.h"
#include "utils.h"
#include "scene.h"

//imGUI
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <ImGuizmo.h>


const int WIDTH = 800;
const int HEIGHT = 300;

bool firstMouse = true;
float lastX = WIDTH / 2.f;
float lastY = HEIGHT / 2.f;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

GLFWwindow *window;

namespace VolumeRendering {

    Config config;
    std::ifstream fin;
    std::string file_path;
    VDBLoader loader;
    std::shared_ptr<ImageRGB> rendered_img;
    std::shared_ptr<Camera> camera;
    std::shared_ptr<Scene> scene;
    std::chrono::time_point<std::chrono::system_clock> start, end;
    std::unique_ptr<Integrator> integrator;

    bool show_demo_window = false;
    bool render = false;
    bool write_img = false;

    void LoadingConfig(int argc, char *argv[]) {
        /// load config from json file
        if (argc == 1) {
            std::cout << "No json specified, use default path." << std::endl;
            file_path = GetFilePath("configs/single-small.json");
            fin.open(file_path);
        } else {
            file_path = argv[1];
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
    }

    void InitSettings() {
        // initialize all settings from config
        // set image resolution.
        rendered_img = std::make_shared<ImageRGB>(config.image_resolution[0], config.image_resolution[1]);
        std::cout << "Image resolution: "
                  << config.image_resolution[0] << " x " << config.image_resolution[1] << std::endl;
        // set camera
        camera = std::make_shared<Camera>(config.cam_config, rendered_img);
        // construct scene.
        scene = std::make_shared<Scene>();
        initSceneFromConfig(config, scene);
        scene->setAmbient(Vec3f(0.1, 0.1, 0.1));
        //load vdb
        loader.load(GetFilePath(config.file_path));
    }
    void DrawContents(uint8_t *data) {
        glDrawPixels(WIDTH, HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, data);
    }
    void RenderImg() {
        integrator = std::make_unique<Integrator>(camera, scene, config.spp, loader.grids, config.iso_value, config.var,
                                                  config.step_scale);
        integrator->render();
        DrawContents(camera->getImage()->getdata());
        render = false;
    }

    void WriteImg() {
        rendered_img->writeImgToFile("../result.png");
        std::cout << "Image saved to disk." << std::endl;
        write_img = false;
    }

    void RenderMainImGui() {

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window//
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        if (ImGui::Button("Start Rendering")) {
            render = true;
        }

        if (render) {
            std::cout << "Start Rendering..." << std::endl;
            start = std::chrono::system_clock::now();
            // render scene
            RenderImg();

            end = std::chrono::system_clock::now();
            auto time = std::chrono::duration<double>(end - start);
            std::cout << "\nRender Finished in " << time << "s." <<std::endl;

        }

        if (ImGui::Button("Write Image"))
            write_img = true;
        if (write_img)
            WriteImg();


        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

}

//#define TEST
struct Local {
    static inline void diff(const Vec3f &a, const Vec3f &b, Vec3f &result) {
        result = a - b;
    }
};

std::chrono::time_point<std::chrono::system_clock> start, end;
std::chrono::duration<double> elapsed_seconds;


void processInput(GLFWwindow *window, std::shared_ptr<Camera> camera) {
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

void mouse_callback(GLFWwindow *window, double x, double y) {
    if (firstMouse) {
        lastX = x;
        lastY = y;
        firstMouse = false;
    }

    float xoffset = x - lastX;
    float yoffset = lastY - y; // reversed since y-coordinates go from bottom to top

    lastX = x;
    lastY = y;


}



int main(int argc, char *argv[]) {

    VolumeRendering::LoadingConfig(argc, argv);
    VolumeRendering::InitSettings();


//GUI
    WindowGuard windowGuard(window, WIDTH, HEIGHT, "CG");
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

    ImGui::StyleColorsDark(); // Setup Dear ImGui style
    const char *glsl_version = "#version 130";
    ImGui_ImplGlfw_InitForOpenGL(window, true); // Setup Platform/Renderer bindings
    ImGui_ImplOpenGL3_Init(glsl_version);

    while (!glfwWindowShouldClose(window)) {
        //processInput(window);
        VolumeRendering::RenderMainImGui();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }


#ifndef TEST
    // init integrator
//    auto single_grid=loader.grids.grids[0];
//    auto length=(loader.grids.whole_wbbox.max() - loader.grids.whole_wbbox.min())[loader.grids.whole_wbbox.maxExtent()];
//    auto dim=single_grid->evalActiveVoxelBoundingBox().dim();



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