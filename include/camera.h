#ifndef CAMERA_H_
#define CAMERA_H_

//#include "core.h"
#include "common.h"
#include "ray.h"
#include "image.h"
#include "config.h"
enum class Camera_Movement
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN,
};
class Camera {
 public:
  Camera();
  explicit Camera(const Config::CamConfig &config, std::shared_ptr<ImageRGB>& img);

  Ray generateRay(float x, float y);
  void lookAt(const Vec3f &look_at, const Vec3f &ref_up = {0, 1, 0});

  void setPosition(const Vec3f &pos);
  [[nodiscard]] Vec3f getPosition() const;
  void setFov(float new_fov);
  [[nodiscard]] float getFov() const;
  void setImage(std::shared_ptr<ImageRGB> &img);
  [[nodiscard]] std::shared_ptr<ImageRGB> &getImage();
    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        float velocity = 1.0f* deltaTime;
        switch (direction)
        {
            case Camera_Movement::FORWARD:
                position += forward * velocity;
                break;
            case Camera_Movement::BACKWARD:
                position -= forward * velocity;
                break;
            case Camera_Movement::LEFT:
                position -= right * velocity;
                break;
            case Camera_Movement::RIGHT:
                position += right * velocity;
                break;
            case Camera_Movement::UP:
                position += up * velocity;
                break;
            case Camera_Movement::DOWN:
                position -= up * velocity;
                break;
            default:
                assert("direction error");
                break;
        }
    }
 private:
  Vec3f position;
  Vec3f forward;
  Vec3f up;
  Vec3f right;
  float focal_len;
  float fov;

  std::shared_ptr<ImageRGB> image;
};

#endif // CAMERA_H_
