#ifndef SCENE_H_
#define SCENE_H_

#include <vector>

#include "camera.h"
#include "image.h"
//#include "geometry.h"
//#include "light.h"
//#include "interaction.h"
//#include "config.h"

class Scene {
 public:
  Scene() = default;
//  void addObject(std::shared_ptr<TriangleMesh> &geometry);
//  [[nodiscard]] const std::shared_ptr<Light> &getLight() const;
//  void setLight(const std::shared_ptr<Light> &new_light);
//  bool isShadowed(Ray &shadow_ray);
//  bool intersect(Ray &ray, Interaction &interaction);
 private:
//  std::vector<std::shared_ptr<TriangleMesh>> objects;
//  std::shared_ptr<Light> light;
};

//void initSceneFromConfig(const Config &config, std::shared_ptr<Scene> &scene);

#endif //SCENE_H_
