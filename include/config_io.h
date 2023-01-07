#ifndef CONFIG_IO_H_
#define CONFIG_IO_H_

#define JSON_USE_IMPLICIT_CONVERSIONS 0

#include <nlohmann/json.hpp>
#include "config.h"


NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Config::CamConfig, position, look_at, ref_up, vertical_fov, focal_length);

// add your own bsdf name if needed

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Config,
                                   spp,
                                   image_resolution,
                                   cam_config,
                                   file_path,
                                   iso_value,
                                   var,
                                   step_scale
                                   );

#endif // CONFIG_IO_H_