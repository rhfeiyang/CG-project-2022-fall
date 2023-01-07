////
//// Created by Renhui on 2023/1/7.
////
//#include <cuda_runtime.h>
//#include <cuda.h>
//
//
//#include "camera.h"
//#include "scene.h"
//#include "Construct.h"
////#include "interaction.h"
//
//class Integrate {
//public:
//    Integrate(cuda::shared_ptr<Camera> cam,
//               cuda::shared_ptr<Scene> scene, int spp,Grids_data &gridsData,  float iso_value, float var=0.001, float step_scale=1){
//
//    }
//
//    void render() const{
//
//    }
//
//    [[nodiscard]] float opacity_transfer(float value) const{
//
//    }
//
//    [[nodiscard]] static Vec3f color_transfer(float val) {
//
//    }
//
//    [[nodiscard]] float interpolation(Vec3f pos, uint32_t grid_idx_bm) const{
//
//    }
//
//    static float opacity_correction(float actual_step, float step_base, float opacity){
//
//    }
//
//    Vec3f front_to_back(Ray &ray) const{
//
//    }
//
//    [[nodiscard]] float step_Base(Vec3f pos, uint32_t grid_idx_bm) const{
//
//    }
//
//private:
//    cuda::shared_ptr<Camera> camera;
//    cuda::shared_ptr<Scene> scene;
////    int max_depth;
//    int spp;
////    float dist_limit;
//    Grids_data &gridsData;
//    Kdtree kdtree;
//    float variance;
//    float iso_value;
//    float step_scale;
//};