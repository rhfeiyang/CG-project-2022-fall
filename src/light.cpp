#include "light.h"
#include "utils.h"

constexpr uint32_t SAMPLE_NUM = 16;

Light::Light(const Vec3f &pos, const Vec3f &color) :
    position(pos), radiance(color) {}


//SquareAreaLight::SquareAreaLight(const Vec3f &pos, const Vec3f &color, const Vec2f &size) :
//    Light(pos, color), size(size) {
//  Vec3f v1, v2, v3, v4;
//  v1 = pos + Vec3f(size.x() / 2, 0.f, -size.y() / 2);
//  v2 = pos + Vec3f(-size.x() / 2, 0.f, -size.y() / 2);
//  v3 = pos + Vec3f(-size.x() / 2, 0.f, size.y() / 2);
//  v4 = pos + Vec3f(size.x() / 2, 0.f, size.y() / 2);
//  left_behind_pos=v2;
//  light_mesh =TriangleMesh({v1, v2, v3, v4}, {Vec3f(0, -1, 0)}, {0, 1, 2, 0, 2, 3}, {0,0,0,0,0,0});
//}
//
//Vec3f SquareAreaLight::emission(const Vec3f &pos, const Vec3f &dir) const {
//    auto cos_theta=std::max((-dir).dot(Vec3f (0,-1,0)),0.0f);
//  return radiance*cos_theta;
//}
//
//float SquareAreaLight::pdf(const Interaction &interaction, Vec3f pos) const {
//    return 1.0/(size.x()*size.y());
//}
//
//Vec3f SquareAreaLight::sample(Interaction &interaction, float *pdf, Sampler &sampler) const {
//    auto spos=sampler.get2D();
//    spos.x()*=size.x();
//    spos.y()*=size.y();
//    Vec3f pos=left_behind_pos+Vec3f(1,0,0)*spos.x()+Vec3f(0,0,1)*spos.y();
//    interaction.wi=pos-interaction.pos;
//    *pdf=this->pdf(interaction,pos);
//    return pos;
//}
//
//bool SquareAreaLight::intersect(Ray &ray, Interaction &interaction) const {
//  if (light_mesh.intersect(ray, interaction)) {
//    interaction.type = Interaction::Type::LIGHT;
//    return true;
//  }
//  return false;
//}
Vec3f PointLight::emission(const Vec3f &pos, const Vec3f &dir) const {
    return radiance;
}
