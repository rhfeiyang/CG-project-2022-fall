#include "integrator.h"
#include "utils.h"
#include <omp.h>
#include "common.h"
#include <utility>
#include <cmath>

#define OMP

Integrator::Integrator(std::shared_ptr<Camera> cam,
                       std::shared_ptr<Scene> scene, int spp, Grids_data &gridsData, float iso_value,
                       float var, float step_scale)
        : camera(std::move(cam)), scene(std::move(scene)), spp(spp), gridsData(gridsData), variance(var),
          iso_value(iso_value),
          kdtree(gridsData), step_scale(step_scale) {}

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
                L += front_to_back(ray);
            }
            L /= spp;
            camera->getImage()->setPixel(dx, dy, L);
        }
    }
}

float Integrator::opacity_transfer(float value) const {
    //first for isovalue of iso-surface, second for value to be the opacity
    //using Gauss pdf
    if (value > 0.01) {
        return 0.6;
//        float xu = value - 1.0;
//        return 0.9 * exp(-0.5*xu*xu/(1.0*1.0));
    }
//    if (0.05 < value && value < 1){
//        float xu = value - 0.5;
//        return 0.9 * exp(-0.5*xu*xu/(0.5*0.5));
//    }

//    if (value > 0.75) return 1;
    return 0;
}

float Integrator::opacity_correction(float actual_step, float opacity) {
    return 1 - pow((1 - opacity), actual_step);
}

Vec3f Integrator::color_transfer(float val) {
    float r = std::min(1.0, std::sqrt(val * val / 4.0));
    float g = std::max(0.0, std::sqrt(2 * val * (2.0 - val) / 4.0));
    float b = std::max(0.0, std::sqrt((val - 2.0) * (val - 2.0) / 4.0));
    return {r, g, b};
}

inline float sample(float dx, const FloatGrid &grid, const Vec3f &pos) {
    const Vec3i inIdx = floorVec3(grid.worldToIndex(pos));
    const Vec3f cell_pos = grid.indexToWorld(inIdx);
//    auto ijk = Coord(inIdx);
    const auto &inTree = grid.tree();
    float sum_weights = 0;
    float sum_weightedValues = 0;
    float temp_val;
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            for (int k = 0; k < 2; k++) {
                // For all 8 points
                auto ijk = Coord(inIdx + Vec3i(i, j, k));
                if (inTree.probeValue(ijk, temp_val)) {
                    auto C_p = cell_pos + dx * Vec3i(i, j, k);
                    float H_hat = std::max(0.0, 1.0 - abs(C_p[0] - pos[0]) / dx)
                                  * std::max(0.0, 1.0 - abs(C_p[1] - pos[1]) / dx)
                                  * std::max(0.0, 1.0 - abs(C_p[2] - pos[2]) / dx);
                    sum_weights += H_hat;
                    sum_weightedValues += H_hat * temp_val;
                }
            }
        }
    }
    return sum_weightedValues / sum_weights;
}

inline Vec3f gradient(float dx, const FloatGrid &grid, const Vec3f &pos) {
    const Vec3i inIdx = floorVec3(grid.worldToIndex(pos));
    const Vec3f cell_pos = grid.indexToWorld(inIdx);
    const auto &inTree = grid.tree();
    float temp_val;
    Vec3f grad(0,0,0);
    for (int axis = 0; axis < 3; axis++) {
        // For gradient on each direction (dx, dy, dz)
        float sum_weights = 0;
        float sum_weightedValues = 0;
        float sum_dH_dxyz = 0;
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 2; j++) {
                for (int k = 0; k < 2; k++) {
                    // For all 8 points
                    auto ijk = Coord(inIdx + Vec3i(i, j, k));
                    if (inTree.probeValue(ijk, temp_val)) {
                        auto C_p = cell_pos + dx * Vec3i(i, j, k);
                        float h[3] = {(float) std::max(0.0, 1.0 - abs(C_p[0] - pos[0]) / dx),
                                      (float) std::max(0.0, 1.0 - abs(C_p[1] - pos[1]) / dx),
                                      (float) std::max(0.0, 1.0 - abs(C_p[2] - pos[2]) / dx)};
                        float H_hat = h[0] * h[1] * h[2];
                        sum_weights += H_hat;
                        sum_weightedValues += H_hat * temp_val;
                        float X_t = C_p[axis] < pos[axis] ? 1 : -1;
                        sum_dH_dxyz += H_hat / h[axis] * X_t / dx;
                    }
                }
            }
        }
        grad[axis] = (sum_weights - sum_weightedValues) * sum_dH_dxyz / (sum_weights * sum_weights);
    }
    return grad;
}


float Integrator::interpolation(Vec3f pos, uint32_t grid_idx_bm) const {
//    //TODO
    float result = 0;
    int cnt = 0;
//    for(auto i:grid_idx){
    for (int i = 0; grid_idx_bm; i++, grid_idx_bm >>= 1) {
        if (grid_idx_bm & 1) {
            auto grid = gridsData.grids[i];
//            FloatGrid::ConstAccessor accessor = grid->getConstAccessor();
//            openvdb::tools::GridSampler
//            <FloatGrid::ConstAccessor, openvdb::tools::BoxSampler> sampler(accessor, grid->transform());
//            float temp_val=sampler.wsSample(pos);
            float temp_val = sample((float)gridsData.dx[i], *grid, pos);
            if(temp_val < 100)
                result = temp_val;
//            float value = sample((float) gridsData.dx[i], *grid, pos);
//            if (value < 1) {
//                result += value;
//                cnt++;
//            }
        }
    }
//    result /= float(cnt);
    return result;
}

float Integrator::step_Base(Vec3f pos, uint32_t grid_idx_bm) const {
    float step = std::numeric_limits<float>::max();
//    for(auto i:grid_idx){
    for (int i = 0; grid_idx_bm; i++, grid_idx_bm >>= 1) {
        if (grid_idx_bm & 1) {
            step = std::min(step, float(gridsData.grids[i]->metaValue<double>("dx")));
        }
    }
    return step;
}

Vec3f interleaved_sampling(Vec3f dt, Vec3f t0) {
    Sampler sampler;
    auto rho = sampler.get1D();
    return dt * (Vec3f(rho) + ceilVec3((t0 + rho * dt) / dt));
}


Vec3f Integrator::front_to_back(Ray &ray) const {
    auto grid = gridsData.grids[0];
    ray.direction.normalize();
//    cout<<ray.direction<<endl;
    Vec3f result{0, 0, 0};
    float T = 1;
//    auto temp_pos = ray.origin;
    std::array<float, 2> t_range{};
    if (!ray_bbox_range(ray, gridsData.whole_wbbox, t_range))
        return result;
    auto limit = t_range[1] - t_range[0];
    ray.origin = ray(t_range[0] + EPS);
    auto contribute_grids_bm = kdtree.grid_contribute(ray.origin);
    float step_base = step_Base(ray.origin, contribute_grids_bm);
    auto actual_step = step_base * step_scale;
    ray.origin = interleaved_sampling(actual_step * ray.direction, ray.origin) - EPS;


    while (T > 0.05 && limit > 0) {
        auto next_pos = ray(actual_step);
        auto sample_pos = (ray.origin + next_pos) / 2;

        auto temp_val = interpolation(sample_pos, contribute_grids_bm);
        auto opacity = opacity_correction(actual_step, opacity_transfer(temp_val));

        result += T * opacity * color_transfer(temp_val);
        T *= (1.0f - opacity);

        ray.origin = next_pos;
        limit -= actual_step;

        contribute_grids_bm = kdtree.grid_contribute(ray.origin);
        step_base = step_Base(ray.origin, contribute_grids_bm);
        actual_step = step_base * step_scale;
//        cout<<result<<endl;
    }

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

//template<class ValueT, size_t N>
//inline ValueT trilinearInterpolation(ValueT (&data)[N][N][N], const Vec3R &uvw) {
//    auto _interpolate = [](const ValueT &a, const ValueT &b, double weight) {
//        OPENVDB_NO_TYPE_CONVERSION_WARNING_BEGIN
//        const auto temp = (b - a) * weight;
//        OPENVDB_NO_TYPE_CONVERSION_WARNING_END
//        return static_cast<ValueT>(a + ValueT(temp));
//    };
//    return _interpolate(_interpolate(_interpolate(data[0][0][0], data[0][0][1], uvw[2]),
//                                     _interpolate(data[0][1][0], data[0][1][1], uvw[2]),
//                                     uvw[1]),
//                        _interpolate(_interpolate(data[1][0][0], data[1][0][1], uvw[2]),
//                                     _interpolate(data[1][1][0], data[1][1][1], uvw[2]),
//                                     uvw[1]),
//                        uvw[0]);
//}

//template<class ValueT, class TreeT, size_t N>
//inline int probeValues(ValueT (&data)[N][N][N], const TreeT &inTree, Coord ijk) {
//    int hasActiveValues = 0;
//    hasActiveValues += inTree.probeValue(ijk, data[0][0][0]); // i, j, k
//
//    ijk[2] += 1;
//    hasActiveValues += inTree.probeValue(ijk, data[0][0][1]); // i, j, k + 1
//
//    ijk[1] += 1;
//    hasActiveValues += inTree.probeValue(ijk, data[0][1][1]); // i, j+1, k + 1
//
//    ijk[2] -= 1;
//    hasActiveValues += inTree.probeValue(ijk, data[0][1][0]); // i, j+1, k
//
//    ijk[0] += 1;
//    ijk[1] -= 1;
//    hasActiveValues += inTree.probeValue(ijk, data[1][0][0]); // i+1, j, k
//
//    ijk[2] += 1;
//    hasActiveValues += inTree.probeValue(ijk, data[1][0][1]); // i+1, j, k + 1
//
//    ijk[1] += 1;
//    hasActiveValues += inTree.probeValue(ijk, data[1][1][1]); // i+1, j+1, k + 1
//
//    ijk[2] -= 1;
//    hasActiveValues += inTree.probeValue(ijk, data[1][1][0]); // i+1, j+1, k
//
//    return hasActiveValues;
//}


// From AMR_basis
//    if (inTree.probeValue(ijk, data[0][0][0])) { // i, j, k
//        float H_hat = Hhat(grid.indexToWorld(ijk), dx, pos);
//        sum_weights += H_hat;
//        sum_weightedValues += H_hat * data[0][0][0];
//    }
//    ijk[2] += 1;
//    if (inTree.probeValue(ijk, data[0][0][1])) { // i, j, k + 1
//        float H_hat = Hhat(grid.indexToWorld(ijk), dx, pos);
//        sum_weights += H_hat;
//        sum_weightedValues += H_hat * data[0][0][1];
//    }
//    ijk[1] += 1;
//    if (inTree.probeValue(ijk, data[0][1][1])) { // i, j + 1, k + 1
//        float H_hat = Hhat(grid.indexToWorld(ijk), dx, pos);
//        sum_weights += H_hat;
//        sum_weightedValues += H_hat * data[0][1][1];
//    }
//    ijk[2] -= 1;
//    if (inTree.probeValue(ijk, data[0][1][0])) { // i, j + 1, k
//        float H_hat = Hhat(grid.indexToWorld(ijk), dx, pos);
//        sum_weights += H_hat;
//        sum_weightedValues += H_hat * data[0][1][0];
//    }
//    ijk[0] += 1;
//    ijk[1] -= 1;
//    if (inTree.probeValue(ijk, data[1][0][0])) { // i + 1, j, k
//        float H_hat = Hhat(grid.indexToWorld(ijk), dx, pos);
//        sum_weights += H_hat;
//        sum_weightedValues += H_hat * data[1][0][0];
//    }
//    ijk[2] += 1;
//    if (inTree.probeValue(ijk, data[1][0][1])) { // i + 1, j, k + 1
//        float H_hat = Hhat(grid.indexToWorld(ijk), dx, pos);
//        sum_weights += H_hat;
//        sum_weightedValues += H_hat * data[1][0][1];
//    }
//    ijk[1] += 1;
//    if (inTree.probeValue(ijk, data[1][1][1])) { // i + 1, j + 1, k + 1
//        float H_hat = Hhat(grid.indexToWorld(ijk), dx, pos);
//        sum_weights += H_hat;
//        sum_weightedValues += H_hat * data[1][1][1];
//    }
//    ijk[2] -= 1;
//    if (inTree.probeValue(ijk, data[1][1][0])) { // i + 1, j + 1, k
//        float H_hat = Hhat(grid.indexToWorld(ijk), dx, pos);
//        sum_weights += H_hat;
//        sum_weightedValues += H_hat * data[1][1][0];
//    }
//    return sum_weightedValues / sum_weights;

//inline float Hhat(Vec3R cp, float dx, Vec3f pos) {
////    return 1;
//    float H_hat = 1;
//    for (int i = 0; i < 3; i++) {
//        H_hat *= std::max(0.0, 1.0 - abs(cp[i] + 0.5 * dx - pos[i]) / dx);
//    }
//    return H_hat;
//}

// From sample
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