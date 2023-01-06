//
// Created by Renhui on 2022/12/19.
//

#ifndef CG_PROJECT_2022_FALL_COMMON_H
#define CG_PROJECT_2022_FALL_COMMON_H
#include <openvdb/openvdb.h>
#include <openvdb/tree/NodeManager.h>
#include <openvdb/version.h>
#include <openvdb/tools/SignedFloodFill.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <openvdb/tools/Interpolation.h>
#include <openvdb/tools/ChangeBackground.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>

using Vec4f=openvdb::Vec4f;
using Vec3f=openvdb::Vec3f;
using Vec3i=openvdb::Vec3i;
using Vec3d=openvdb::Vec3d;
using Vec3R=openvdb::Vec3R;
using Vec2f=openvdb::Vec2f;
using Vec2i=openvdb::Vec2i;
using Mat4f=openvdb::Mat4s;
using Mat3f=openvdb::Mat3s;
using Coord=openvdb::Coord;
using iBBox=openvdb::CoordBBox;
using wBBox=openvdb::BBoxd;
using Vec3sGrid=openvdb::v10_0::Vec3SGrid;
using FloatGrid=openvdb::v10_0::FloatGrid;

using std::cin;
using std::cout;
using std::endl;

///From hw5
using Float = GLfloat; // GLfloat usually equals to float32
constexpr Float zero = Float(0);
constexpr Float one = Float(1);
constexpr float RAY_DEFAULT_MIN = 1e-5;
constexpr float RAY_DEFAULT_MAX = 1e7;
constexpr float PI = 3.141592653579f;
constexpr float INV_PI = 0.31830988618379067154;
constexpr float EPS = 1e-5;
using Quat = glm::qua<Float, glm::defaultp>;
//using Quat = openvdb::v10_0::math::Quats;

using gVec2 = glm::vec<2, Float, glm::defaultp>;
using gVec3 = glm::vec<3, Float, glm::defaultp>;
using gVec4 = glm::vec<4, Float, glm::defaultp>;
using gMat2 = glm::mat<2, 2, Float, glm::defaultp>;
using gMat3 = glm::mat<3, 3, Float, glm::defaultp>;
using gMat4 = glm::mat<4, 4, Float, glm::defaultp>;
//using IVec2 = glm::ivec2;
//using IVec3 = glm::ivec3;
//using UVec2 = glm::uvec2;
//using UVec3 = glm::uvec3;


//template<typename GridType>

//Vec3f Grid_world_pos(openvdb::GridBase::Ptr grid, Coord coord);
std::string GetFilePath(const std::string& target, int depth = 5);

struct Grids_data{
    void addGrid(const FloatGrid::Ptr& grid);
    std::vector<FloatGrid::Ptr> grids;
    std::vector<double> dx;
    double max_dx=std::numeric_limits<double>::min();
    double min_dx=std::numeric_limits<double>::max();
//    Vec3f origin=Vec3f{std::numeric_limits<float>::max(),std::numeric_limits<float>::max(),std::numeric_limits<float>::max()};
    wBBox whole_wbbox;
    std::vector<wBBox> wbboxes;
};

bool floatEqual(const float& first,const float& second);
wBBox ibbTowbb(const FloatGrid::Ptr & grid,const iBBox& ibb);

//Mat4f Mat4g2v(glm::)
#endif //CG_PROJECT_2022_FALL_COMMON_H
