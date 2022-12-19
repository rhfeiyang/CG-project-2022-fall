cmake_minimum_required(VERSION 3.22)
project(CG_project_2022_fall)
find_package(OpenVDB CONFIG REQUIRED)

set(CMAKE_CXX_STANDARD 17)

add_executable(${PROJECT_NAME}
        main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE OpenVDB::openvdb)