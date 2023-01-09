#ifndef SCENE_H_
#define SCENE_H_

#include "common.h"
#include "camera.h"
#include "image.h"
//#include "geometry.h"
#include "light.h"
//#include "interaction.h"
//#include "config.h"

class Scene {
public:
    Scene() = default;

//    void loadDataFromFile(const std::string &path);
    void addObject(std::shared_ptr<TriangleMesh> &geometry);

//  [[nodiscard]] const std::shared_ptr<Light> &getLight() const;
    void setLight(const std::shared_ptr<Light> &new_light);

    void setAmbient(Vec3f ambient);

    [[nodiscard]] const std::shared_ptr<Light> &getLight() const;

    [[nodiscard]]const Vec3f &getAmbient() const;

    bool isShadowed(Ray &shadow_ray);
    bool intersect(Ray &ray, Interaction &interaction);
    void setObjColor(const Vec3f &color){
        objects[0]->setColor(color);
    }
    void setObjPosition(const Vec3f &pos){
        objects[0]->SetPosition(pos);
    }
    void setObjScale(float s){
        objects[0]->SetScale(s);
    }
    Vec3f GetPosition(){return objects[0]->GetPosition();}
    float GetScale(){return objects[0]->GetScale();}

private:
    std::vector<std::shared_ptr<TriangleMesh>> objects;
    std::shared_ptr<Light> light;
    Vec3f ambient_light;
};

void initSceneFromConfig(const Config &config, std::shared_ptr<Scene> &scene);

#endif //SCENE_H_
