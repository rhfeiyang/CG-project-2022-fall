#ifndef RAY_H_
#define RAY_H_

#include "common.h"

struct Ray {
  /// origin point of ray
  Vec3f origin;
  /// normalized direction of the ray
  Vec3f direction;
  /// min and max distance of the ray
  float t_min;
  float t_max;

  explicit Ray(const Vec3f &o, const Vec3f &dir, float t_min = RAY_DEFAULT_MIN, float t_max = RAY_DEFAULT_MAX)
  : origin(o)
  , direction(dir)
  , t_min(t_min)
  , t_max(t_max) {}

  [[nodiscard]] Vec3f operator()(float t) const {
    return origin + t * direction;
  }
};

// The intersection test for bounding box
// Return true if hit, t_in t_out in result
bool ray_bbox_range(Ray ray, wBBox bbox,std::array<float,2>& result) {
    float dir_frac_x = (ray.direction[0] == 0.0) ? 1.0e32 : 1.0 / ray.direction[0];
    float dir_frac_y = (ray.direction[1] == 0.0) ? 1.0e32 : 1.0 / ray.direction[1];
    float dir_frac_z = (ray.direction[2] == 0.0) ? 1.0e32 : 1.0 / ray.direction[2];

    float tx1 = (bbox.min().x() - ray.origin[0]) * dir_frac_x;
    float tx2 = (bbox.max().x() - ray.origin[0]) * dir_frac_x;
    float ty1 = (bbox.min().y() - ray.origin[1]) * dir_frac_y;
    float ty2 = (bbox.max().y() - ray.origin[1]) * dir_frac_y;
    float tz1 = (bbox.min().z() - ray.origin[2]) * dir_frac_z;
    float tz2 = (bbox.max().z() - ray.origin[2]) * dir_frac_z;

    // t_in and t_out, maybe negative, but will return false
    result[0] = std::max(std::max(std::min(tx1, tx2), std::min(ty1, ty2)), std::min(tz1, tz2));
    result[1] = std::min(std::min(std::max(tx1, tx2), std::max(ty1, ty2)), std::max(tz1, tz2));

    return (result[1] > result[0]) && result[1] > 0;
}

#endif //RAY_H_
