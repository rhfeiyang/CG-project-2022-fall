#include "common.h"
#include "VDBLoader.h"
#include "VDBLoader.cpp"
#include "Cell.h"
#include "Brick.h"
#include "scene.h"

struct Local {
    static inline void diff(const Vec3f& a, const Vec3f& b, Vec3f& result) {
        result = a - b;
    }
};


int main() {
    VDBLoader<Vec3sGrid> loader("../resource/single-res small.vdb");

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
            cout<<grid_idx<<" "<<value<<" "<<iter.getCoord()<<" "<<Grid_world_pos(grid,iter.getCoord())<<endl;

        }
        grid_idx++;
    }
    /// settings

    // window
    constexpr int window_width = 1920;
    constexpr int window_height = 1080;


    /// setup window
    GLFWwindow* window;
    {
        if (!glfwInit()) // initialize glfw library
            return -1;

        // setting glfw window hints and global configurations
        {
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // use core mode
            // glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE); // use debug context

        }

        // create a windowed mode window and its OpenGL context
        window = glfwCreateWindow(window_width, window_height, "CS171 HW5: Cloth Simulation", NULL, NULL);
        if (!window) {
            glfwTerminate();
            return -1;
        }

        // make the window's context current
        glfwMakeContextCurrent(window);

        // load Opengl
        if (!gladLoadGL()) {
            glfwTerminate();
            return -1;
        }

        // setup call back functions
        glfwSetFramebufferSizeCallback(window, Input::CallBackResizeFlareBuffer);
    }

    /// main Loop
    {
        // shader
        Shader::Initialize();

        // scene
        Scene scene(45);
        scene.camera.transform.position = { 0, -1.5, -6 };
        scene.camera.transform.rotation = { 0, 0, 1, 0 };
        scene.light_position = { 0, 3, -10 };
        scene.light_color = gVec3(1, 1, 1) * Float(1.125);


        // loop until the user closes the window
        Input::Start(window);
//    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glEnable(GL_DEPTH_TEST);
        while (!glfwWindowShouldClose(window)) {
            Input::Update();
            Time::Update();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            /// terminate
            if (Input::GetKey(KeyCode::Escape))
                glfwSetWindowShouldClose(window, true);

            /// fixed update
            for (unsigned i = 0; i < Time::fixed_update_times_this_frame; ++i) {
                if(Input::GetKey(KeyCode::Space)) { //! only when space is pressed
                    scene.FixedUpdate();
                }
            }

            /// update
            {
                scene.Update();
//        printf("Pos = (%f, %f, %f)\n", scene.camera.transform.position.x, scene.camera.transform.position.y, scene.camera.transform.position.z);
//        printf("Rot = (%f, %f, %f, %f)\n", scene.camera.transform.rotation.w, scene.camera.transform.rotation.x, scene.camera.transform.rotation.y, scene.camera.transform.rotation.z);
//        printf("\n");
            }

            /// render
            {
                scene.RenderUpdate();
            }

            // swap front and back buffers
            glfwSwapBuffers(window);

            // poll for and process events
            glfwPollEvents();
        }
    }

    glfwTerminate();
    return 0;
}