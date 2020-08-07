/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * SCCB (I2C like) driver.
 *
 */
#include <stdbool.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "sccb.h"
#include <stdio.h>
#include "sdkconfig.h"
#if defined(ARDUINO_ARCH_ESP32) && defined(CONFIG_ARDUHAL_ESP_LOG)
#include "esp32-hal-log.h"
#else
#include "esp_log.h"
static const char* TAG = "sccb";
#endif

//#undef CONFIG_SCCB_HARDWARE_I2C

#define LITTLETOBIG(x)          ((x<<8)|(x>>8))

#ifdef CONFIG_SCCB_HARDWARE_I2C
#include "driver/i2c.h"

#define SCCB_FREQ               100000           /*!< I2C master frequency*/
#define WRITE_BIT               I2C_MASTER_WRITE /*!< I2C master write */
#define READ_BIT                I2C_MASTER_READ  /*!< I2C master read */
#define ACK_CHECK_EN            0x1              /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS           0x0              /*!< I2C master will not check ack from slave */
#define ACK_VAL                 0x0              /*!< I2C ack value */
#define NACK_VAL                0x1              /*!< I2C nack value */
#if CONFIG_SCCB_HARDWARE_I2C_PORT1
const int SCCB_I2C_PORT         = 1;
#else
const int SCCB_I2C_PORT         = 0;
#endif
static uint8_t ESP_SLAVE_ADDR   = 0x3c;
#else
#include "twi.h"
#endif

int SCCB_Init(int pin_sda, int pin_scl)
{
    ESP_LOGI(TAG, "pin_sda %d pin_scl %d\n", pin_sda, pin_scl);
#ifdef CONFIG_SCCB_HARDWARE_I2C
    //log_i("SCCB_Init start");
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = pin_sda;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = pin_scl;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = SCCB_FREQ;

    i2c_param_config(SCCB_I2C_PORT, &conf);
    i2c_driver_install(SCCB_I2C_PORT, conf.mode, 0, 0, 0);
#else
    twi_init(pin_sda, pin_scl);
#endif
    return 0;
}

uint8_t SCCB_Probe()
{
#ifdef CONFIG_SCCB_HARDWARE_I2C
    uint8_t slave_addr = 0x0;
    while(slave_addr < 0x7f) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, ( slave_addr << 1 ) | WRITE_BIT, ACK_CHECK_EN);
        i2c_master_stop(cmd);
        esp_err_t ret = i2c_master_cmd_begin(SCCB_I2C_PORT, cmd, 1000 / portTICK_RATE_MS);
        i2c_cmd_link_delete(cmd);
        if( ret == ESP_OK) {
            ESP_SLAVE_ADDR = slave_addr;
            return ESP_SLAVE_ADDR;
        }
        slave_addr++;
    }
    return ESP_SLAVE_ADDR;
#else
    uint8_t reg = 0x00;
    uint8_t slv_addr = 0x00;

    ESP_LOGI(TAG, "SCCB_Probe start");
    for (uint8_t i = 0; i < 127; i++) {
        if (twi_writeTo(i, &reg, 1, true) == 0) {
            slv_addr = i;
            break;
        }

        if (i!=126) {
            vTaskDelay(10 / portTICK_PERIOD_MS); // Necessary for OV7725 camera (not for OV2640).
        }
    }
    return slv_addr;
#endif
}

uint8_t SCCB_Read(uint8_t slv_addr, uint8_t reg)
{
#ifdef CONFIG_SCCB_HARDWARE_I2C
    uint8_t data=0;
    esp_err_t ret = ESP_FAIL;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, ( slv_addr << 1 ) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(SCCB_I2C_PORT, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    if(ret != ESP_OK) return -1;
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, ( slv_addr << 1 ) | READ_BIT, ACK_CHECK_EN);
    i2c_master_read_byte(cmd, &data, NACK_VAL);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(SCCB_I2C_PORT, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    if(ret != ESP_OK) {
        ESP_LOGE(TAG, "SCCB_Read Failed addr:0x%02x, reg:0x%02x, data:0x%02x, ret:%d", slv_addr, reg, data, ret);
    }
    return data;
#else
    uint8_t data=0;

    int rc = twi_writeTo(slv_addr, &reg, 1, true);
    if (rc != 0) {
        data = 0xff;
    } else {
        rc = twi_readFrom(slv_addr, &data, 1, true);
        if (rc != 0) {
            data=0xFF;
        }
    }
    if (rc != 0) {
        ESP_LOGE(TAG, "SCCB_Read [%02x] failed rc=%d\n", reg, rc);
    }
    return data;
#endif
}

uint8_t SCCB_Write(uint8_t slv_addr, uint8_t reg, uint8_t data)
{
#ifdef CONFIG_SCCB_HARDWARE_I2C
    esp_err_t ret = ESP_FAIL;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, ( slv_addr << 1 ) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, data, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(SCCB_I2C_PORT, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    if(ret != ESP_OK) {
        ESP_LOGE(TAG, "SCCB_Write Failed addr:0x%02x, reg:0x%02x, data:0x%02x, ret:%d", slv_addr, reg, data, ret);
    }
    return ret == ESP_OK ? 0 : -1;
#else
    uint8_t ret=0;
    uint8_t buf[] = {reg, data};

    if(twi_writeTo(slv_addr, buf, 2, true) != 0) {
        ret=0xFF;
    }
    if (ret != 0) {
        ESP_LOGE(TAG, "SCCB_Write [%02x]=%02x failed\n", reg, data);
    }
    return ret;
#endif
}

uint8_t SCCB_Read16(uint8_t slv_addr, uint16_t reg)
{
#ifdef CONFIG_SCCB_HARDWARE_I2C
    uint8_t data=0;
    esp_err_t ret = ESP_FAIL;
    uint16_t reg_htons = LITTLETOBIG(reg);
    uint8_t *reg_u8 = (uint8_t *)&reg_htons;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, ( slv_addr << 1 ) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg_u8[0], ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg_u8[1], ACK_CHECK_EN);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(SCCB_I2C_PORT, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    if(ret != ESP_OK) return -1;
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, ( slv_addr << 1 ) | READ_BIT, ACK_CHECK_EN);
    i2c_master_read_byte(cmd, &data, NACK_VAL);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(SCCB_I2C_PORT, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    if(ret != ESP_OK) {
        ESP_LOGE(TAG, "W [%04x]=%02x fail\n", reg, data);
    }
    return data;
#else
    uint8_t data=0;
    uint16_t reg_htons = LITTLETOBIG(reg);
    uint8_t *reg_u8 = (uint8_t *)&reg_htons;
    uint8_t buf[] = {reg_u8[0], reg_u8[1]};

    int rc = twi_writeTo(slv_addr, buf, 2, true);
    if (rc != 0) {
        data = 0xff;
    } else {
        rc = twi_readFrom(slv_addr, &data, 1, true);
        if (rc != 0) {
            data=0xFF;
        }
    }
    if (rc != 0) {
        ESP_LOGE(TAG, "R [%04x] fail rc=%d\n", reg, rc);
    }
    return data;
#endif
}

uint8_t SCCB_Write16(uint8_t slv_addr, uint16_t reg, uint8_t data)
{
    static uint16_t i = 0;
#ifdef CONFIG_SCCB_HARDWARE_I2C
    esp_err_t ret = ESP_FAIL;
    uint16_t reg_htons = LITTLETOBIG(reg);
    uint8_t *reg_u8 = (uint8_t *)&reg_htons;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, ( slv_addr << 1 ) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg_u8[0], ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg_u8[1], ACK_CHECK_EN);
    i2c_master_write_byte(cmd, data, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(SCCB_I2C_PORT, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    if(ret != ESP_OK) {
        ESP_LOGE(TAG, "W [%04x]=%02x %d fail\n", reg, data, i++);
    }
    return ret == ESP_OK ? 0 : -1;
#else
    uint8_t ret=0;
    uint16_t reg_htons = LITTLETOBIG(reg);
    uint8_t *reg_u8 = (uint8_t *)&reg_htons;
    uint8_t buf[] = {reg_u8[0], reg_u8[1], data};

    if(twi_writeTo(slv_addr, buf, 3, true) != 0) {
        ret = 0xFF;
    }
    if (ret != 0) {
        ESP_LOGE(TAG, "W [%04x]=%02x %d fail\n", reg, data, i++);
    }
    return ret;
#endif
}
