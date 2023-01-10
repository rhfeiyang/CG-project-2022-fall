#ifndef INTEGRATOR_H_
#define INTEGRATOR_H_

#include "camera.h"
#include "scene.h"
#include "Construct.h"
//#include "interaction.h"

class Integrator {
public:
    Integrator(std::shared_ptr<Camera> cam,
               std::shared_ptr<Scene> scene, int spp, Grids_data &gridsData, float iso_value,
               float step_scale = 1);

    void render() const;

    [[nodiscard]] float opacity_transfer(float value) const;

    [[nodiscard]] Vec3f color_transfer(float val) const;

    [[nodiscard]] Vec2f interpolation(Vec3f pos, uint32_t grid_idx_bm, int &finest_grid) const;

    static float opacity_correction(float actual_step, float opacity);

    Vec3f front_to_back(Ray &ray) const;

    [[nodiscard]] float step_Base(Vec3f pos, uint32_t grid_idx_bm) const;

    [[nodiscard]] float step_Base(const int &finest_grid) const;

    Vec3f phongLighting(Interaction &interaction) const;

    void SetColors(std::vector<Vec3f> &c) {
        colors = c;
    }

    void SetPoints(std::vector<float> &p) {
        points = p;
    }

    void setiso_value(float value) {
        iso_value = value;
    }

    void Setstep_scale(float s) {
        step_scale = s;
        adaptive_depth= std::ceil(log2(step_scale / 0.005));
    }

    void Setspp(int s) {
        spp = s;
    }

    [[nodiscard]] int iso_status(const float &value) const;

    bool adaptive_recur(const Vec3f &pos1, const Vec3f &pos2,
                        Vec3f &result_pos, Vec2f &result_value, int &finest_grid, int depth) const;

    bool adaptive_sample(int &status, const Vec3f &pos1, const Vec3f &pos2, Vec3f &result_pos, int &finest_grid,
                         Vec2f &result_value) const;

//    void SetFilter(bool f) {
//        filter = f;
//    }
    std::shared_ptr<Scene> scene;
private:
    std::shared_ptr<Camera> camera;

//    int max_depth;
    int spp;
//    float dist_limit;
    Grids_data &gridsData;
    Kdtree kdtree;
//    float variance;
    float iso_value;
    float step_scale;
    int adaptive_depth;
    std::vector<Vec3f> colors;
    std::vector<float> points;

//    bool filter;
};

#endif //INTEGRATOR_H_
