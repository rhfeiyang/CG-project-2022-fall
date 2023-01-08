#ifndef INTEGRATOR_H_
#define INTEGRATOR_H_

#include "camera.h"
#include "scene.h"
#include "Construct.h"
//#include "interaction.h"

class Integrator {
public:
    Integrator(std::shared_ptr<Camera> cam,
               std::shared_ptr<Scene> scene, int spp,Grids_data &gridsData,  float iso_value, float var=0.001, float step_scale=1);

    void render() const;

    [[nodiscard]] float opacity_transfer(float value) const;

    [[nodiscard]] static Vec3f color_transfer(float val) ;

    [[nodiscard]] Vec2f interpolation(Vec3f pos, uint32_t grid_idx_bm) const;

    static float opacity_correction(float actual_step, float opacity);

    Vec3f front_to_back(Ray &ray) const;

    [[nodiscard]] float step_Base(Vec3f pos, uint32_t grid_idx_bm) const;
    [[nodiscard]] float step_Base(const int& finest_grid) const;
    Vec3f phoneLighting(Interaction& inter) const;

    void setiso_value(float value){
        iso_value=value;
    }

private:
    std::shared_ptr<Camera> camera;
    std::shared_ptr<Scene> scene;
//    int max_depth;
    int spp;
//    float dist_limit;
    Grids_data &gridsData;
    Kdtree kdtree;
    float variance;
    float iso_value;
    float step_scale;
};

#endif //INTEGRATOR_H_
