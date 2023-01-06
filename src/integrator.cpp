#include "integrator.h"
#include "utils.h"
#include <omp.h>
#include "common.h"
#include <utility>
#include <cmath>

#define OMP

Integrator::Integrator(std::shared_ptr<Camera> cam,
                       std::shared_ptr<Scene> scene, int spp, Grids_data &gridsData, float dist_limit, float iso_value,
                       float var)
        : camera(std::move(cam)), scene(std::move(scene)), spp(spp), gridsData(gridsData), dist_limit(dist_limit),
          variance(var), iso_value(iso_value) {}

void Integrator::render() const {
    auto grid = gridsData.grids[0];
    Vec2i resolution = camera->getImage()->getResolution();
    Sampler sampler;
    int cnt = 0;
    std::vector<Vec2f> real_sample_points;

    for (int i = 1; i <= spp; i++) {
        real_sample_points.emplace_back(sampler.get2D());
    }
#ifdef OMP
#pragma omp parallel for schedule(dynamic), shared(cnt), private(sampler)
#endif
    for (int dx = 0; dx < resolution.x(); dx++) {
#ifdef OMP
#pragma omp atomic
#endif
        ++cnt;
        printf("\r%.02f%%", cnt * 100.0 / resolution.x());
        sampler.setSeed(omp_get_thread_num());
        for (int dy = 0; dy < resolution.y(); dy++) {
            Vec3f L(0, 0, 0);
//            auto ray = camera->generateRay(0.5f+dx,0.5f+dy);
//            L += front_to_back(ray, grid->metaValue<double>("dx"));
            for (auto i: real_sample_points) {
                auto ray = camera->generateRay(i.x() + dx, i.y() + dy);
                L += front_to_back(ray, grid->metaValue<double>("dx"));
            }
            L /= spp;
            camera->getImage()->setPixel(dx, dy, L);
        }
    }
}

float Integrator::opacity_transfer(float value) const {
    //first for isovalue of iso-surface, second for value to be the opacity
    //using Gauss pdf
    if (value < 0.065){
        float xu = 0.005f - abs(value - round(value*100)/100);
        return 0.8*exp(-0.5*xu*xu/(0.0025*0.0025));
//        return (0.005f - abs(value - round(value*100)/100))*160;
    }
    return 0;
}

float Integrator::opacity_correction(float step, float opacity) {
    return 1 - pow((1 - opacity), step);
}

Vec3f Integrator::color_transfer(float val) const {
    int b = (int)round(val * 100);
    return{0.0, (36 - b*b) * 0.4f / 36.0f + 0.6f, b * b * 0.4f / 36.0f + 0.6f};
    return {0,0.8, 1.0};
}

template<class ValueT, class TreeT, size_t N>
inline int probeValues(ValueT (&data)[N][N][N], const TreeT &inTree, Coord ijk) {
    int hasActiveValues = 0;
    hasActiveValues += inTree.probeValue(ijk, data[0][0][0]); // i, j, k

    ijk[2] += 1;
    hasActiveValues += inTree.probeValue(ijk, data[0][0][1]); // i, j, k + 1

    ijk[1] += 1;
    hasActiveValues += inTree.probeValue(ijk, data[0][1][1]); // i, j+1, k + 1

    ijk[2] -= 1;
    hasActiveValues += inTree.probeValue(ijk, data[0][1][0]); // i, j+1, k

    ijk[0] += 1;
    ijk[1] -= 1;
    hasActiveValues += inTree.probeValue(ijk, data[1][0][0]); // i+1, j, k

    ijk[2] += 1;
    hasActiveValues += inTree.probeValue(ijk, data[1][0][1]); // i+1, j, k + 1

    ijk[1] += 1;
    hasActiveValues += inTree.probeValue(ijk, data[1][1][1]); // i+1, j+1, k + 1

    ijk[2] -= 1;
    hasActiveValues += inTree.probeValue(ijk, data[1][1][0]); // i+1, j+1, k

    return hasActiveValues;
}

template<class ValueT, size_t N>
inline ValueT trilinearInterpolation(ValueT (&data)[N][N][N], const Vec3R &uvw) {
    auto _interpolate = [](const ValueT &a, const ValueT &b, double weight) {
        OPENVDB_NO_TYPE_CONVERSION_WARNING_BEGIN
        const auto temp = (b - a) * weight;
        OPENVDB_NO_TYPE_CONVERSION_WARNING_END
        return static_cast<ValueT>(a + ValueT(temp));
    };
    return _interpolate(_interpolate(_interpolate(data[0][0][0], data[0][0][1], uvw[2]),
                                     _interpolate(data[0][1][0], data[0][1][1], uvw[2]),
                                     uvw[1]),
                        _interpolate(_interpolate(data[1][0][0], data[1][0][1], uvw[2]),
                                     _interpolate(data[1][1][0], data[1][1][1], uvw[2]),
                                     uvw[1]),
                        uvw[0]);
}

float Integrator::interpolation(Vec3f pos) const {
//    //TODO
//    pos={0.0,0.0,0.05};
    auto grid = gridsData.grids[0];
    FloatGrid::ConstAccessor accessor = grid->getConstAccessor();

    openvdb::tools::GridSampler<FloatGrid::ConstAccessor, openvdb::tools::BoxSampler> sampler(accessor,
                                                                                              grid->transform());
//    cout<<pos<<grid->transform().worldToIndex(pos)<<endl;
//    cout<<grid->getAccessor().getValue({0, 0, 0})<<" "<<
//        grid->getAccessor().getValue({1, 0, 0})<<" "<<
//        grid->getAccessor().getValue({0, 1, 0})<<" "<<
//        grid->getAccessor().getValue({0, 0, 1})<<" "<<
//        grid->getAccessor().getValue({1, 1, 0})<<" "<<
//        grid->getAccessor().getValue({1, 0, 1})<<" "<<
//        grid->getAccessor().getValue({0, 1, 1})<<" "<<
//        grid->getAccessor().getValue({1, 1, 1})<<endl;
//    cout<<sampler.wsSample(pos)<<endl;
//    return sampler.wsSample(pos);
//    openvdb::tools::GridSampler<FloatGrid , openvdb::tools::BoxSampler> sampler(grid->tree(),grid->transform());
    auto world_value=sampler.wsSample(pos);
    return world_value;
}

template<class TreeT>
inline int
sample(const TreeT &inTree, const Vec3R &inCoord,
       typename TreeT::ValueType &result) {
    using ValueT = typename TreeT::ValueType;
    const Vec3i inIdx = openvdb::tools::local_util::floorVec3(inCoord);
    const Vec3R uvw = inCoord - inIdx;
    // Retrieve the values of the eight voxels surrounding the
    // fractional source coordinates.
    ValueT data[2][2][2];
    const int hasActiveValues = probeValues(data, inTree, Coord(inIdx));
    if (hasActiveValues) result = trilinearInterpolation(data, uvw);
    else result = 0;
    return hasActiveValues;
}

///For single-res
Vec3f Integrator::front_to_back(Ray &ray, float step) const {
    auto grid = gridsData.grids[0];
    ray.direction.normalize();
    step /= 4;
//    cout<<ray.direction<<endl;
    Vec3f result{0, 0, 0};
    float T = 1;
    auto temp_pos = ray.origin;
    auto limit = dist_limit;
    while (limit > 0) {
        temp_pos += ray.direction * step;
        limit -= step;
        if (!(gridsData.whole_wbbox.isInside(temp_pos)))
            continue;
        float temp_val = 0;
//        temp_pos = {0.01,0.01,0.01};
        temp_val = interpolation(temp_pos);
//        if (temp_val > 1.0f) cout << temp_val << endl;
//        sample(grid->tree(), grid->worldToIndex(temp_pos), temp_val);
        auto opacity = opacity_correction(step, opacity_transfer(temp_val));
//        if(temp_val>0)
//            cout<<temp_val<<endl;
        result += T * opacity * color_transfer(temp_val);
        T *= (1.0f - opacity);
//        cout<<result<<endl;
    }
//    cout<<cnt<<endl;
//    cout<<T<<" "<<limit<<endl;
    return result;
}


//Some trash files generated during programming

//float Integrator::interpolation(Vec3f pos) const {
//    //TODO
//    pos={0.0,0.0,0.05};
//    auto grid = gridsData.grids[0];
//    FloatGrid::ConstAccessor accessor = grid->getConstAccessor();
//
//    openvdb::tools::GridSampler<FloatGrid::ConstAccessor, openvdb::tools::BoxSampler> sampler(accessor,
//                                                                                              grid->transform());
//    cout<<pos<<grid->transform().worldToIndex(pos)<<endl;
//    cout<<grid->getAccessor().getValue({0, 0, 0})<<" "<<
//        grid->getAccessor().getValue({1, 0, 0})<<" "<<
//        grid->getAccessor().getValue({0, 1, 0})<<" "<<
//        grid->getAccessor().getValue({0, 0, 1})<<" "<<
//        grid->getAccessor().getValue({1, 1, 0})<<" "<<
//        grid->getAccessor().getValue({1, 0, 1})<<" "<<
//        grid->getAccessor().getValue({0, 1, 1})<<" "<<
//        grid->getAccessor().getValue({1, 1, 1})<<endl;
//    cout<<sampler.wsSample(pos)<<endl;
//    return sampler.wsSample(pos);
//    openvdb::tools::GridSampler<FloatGrid , openvdb::tools::BoxSampler> sampler(grid->tree(),grid->transform());
//    auto world_value=sampler.wsSample(pos);
//    return world_value;
//}

//float Integrator::opacity_transfer(float value) const {
    //first for isovalue of iso-surface, second for value to be the opacity
    //using Gauss pdf
//    auto result= 1 / (variance * sqrt(2 * PI)) * exp(-pow((value - isovalue), 2) / (2 * variance * variance));
//    if(abs(value-iso_value)>0.001) return 0;
//    if(value>=0.065 ) return 0;
//    if (value<=EPS) return 0;
//    auto result=  exp(-pow((value - iso_value), 2) / (2 * variance * variance));
//    if(result<0)
//            return 0;
//    else if(result>1)
//        return 1;
//    else
//    return result;
//    return exp(-pow((value - 0.06), 2) / (2 * 0.0015 * 0.0015));

//    if (0.065 < value && value < 0.068) return 0;
//    auto result1=  exp(-pow((value - 0.03), 2) / (2 * 0.01 * 0.01));
//    auto result2=  exp(-pow((value - 0.06), 2) / (2 * 0.002 * 0.002));
//    return std::max(result1, result2);
//    auto result3=  exp(-pow((value - 0.04), 2) / (2 * variance * variance));
//    auto result4=  exp(-pow((value - 0.05), 2) / (2 * variance * variance));
//    auto result5=  exp(-pow((value - 0.06), 2) / (2 * variance * variance));
//    return std::max(std::max(result1, result2), std::max(result3, result4));
//    return std::max(std::max(std::max(result1, result2), std::max(result3, result4)), result5);
//    return  result1;
//
//    if(value>0.01 && value<0.065) return 0.7;
//    else return 0;
//    return 100 * std::max(0.0, (0.005 - abs(value - 0.06)));
//}

//Vec3f Integrator::color_transfer(float val) const {
//    // TODO
//    float v = val / 0.06;
//    float r = fmod(v, 1.0);
//    float g = fmod(v + 0.4, 1.0);
//    float b = fmod(v + 0.8, 1.0);
//    return {r, g, b};
//    return{0.4,0.4,0.6};
//}