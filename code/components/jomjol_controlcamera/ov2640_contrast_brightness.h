#pragma once

#ifndef OV2640_CONTRAST_BRIGHTNESS_H
#define OV2640_CONTRAST_BRIGHTNESS_H

#include "esp_camera.h"

int ov2640_set_contrast_brightness(sensor_t *sensor, int _contrast, int _brightness);

#endif
