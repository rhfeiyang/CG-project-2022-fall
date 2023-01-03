#include "integrator.h"
#include "utils.h"
#include <omp.h>
#include "common.h"
#include <utility>
//#include <iostream>
//#include <chrono>
//Integrator::Integrator(std::shared_ptr<Camera> cam,
//                       std::shared_ptr<Scene> scene, int spp, int max_depth)
//    : camera(std::move(cam)), scene(std::move(scene)), spp(spp), max_depth(max_depth) {
//}
Integrator::Integrator(std::shared_ptr<Camera> cam,
                       std::shared_ptr<Scene> scene, int spp, FloatGrid::Ptr &grid, float dist_limit, float var)
        : camera(std::move(cam)), scene(std::move(scene)), spp(spp), grid(grid), dist_limit(dist_limit),variance(var) {
}

void Integrator::render() const {
  Vec2i resolution = camera->getImage()->getResolution();
  Sampler sampler;
  int cnt = 0;
  std::vector<Vec2f> real_sample_points;

    for(int i=1;i<=spp;i++){
        real_sample_points.emplace_back(sampler.get2D());
    }
#pragma omp parallel for schedule(dynamic), shared(cnt), private(sampler)
  for (int dx = 0; dx < resolution.x(); dx++) {
#pragma omp atomic
    ++cnt;
    printf("\r%.02f%%", cnt * 100.0 / resolution.x());
    sampler.setSeed(omp_get_thread_num());

    for (int dy = 0; dy < resolution.y(); dy++) {

      Vec3f L(0, 0, 0);
      // TODO: generate #spp rays for each pixel and use Monte Carlo integration to compute radiance.
        for (auto i:real_sample_points) {
            auto ray = camera->generateRay(i.x()+dx, i.y()+dy);
            L+= front_to_back(ray,grid->metaValue<double>("dx"),0.065);
        }
        L/=spp;
      camera->getImage()->setPixel(dx, dy, L);
    }
  }
}

float Integrator::opacity_transfer(float isovalue, float value) const {
    //first for isovalue of iso-surface, second for value to be the opacity
    //using Gauss pdf
//    auto result= 1 / (variance * sqrt(2 * PI)) * exp(-pow((value - isovalue), 2) / (2 * variance * variance));
    auto result= exp(-pow((value - isovalue), 2) / (2 * variance * variance));
    if(result<0.001)
            return 0;
        else if(result>1)
            return 1;
        else return result;
}

Vec3f Integrator::color_transfer(float val) const {
    ///TODO
    return {0,0,0.5};
}

float Integrator::interpolation(Vec3f pos) const {
    ///TODO
    FloatGrid::ConstAccessor accessor=grid->getConstAccessor();
    openvdb::tools::GridSampler<FloatGrid::ConstAccessor ,openvdb::tools::BoxSampler> sampler(accessor,grid->transform());
    ///untested
    return sampler.wsSample(pos);
}

///For single-res
Vec3f Integrator::front_to_back(Ray& ray, float step, float isovalue) const {
    ray.direction.normalize();
    Vec3f result{0,0,0};
    float T=1;
    auto temp_pos=ray.origin;
    auto limit=dist_limit;
    while(T>=0.05 && limit>0){
        temp_pos += ray.direction*step;
        limit-=step;
        if(!grid->getAccessor().isValueOn(Coord(Vec3i (grid->worldToIndex(temp_pos)))))
            continue;
        auto temp_val=interpolation(temp_pos);
//        if(temp_val>0)
//            cout<<temp_val<<endl;
        auto opacity=opacity_transfer(isovalue,temp_val);
        T*=(1- opacity);
        result+=T*opacity* color_transfer(temp_val);
    }
//    cout<<T<<" "<<limit<<endl;
    return result;
}
