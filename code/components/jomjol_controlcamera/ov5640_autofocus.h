/*
  Based on work created by Eric Nam, December 08, 2021.
  Released into the public domain.
*/
#pragma once

#ifndef OV5640_AUTOFOCUS_H
#define OV5640_AUTOFOCUS_H

#include "esp_camera.h"

int ov5640_autofocus_init(sensor_t *sensor);
int ov5640_autofocus_set_mode(sensor_t *sensor, uint8_t mode);
int ov5640_autofocus_get_status(sensor_t *sensor, uint8_t *S_Zone, int S_Zone_len);
int ov5640_autofocus_release(sensor_t *sensor);
int ov5640_manual_focus_set(sensor_t *sensor, uint16_t focusLevel);
int ov5640_manual_focus_release(sensor_t *sensor);
uint16_t ov5640_get_focus_level(sensor_t *sensor);
void ov5640_print_vcm_registers(sensor_t *sensor);

#endif
