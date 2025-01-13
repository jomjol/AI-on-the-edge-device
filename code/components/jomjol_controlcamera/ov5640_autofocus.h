/*
  Based on work created by Eric Nam, December 08, 2021.
  Released into the public domain.
*/
#pragma once

#ifndef OV5640_AUTOFOCUS_H
#define OV5640_AUTOFOCUS_H

#include "esp_camera.h"

int ov5640_autofocus_init(sensor_t *sensor);
uint8_t ov5640_autofocus_set_mode(sensor_t *sensor, uint8_t mode);
uint8_t ov5640_autofocus_get_status(sensor_t *sensor, uint8_t *S_Zone, int S_Zone_len);
uint8_t ov5640_release_autofocus(sensor_t *sensor);

#endif
