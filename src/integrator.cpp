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
#define OMP
Integrator::Integrator(std::shared_ptr<Camera> cam,
                       std::shared_ptr<Scene> scene, int spp, FloatGrid::Ptr &grid, float dist_limit, float iso_value, float var)
        : camera(std::move(cam)), scene(std::move(scene)), spp(spp), grid(grid), dist_limit(dist_limit), variance(var), iso_value(iso_value) {
}

void Integrator::render() const {
  Vec2i resolution = camera->getImage()->getResolution();
  Sampler sampler;
  int cnt = 0;
  std::vector<Vec2f> real_sample_points;

    for(int i=1;i<=spp;i++){
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
        for (auto i:real_sample_points) {
            auto ray = camera->generateRay(i.x()+dx, i.y()+dy);
            L+= front_to_back(ray,grid->metaValue<double>("dx"));
        }
        L/=spp;
//        cout<<L<<endl;
      camera->getImage()->setPixel(dx, dy, L);
    }
  }
}

float Integrator::opacity_transfer(float value) const {
    //first for isovalue of iso-surface, second for value to be the opacity
    //using Gauss pdf
//    auto result= 1 / (variance * sqrt(2 * PI)) * exp(-pow((value - isovalue), 2) / (2 * variance * variance));
//    if(abs(value-iso_value)>0.001) return 0;
//    if(value>=0.065 ) return 0;
    if (value<=EPS) return 0;
    auto result=  exp(-pow((value - iso_value), 2) / (2 * variance * variance));
    if(result<0)
            return 0;
    else if(result>1)
        return 1;
    else return result;


//    if(value>0.025&&value<0.065) return 0.9;
//    else return 0;

}

float Integrator::opacity_correction(float step, float opacity){
    return 1- pow((1 - opacity),step);
}

Vec3f Integrator::color_transfer(float val) const {
    ///TODO
    float v=val/0.006;
    float r= fmod(v,1.0);
    float g= fmod(v+0.4,1.0);
    float b= fmod(v+0.8,1.0);
    return {r,g,b};
//    return{0.4,0.4,0.6};
}

float Integrator::interpolation(Vec3f pos) const {
    ///TODO
//    pos={0.0,0.0,0.05};
    FloatGrid::ConstAccessor accessor=grid->getConstAccessor();

    openvdb::tools::GridSampler<FloatGrid::ConstAccessor ,openvdb::tools::PointSampler> sampler(accessor,grid->transform());
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
    return sampler.wsSample(pos);
//    openvdb::tools::GridSampler<FloatGrid , openvdb::tools::BoxSampler> sampler(grid->tree(),grid->transform());
//    auto world_value=sampler.wsSample(pos);
//    return world_value;
}

///For single-res
Vec3f Integrator::front_to_back(Ray& ray, float step) const {
    ray.direction.normalize();
//    cout<<ray.direction<<endl;
    Vec3f result{0,0,0};
//    float T=1;
    float O=0;
    auto temp_pos=ray.origin;
    auto limit=dist_limit;
//    int cnt=0;
    while(O<1 && limit>0){

        temp_pos += (ray.direction*step);
        limit-=step;
//        cnt++;
//        if(!grid->getAccessor().isValueOn(Coord(Vec3i (grid->worldToIndex(temp_pos)))))
//            continue;

        auto temp_val=interpolation(temp_pos);
//        if(temp_val>0)
//            cout<<temp_val<<endl;
        auto opacity= opacity_correction(step,opacity_transfer(temp_val));
//        result+=T*opacity* color_transfer(temp_val);
//        T*=(1- opacity);

        result+=((1-O)*opacity*color_transfer(temp_val));
        O+=((1-O)*opacity);

//        cout<<result<<endl;
    }
//    cout<<cnt<<endl;
//    cout<<T<<" "<<limit<<endl;
    return result;
}
