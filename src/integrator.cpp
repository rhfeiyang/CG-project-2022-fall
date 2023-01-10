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
            for(int s=0;s<spp;++s){
                auto i=sampler.get2D();
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
//    if (0.009 < value && value < 0.03) {
//        // 0.006 - 0.01: Gauss
//        // 0.01 - 0.02: 1
//        // 0.02 - 0.03: Gauss
//        return std::min(1.0, 1.6 * exp(-0.5 * (value - 0.015) * (value - 0.015) / (0.005 * 0.005)));
//    }
    if (iso_value - 0.008 < value && value < iso_value + 0.008)
        return std::min(1.0, 1.6 * exp(-0.5 * (value - iso_value) * (value - iso_value) / (0.004 * 0.004)));
    return 0;
}

float Integrator::opacity_correction(float actual_step, float opacity) {
    return 1 - pow((1 - opacity), actual_step);
}

Vec3f Integrator::color_transfer(float val)const {
    /*Vec3f r = Vec3f{1, 0.05, 0.05} * 0.8;
    Vec3f g = Vec3f{0.05, 1, 0.05} * 0.8;
    Vec3f b = Vec3f{0.05, 0.05, 1} * 0.8;
    if (val < 0.015) {
        return b;
    }
    else if (val < 0.035) {
        return (0.035 - val) / 0.02 * b + (val - 0.015) / 0.02 * g;
    }
    else if (val < 0.045) {
        return g;
    }
    else if (val < 0.065) {
        return (0.065 - val) / 0.02 * g + (val - 0.045) / 0.02 * r;
    }
    else {
        return r;
    }*/

    for (int i = 0; i < colors.size(); ++i) {
        // 1、对于color前后0.005，即共0.01的区间内，都为这个点的颜色 i.e. 0.4 -> 0.35~0.45
        // 2、对于两个区间之间的部分，进行插值，注意分母 i.e. 0.04&0.08 -> 0.045~0.075区间需要插值 -> 分母是0.075-0.045=0.03
        // 3、对于小于最小点or大于最大点区间的部分，按照最小点or最大点
        if (i == 0 && val < points[i] + 0.0025) {
            return colors[i];
        } else if (i == colors.size() - 1 && val > points[i] - 0.0025) {
            return colors[i];
        } else if (points[i] - 0.0025 < val && val < points[i] + 0.0025) {
            return colors[i];
        } else if (points[i - 1] + 0.0025 < val && val < points[i] - 0.0025) {
            return (((points[i] - 0.0025) - val) * colors[i - 1] +
                    (val - (points[i - 1] + 0.0025)) * colors[i]) / (points[i] - points[i - 1]);
        }
    }
    return {0,0,0};
}

inline Vec2f sample(float dx, const Vec3sGrid &grid, const Vec3f &pos) {
    const Vec3i inIdx = floorVec3(grid.worldToIndex(pos));
    const Vec3f cell_pos = grid.indexToWorld(inIdx);
//    auto ijk = Coord(inIdx);
    const auto &inTree = grid.tree();
    float sum_weights = 0;
    float sum_weightednorm = 0;
    float sum_weightedq = 0;
    Vec3f temp_val;
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
                    sum_weightednorm += H_hat * temp_val[0];
                    sum_weightedq += H_hat * temp_val[1];
                }
            }
        }
    }
    return {sum_weightednorm / sum_weights, sum_weightedq / sum_weights};
}

inline bool gradient(float dx, const Vec3sGrid &grid, const Vec3f &pos, Vec3f & result) {
    const Vec3i inIdx = floorVec3(grid.worldToIndex(pos));
    const Vec3f cell_pos = grid.indexToWorld(inIdx);
    const auto &inTree = grid.tree();
    Vec3f temp_val;
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
                        sum_weightedValues += H_hat * temp_val[1];
                        float X_t = C_p[axis] < pos[axis] ? 1 : -1;
                        sum_dH_dxyz += H_hat / h[axis] * X_t / dx;
                    }
                }
            }
        }
        grad[axis] = (sum_weights - sum_weightedValues) * sum_dH_dxyz / (sum_weights * sum_weights);
    }
    if(grad.lengthSqr()>0){
        result=grad.unit();
        return true;
    }
    else return false;
}


Vec2f Integrator::interpolation(Vec3f pos, uint32_t grid_idx_bm, int& finest_grid) const {
//    //TODO
    Vec2f result = {0, 0};
    for (int i = 0; grid_idx_bm; i++, grid_idx_bm >>= 1) {
        if (grid_idx_bm & 1) {
            auto grid = gridsData.grids[i];
//            FloatGrid::ConstAccessor accessor = grid->getConstAccessor();
//            openvdb::tools::GridSampler
//            <FloatGrid::ConstAccessor, openvdb::tools::BoxSampler> sampler(accessor, grid->transform());
//            float temp_val=sampler.wsSample(pos);
            Vec2f temp_val = sample((float)gridsData.dx[i], *grid, pos);
//            cout<<temp_val<<endl;
            if(temp_val[0] < 100) {
                result = temp_val;
                finest_grid=i;
            }
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

float Integrator::step_Base(const int& finest_grid) const {
    return float(gridsData.grids[finest_grid]->metaValue<double>("dx"));
}

Vec3f interleaved_sampling(Vec3f dt, Vec3f t0) {
    Sampler sampler;
    auto rho = sampler.get1D();
    return dt * (Vec3f(rho) + ceilVec3((t0 + rho * dt) / dt));
}

Vec3f Integrator::phongLighting(Interaction &interaction) const {
    Vec3f radiance;
    auto & light=scene->getLight();

    Vec3f diffuse={0.0f,0.0f,0.0f};
    Vec3f specular={0.0f,0.0f,0.0f};
    Vec3f ambient=scene->getAmbient()*interaction.color;

    auto light_dir=(light->position-interaction.pos).unit();
    Ray ray{interaction.pos,light_dir};
    if(!scene->isShadowed(ray)) {
        //diffuse
        diffuse+=Vec3f (std::max(light_dir.dot(interaction.normal),0.0f));
        //specular
        auto reflectdir=2*(light_dir.dot(interaction.normal))*interaction.normal-light_dir;
        auto viewdir=(camera->getPosition()-interaction.pos).unit();
        specular+=Vec3f(pow(std::max(double(viewdir.dot(reflectdir)), 0.0),16.0));
    }
    diffuse=diffuse*interaction.color;
    specular=specular*interaction.color;
//    cout<<diffuse<<" "<<specular<<" "<<ambient<<endl;
    radiance=(diffuse+specular)*(light->emission(ray.origin,ray.direction))+ambient;
    return radiance;
}

int Integrator::iso_status(const float & value) const {
    ///status: 0: just in  1: smaller 2: larger
    int sample_status;
    if(abs(value-iso_value)<=0.004)
        sample_status=0;
    else if(value<iso_value-0.004)
        sample_status=1;
    else if(value>iso_value+0.004)
        sample_status=2;
    else
        return 1;
    return sample_status;
}

///define: pos1: smaller pos2:larger
bool Integrator::adaptive_recur(const Vec3f& pos1, const Vec3f& pos2,
                                Vec3f& result_pos, Vec2f& result_value, int& finest_grid, int depth) const {
    if(depth> log2(step_scale/0.005)) return false;
    auto sample_pos=(pos1+pos2)/2;
    auto contribute_grid=kdtree.grid_contribute(sample_pos);
//    int finest_grid;
    int f_g;
    auto temp_val= interpolation(sample_pos,contribute_grid,f_g);
    switch (iso_status(temp_val[1])) {
        case 0:{
            result_pos=sample_pos;
            result_value=temp_val;
            finest_grid=f_g;
            return true;
        }
        case 1:{
            return adaptive_recur(sample_pos, pos2, result_pos, result_value, finest_grid, depth + 1);
        }
        case 2:{
            return adaptive_recur(pos1, sample_pos, result_pos, result_value, finest_grid, depth + 1);
        }
    }
    return false;
}

bool Integrator::adaptive_sample(int &status, const Vec3f &pos1, const Vec3f &pos2, Vec3f &result_pos, int &finest_grid,
                                 Vec2f &result_value) const {
    ///Only when last status=1, pos2 larger or last status=2, pos2 smaller, use it
    if(status==1){
        return adaptive_recur(pos1, pos2, result_pos, result_value, finest_grid, 0);
    }
    else if(status==2){
        return adaptive_recur(pos2, pos1, result_pos, result_value, finest_grid, 0);
    }
    else{
        printf("adaptive_sample w\n");
        return false;
    }
}

Vec3f Integrator::front_to_back(Ray &ray) const {
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
    Interaction interaction;
    bool path_has_obj= true;
    if(! scene->intersect(ray,interaction))
        path_has_obj= false;

    ///Adaptive sampling
    contribute_grids_bm = kdtree.grid_contribute(ray.origin);
    int finest_grid_idx = gridsData.grids.size()-1;
    auto temp_val=interpolation(ray.origin, contribute_grids_bm,finest_grid_idx);
    int last_sample_status=iso_status(temp_val[1]);

    while (T > 0.05 && limit > 0) {
        auto next_pos = ray(actual_step);
        auto sample_pos = (ray.origin + next_pos) / 2;

        contribute_grids_bm = kdtree.grid_contribute(sample_pos);
        int finest_grid_idx = gridsData.grids.size()-1;
        auto temp_val = interpolation(sample_pos, contribute_grids_bm,finest_grid_idx);
        int sample_status=iso_status(temp_val[1]);

        if(sample_status!=0 && last_sample_status!=0 &&sample_status!=last_sample_status){
            if(adaptive_sample(last_sample_status, ray.origin, sample_pos, sample_pos,
                               finest_grid_idx, temp_val)){
                sample_status=0;
            }
        }
        actual_step=(sample_pos-ray.origin).length();
        if(sample_status==0){
            /// temp = {norm, q};
//            cout<<actual_step<<endl;
            auto opacity = opacity_correction(actual_step, opacity_transfer(temp_val[1]));
            const float self_emission_rate=0.8;
            if(opacity>0.005){
                Vec3f grad;
//            cout<<temp_val[0]<<endl;
                auto color=opacity*color_transfer(temp_val[0]);
                if(gradient(step_base,*gridsData.grids[finest_grid_idx],sample_pos,grad)){
                    Interaction inter{sample_pos,1,grad,color};
                    color=self_emission_rate*color+ phongLighting(inter);
                    result+=T*color;
                }
                else result += T  * self_emission_rate* color;
                T *= (1.0f - opacity);
            }
        }
        if(path_has_obj){
            if(interaction.dist< EPS){
                result+= T * phongLighting(interaction);
                break;
            }
            interaction.dist-=actual_step;
        }
        last_sample_status=sample_status;
        ray.origin = sample_pos;
        limit -= actual_step;

        step_base = step_Base(finest_grid_idx);

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