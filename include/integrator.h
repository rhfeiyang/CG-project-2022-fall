#ifndef INTEGRATOR_H_
#define INTEGRATOR_H_

#include "camera.h"
#include "scene.h"
//#include "interaction.h"

class Integrator {
public:
    Integrator(std::shared_ptr<Camera> cam,
               std::shared_ptr<Scene> scene, int spp,Grids_data &gridsData, float dist_limit, float iso_value, float var=sqrt(0.0000001));

    void render() const;

    float opacity_transfer(float value) const;

    Vec3f color_transfer(float val) const;

    float interpolation(Vec3f pos) const;

    static float opacity_correction(float step, float opacity);

    Vec3f front_to_back(Ray &ray, float step) const;


private:
    std::shared_ptr<Camera> camera;
    std::shared_ptr<Scene> scene;
//    int max_depth;
    int spp;
    float dist_limit;
    Grids_data &gridsData;
    float variance;
    float iso_value;

//    bool isNearvalueon(Vec3f pos) const;
};

#endif //INTEGRATOR_H_
