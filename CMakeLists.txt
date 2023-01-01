cmake_minimum_required(VERSION 3.22)
project(CG_project_2022_fall)
find_package(OpenVDB CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
set(CMAKE_CXX_STANDARD 17)
include_directories(include)
include_directories(src)

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
        include/VDBLoader.hpp
        )
set(SRC
        src/main.cpp
        src/VDBLoader.cpp
        src/Brick.cpp
        src/Support.cpp
        src/Construct.cpp
        src/Adaptive.cpp
        src/Gradient.cpp
        src/Opacity_corr.cpp
        src/Space_skipping.cpp
        src/ABR.cpp
        include/Transfer_func.cpp
        include/Render.cpp)

add_executable(${PROJECT_NAME} ${HEADS} ${SRC})

target_link_libraries(${PROJECT_NAME} PRIVATE OpenVDB::openvdb)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)
target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)
target_link_libraries(${PROJECT_NAME} PRIVATE glad::glad)