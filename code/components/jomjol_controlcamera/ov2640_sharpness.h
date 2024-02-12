#pragma once

#ifndef OV2640_SHARPNESS_H
#define OV2640_SHARPNESS_H

#include "esp_camera.h"

int ov2640_enable_auto_sharpness(sensor_t *sensor);
int ov2640_set_sharpness(sensor_t *sensor, int sharpness); // -3 to +3, -4 for auto-sharpness

#endif
