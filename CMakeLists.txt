cmake_minimum_required(VERSION 3.22)
project(CG_project_2022_fall)
find_package(OpenVDB CONFIG REQUIRED)
#find_package(GLUT REQUIRED)
Find_package(OpenGL)
find_package(OpenMP REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
set(CMAKE_CXX_STANDARD 20)

include_directories(include)
include_directories(src)
#include_directories(libs)
add_subdirectory(libs)
#add_subdirectory(cuda)

set(HEADS
        include/Cell.h
        include/Brick.h
        include/Support.h
        include/Construct.h
        include/Gradient.h
        include/Adaptive.h
        include/Opacity_corr.h
        include/Space_skipping.h
        include/ABR.h
        include/Transfer_func.h
        include/Render.h
        include/VDBLoader.h
        include/camera.h
        include/integrator.h
        include/ray.h
        include/scene.h
        include/image.h
        include/utils.h include/config.h include/config_io.h include/load_obj.h include/geometry.h include/interaction.h include/accel.h)
set(SRC
        src/main.cpp
        src/Brick.cpp
        src/Support.cpp
        src/Construct.cpp
        src/Adaptive.cpp
        src/Gradient.cpp
        src/Opacity_corr.cpp
        src/Space_skipping.cpp
        src/ABR.cpp
        src/Transfer_func.cpp
        src/Render.cpp
        src/VDBLoader.cpp
        src/common.cpp
        src/common.cpp
        src/camera.cpp
        src/integrator.cpp
        src/scene.cpp
        src/image.cpp
#        trash/camera.cpp
#        trash/input.cpp
#        trash/shader.cpp
#        trash/scene.cpp
        src/ray.cpp src/load_obj.cpp src/geometry.cpp src/accel.cpp)

#set (EXTRA_LIBS ${EXTRA_LIBS} gpu)

add_executable(${PROJECT_NAME} ${HEADS} ${SRC})

target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json stb OpenMP::OpenMP_CXX tinyobjloader)
target_link_libraries(${PROJECT_NAME} PRIVATE OpenVDB::openvdb)

#target_link_libraries(${PROJECT_NAME} PRIVATE GLUT::GLUT)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)
target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)
target_link_libraries(${PROJECT_NAME} PRIVATE glad::glad)
target_link_libraries (${PROJECT_NAME} PRIVATE ${EXTRA_LIBS})
