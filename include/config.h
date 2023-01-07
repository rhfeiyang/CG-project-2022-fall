#ifndef CONFIG_H
#define CONFIG_H

//#include "core.h"
#include "common.h"

struct Config {
  struct CamConfig {
    float position[3];
    float look_at[3];
    float ref_up[3];
    float vertical_fov;
    float focal_length;
  };

  //   RenderConfig render_config;
  int spp;
  int image_resolution[2];
  CamConfig cam_config;
  std::string file_path;
  float iso_value;
  float var;
  float step_scale;
};

#endif // CONFIG_H