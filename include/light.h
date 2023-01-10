#ifndef LIGHT_H_
#define LIGHT_H_

#include <vector>

//#include "core.h"
#include "ray.h"
#include "geometry.h"

class Light {
public:
    Light() = default;

    explicit Light(const Vec3f &pos, const Vec3f &color);

    virtual ~Light() = default;

    [[nodiscard]] virtual Vec3f emission(const Vec3f &pos, const Vec3f &dir) const = 0;
//  virtual float pdf(const Interaction &interaction, Vec3f pos)const = 0;
//  [[nodiscard]] virtual Vec3f sample(Interaction &interaction, float *pdf, Sampler &sampler) const = 0;
//  virtual bool intersect(Ray &ray, Interaction &interaction) const = 0;


    /// position of light in world space
    Vec3f position;
protected:
    /// RGB color of the light
    Vec3f radiance;
};

class PointLight : public Light {
    using Light::Light;

    Vec3f emission(const Vec3f &pos, const Vec3f &dir) const override;
};

#endif //LIGHT_H_
