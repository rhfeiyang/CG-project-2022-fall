#ifndef INTEGRATOR_H_
#define INTEGRATOR_H_

#include "camera.h"
#include "scene.h"
//#include "interaction.h"

class Integrator {
public:
//    Integrator(std::shared_ptr<Camera> cam,
//               std::shared_ptr<Scene> scene, int spp, int max_depth);
    Integrator(std::shared_ptr<Camera> cam,
                           std::shared_ptr<Scene> scene, int spp, FloatGrid::Ptr &grid, float dist_limit, float var=sqrt(0.0001));

    void render() const;

    float opacity_transfer(float isovalue, float value) const;

    Vec3f color_transfer(float val) const;

    float interpolation(Vec3f pos) const;

    Vec3f front_to_back(Ray &ray, float step, float isovalue) const;


private:
    std::shared_ptr<Camera> camera;
    std::shared_ptr<Scene> scene;
//    int max_depth;
    int spp;
    float dist_limit;
    FloatGrid::Ptr &grid;
    float variance;
};

#endif //INTEGRATOR_H_
