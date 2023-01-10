#include "scene.h"
#include "load_obj.h"


void Scene::addObject(std::shared_ptr<TriangleMesh> &mesh) {
    objects.push_back(mesh);
}

bool Scene::isShadowed(Ray &shadow_ray) {
    Interaction in;
    return intersect(shadow_ray, in) && in.type == Interaction::Type::GEOMETRY;
}

void Scene::setLight(const std::shared_ptr<Light> &new_light) {
    light = new_light;
}


bool Scene::intersect(Ray &ray, Interaction &interaction) {
//  light->intersect(ray, interaction);
    for (const auto &obj: objects) {
        Interaction cur_it;
        if (obj->intersect(ray, cur_it) && (cur_it.dist < interaction.dist)) {
            interaction = cur_it;
        }
    }
    return interaction.type != Interaction::Type::NONE;
}

const std::shared_ptr<Light> &Scene::getLight() const {
    return light;
}

void Scene::setAmbient(Vec3f ambient) {
    ambient_light = ambient;
}

const Vec3f &Scene::getAmbient() const {
    return ambient_light;
}

//
void initSceneFromConfig(const Config &config, std::shared_ptr<Scene> &scene) {
//  // add square light to scene.
//  std::shared_ptr<Light> light = std::make_shared<SquareAreaLight>(Vec3f(config.light_config.position),
//                                                                   Vec3f(config.light_config.radiance),
//                                                                   Vec2f(config.light_config.size));
//  scene->setLight(light);
    std::shared_ptr<Light> light = std::make_shared<PointLight>(Vec3f(config.light_config.position),
                                                                Vec3f(config.light_config.radiance));
    scene->setLight(light);
//  // init all materials.
    std::map<std::string, Vec3f> mat_list;
    for (const auto &mat: config.materials) {
        mat_list[mat.name] = Vec3f(mat.color);
    }
    // add mesh objects to scene. Translation and scaling are directly applied to vertex coordinates.
    // then set corresponding material by name.
    std::cout << "loading obj files..." << std::endl;
    for (auto &object: config.objects) {
        cout << object.scale << endl;
        auto mesh_obj = makeMeshObject(object.path, Vec3f(object.position), object.scale);
//    mesh_obj->setMaterial(mat_list[object.material_name]);
        mesh_obj->setColor(mat_list[object.material_name]);
        if (object.has_bvh) {
            mesh_obj->buildBVH();
        }
        scene->addObject(mesh_obj);
    }
}