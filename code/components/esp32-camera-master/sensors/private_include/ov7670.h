/*
 * This file is part of the OpenMV project.
 * author: Juan Schiavoni <juanjoseschiavoni@hotmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * OV7670 driver.
 *
 */
#ifndef __OV7670_H__
#define __OV7670_H__
#include "sensor.h"

int ov7670_init(sensor_t *sensor);
#endif // __OV7670_H__
