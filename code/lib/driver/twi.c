/*
  si2c.c - Software I2C library for ESP31B

  Copyright (c) 2015 Hristo Gochkov. All rights reserved.
  This file is part of the ESP31B core for Arduino environment.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include <stdint.h>
#include <stdbool.h>
#include "twi.h"
#include "soc/gpio_reg.h"
#include "soc/gpio_struct.h"
#include "soc/io_mux_reg.h"
#include "driver/rtc_io.h"
#include <stdio.h>


#define LOW               0x0
#define HIGH              0x1

//GPIO FUNCTIONS
#define INPUT             0x01
#define OUTPUT            0x02
#define PULLUP            0x04
#define INPUT_PULLUP      0x05
#define PULLDOWN          0x08
#define INPUT_PULLDOWN    0x09
#define OPEN_DRAIN        0x10
#define OUTPUT_OPEN_DRAIN 0x12
#define SPECIAL           0xF0
#define FUNCTION_1        0x00
#define FUNCTION_2        0x20
#define FUNCTION_3        0x40
#define FUNCTION_4        0x60
#define FUNCTION_5        0x80
#define FUNCTION_6        0xA0

#define ESP_REG(addr) *((volatile uint32_t *)(addr))

const uint8_t pin_to_mux[40] = { 0x44, 0x88, 0x40, 0x84, 0x48, 0x6c, 0x60, 0x64, 0x68, 0x54, 0x58, 0x5c, 0x34, 0x38, 0x30, 0x3c, 0x4c, 0x50, 0x70, 0x74, 0x78, 0x7c, 0x80, 0x8c, 0, 0x24, 0x28, 0x2c, 0, 0, 0, 0, 0x1c, 0x20, 0x14, 0x18, 0x04, 0x08, 0x0c, 0x10};

static void pinMode(uint8_t pin, uint8_t mode)
{
    if(pin >= 40) {
        return;
    }

    uint32_t rtc_reg = rtc_gpio_desc[pin].reg;

    //RTC pins PULL settings
    if(rtc_reg) {
        //lock rtc
        ESP_REG(rtc_reg) = ESP_REG(rtc_reg) & ~(rtc_gpio_desc[pin].mux);
        if(mode & PULLUP) {
            ESP_REG(rtc_reg) = (ESP_REG(rtc_reg) | rtc_gpio_desc[pin].pullup) & ~(rtc_gpio_desc[pin].pulldown);
        } else if(mode & PULLDOWN) {
            ESP_REG(rtc_reg) = (ESP_REG(rtc_reg) | rtc_gpio_desc[pin].pulldown) & ~(rtc_gpio_desc[pin].pullup);
        } else {
            ESP_REG(rtc_reg) = ESP_REG(rtc_reg) & ~(rtc_gpio_desc[pin].pullup | rtc_gpio_desc[pin].pulldown);
        }
        //unlock rtc
    }

    uint32_t pinFunction = 0, pinControl = 0;

    //lock gpio
    if(mode & INPUT) {
        if(pin < 32) {
            GPIO.enable_w1tc = BIT(pin);
        } else {
            GPIO.enable1_w1tc.val = BIT(pin - 32);
        }
    } else if(mode & OUTPUT) {
        if(pin > 33) {
            //unlock gpio
            return;//pins above 33 can be only inputs
        } else if(pin < 32) {
            GPIO.enable_w1ts = BIT(pin);
        } else {
            GPIO.enable1_w1ts.val = BIT(pin - 32);
        }
    }

    if(mode & PULLUP) {
        pinFunction |= FUN_PU;
    } else if(mode & PULLDOWN) {
        pinFunction |= FUN_PD;
    }

    pinFunction |= ((uint32_t)2 << FUN_DRV_S);//what are the drivers?
    pinFunction |= FUN_IE;//input enable but required for output as well?

    if(mode & (INPUT | OUTPUT)) {
        pinFunction |= ((uint32_t)2 << MCU_SEL_S);
    } else if(mode == SPECIAL) {
        pinFunction |= ((uint32_t)(((pin)==1||(pin)==3)?0:1) << MCU_SEL_S);
    } else {
        pinFunction |= ((uint32_t)(mode >> 5) << MCU_SEL_S);
    }

    ESP_REG(DR_REG_IO_MUX_BASE + pin_to_mux[pin]) = pinFunction;

    if(mode & OPEN_DRAIN) {
        pinControl = (1 << GPIO_PIN0_PAD_DRIVER_S);
    }

    GPIO.pin[pin].val = pinControl;
    //unlock gpio
}

static void digitalWrite(uint8_t pin, uint8_t val)
{
    if(val) {
        if(pin < 32) {
            GPIO.out_w1ts = BIT(pin);
        } else if(pin < 34) {
            GPIO.out1_w1ts.val = BIT(pin - 32);
        }
    } else {
        if(pin < 32) {
            GPIO.out_w1tc = BIT(pin);
        } else if(pin < 34) {
            GPIO.out1_w1tc.val = BIT(pin - 32);
        }
    }
}


unsigned char twi_dcount = 18;
static unsigned char twi_sda, twi_scl;


static inline void SDA_LOW()
{
    //Enable SDA (becomes output and since GPO is 0 for the pin,
    // it will pull the line low)
    if (twi_sda < 32) {
        GPIO.enable_w1ts = BIT(twi_sda);
    } else {
        GPIO.enable1_w1ts.val = BIT(twi_sda - 32);
    }
}

static inline void SDA_HIGH()
{
    //Disable SDA (becomes input and since it has pullup it will go high)
    if (twi_sda < 32) {
        GPIO.enable_w1tc = BIT(twi_sda);
    } else {
        GPIO.enable1_w1tc.val = BIT(twi_sda - 32);
    }
}

static inline uint32_t SDA_READ()
{
    if (twi_sda < 32) {
        return (GPIO.in & BIT(twi_sda)) != 0;
    } else {
        return (GPIO.in1.val & BIT(twi_sda - 32)) != 0;
    }
}

static void SCL_LOW()
{
    if (twi_scl < 32) {
        GPIO.enable_w1ts = BIT(twi_scl);
    } else {
        GPIO.enable1_w1ts.val = BIT(twi_scl - 32);
    }
}

static void SCL_HIGH()
{
    if (twi_scl < 32) {
        GPIO.enable_w1tc = BIT(twi_scl);
    } else {
        GPIO.enable1_w1tc.val = BIT(twi_scl - 32);
    }
}

static uint32_t SCL_READ()
{
    if (twi_scl < 32) {
        return (GPIO.in & BIT(twi_scl)) != 0;
    } else {
        return (GPIO.in1.val & BIT(twi_scl - 32)) != 0;
    }
}


#ifndef FCPU80
#define FCPU80 80000000L
#endif

#if F_CPU == FCPU80
#define TWI_CLOCK_STRETCH 800
#else
#define TWI_CLOCK_STRETCH 1600
#endif

void twi_setClock(unsigned int freq)
{
#if F_CPU == FCPU80
    if(freq <= 100000) {
        twi_dcount = 19;    //about 100KHz
    } else if(freq <= 200000) {
        twi_dcount = 8;    //about 200KHz
    } else if(freq <= 300000) {
        twi_dcount = 3;    //about 300KHz
    } else if(freq <= 400000) {
        twi_dcount = 1;    //about 400KHz
    } else {
        twi_dcount = 1;    //about 400KHz
    }
#else
    if(freq <= 100000) {
        twi_dcount = 32;    //about 100KHz
    } else if(freq <= 200000) {
        twi_dcount = 14;    //about 200KHz
    } else if(freq <= 300000) {
        twi_dcount = 8;    //about 300KHz
    } else if(freq <= 400000) {
        twi_dcount = 5;    //about 400KHz
    } else if(freq <= 500000) {
        twi_dcount = 3;    //about 500KHz
    } else if(freq <= 600000) {
        twi_dcount = 2;    //about 600KHz
    } else {
        twi_dcount = 1;    //about 700KHz
    }
#endif
}

void twi_init(unsigned char sda, unsigned char scl)
{
    twi_sda = sda;
    twi_scl = scl;
    pinMode(twi_sda, OUTPUT);
    pinMode(twi_scl, OUTPUT);

    digitalWrite(twi_sda, 0);
    digitalWrite(twi_scl, 0);

    pinMode(twi_sda, INPUT_PULLUP);
    pinMode(twi_scl, INPUT_PULLUP);
    twi_setClock(100000);
}

void twi_stop(void)
{
    pinMode(twi_sda, INPUT);
    pinMode(twi_scl, INPUT);
}

static void twi_delay(unsigned char v)
{
    unsigned int i;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
    unsigned int reg;
    for(i=0; i<v; i++) {
        reg = REG_READ(GPIO_IN_REG);
    }
#pragma GCC diagnostic pop
}

static bool twi_write_start(void)
{
    SCL_HIGH();
    SDA_HIGH();
    if (SDA_READ() == 0) {
        return false;
    }
    twi_delay(twi_dcount);
    SDA_LOW();
    twi_delay(twi_dcount);
    return true;
}

static bool twi_write_stop(void)
{
    unsigned int i = 0;
    SCL_LOW();
    SDA_LOW();
    twi_delay(twi_dcount);
    SCL_HIGH();
    while (SCL_READ() == 0 && (i++) < TWI_CLOCK_STRETCH);// Clock stretching (up to 100us)
    twi_delay(twi_dcount);
    SDA_HIGH();
    twi_delay(twi_dcount);

    return true;
}

bool do_log = false;
static bool twi_write_bit(bool bit)
{
    unsigned int i = 0;
    SCL_LOW();
    if (bit) {
        SDA_HIGH();
        if (do_log) {
            twi_delay(twi_dcount+1);
        }
    } else {
        SDA_LOW();
        if (do_log) {}
    }
    twi_delay(twi_dcount+1);
    SCL_HIGH();
    while (SCL_READ() == 0 && (i++) < TWI_CLOCK_STRETCH);// Clock stretching (up to 100us)
    twi_delay(twi_dcount);
    return true;
}

static bool twi_read_bit(void)
{
    unsigned int i = 0;
    SCL_LOW();
    SDA_HIGH();
    twi_delay(twi_dcount+2);
    SCL_HIGH();
    while (SCL_READ() == 0 && (i++) < TWI_CLOCK_STRETCH);// Clock stretching (up to 100us)
    bool bit = SDA_READ();
    twi_delay(twi_dcount);
    return bit;
}

static bool twi_write_byte(unsigned char byte)
{

    if (byte == 0x43) {
        // printf("TWB %02x ", (uint32_t) byte);
        // do_log = true;
    }
    unsigned char bit;
    for (bit = 0; bit < 8; bit++) {
        twi_write_bit((byte & 0x80) != 0);
        byte <<= 1;
    }
    if (do_log) {
        printf("\n");
        do_log = false;
    }
    return !twi_read_bit();//NACK/ACK
}

static unsigned char twi_read_byte(bool nack)
{
    unsigned char byte = 0;
    unsigned char bit;
    for (bit = 0; bit < 8; bit++) {
        byte = (byte << 1) | twi_read_bit();
    }
    twi_write_bit(nack);
    return byte;
}

unsigned char twi_writeTo(unsigned char address, unsigned char * buf, unsigned int len, unsigned char sendStop)
{
    unsigned int i;
    if(!twi_write_start()) {
        return 4;    //line busy
    }
    if(!twi_write_byte(((address << 1) | 0) & 0xFF)) {
        if (sendStop) {
            twi_write_stop();
        }
        return 2; //received NACK on transmit of address
    }
    for(i=0; i<len; i++) {
        if(!twi_write_byte(buf[i])) {
            if (sendStop) {
                twi_write_stop();
            }
            return 3;//received NACK on transmit of data
        }
    }
    if(sendStop) {
        twi_write_stop();
    }
    i = 0;
    while(SDA_READ() == 0 && (i++) < 10) {
        SCL_LOW();
        twi_delay(twi_dcount);
        SCL_HIGH();
        twi_delay(twi_dcount);
    }
    return 0;
}

unsigned char twi_readFrom(unsigned char address, unsigned char* buf, unsigned int len, unsigned char sendStop)
{
    unsigned int i;
    if(!twi_write_start()) {
        return 4;    //line busy
    }
    if(!twi_write_byte(((address << 1) | 1) & 0xFF)) {
        if (sendStop) {
            twi_write_stop();
        }
        return 2;//received NACK on transmit of address
    }
    for(i=0; i<(len-1); i++) {
        buf[i] = twi_read_byte(false);
    }
    buf[len-1] = twi_read_byte(true);
    if(sendStop) {
        twi_write_stop();
    }
    i = 0;
    while(SDA_READ() == 0 && (i++) < 10) {
        SCL_LOW();
        twi_delay(twi_dcount);
        SCL_HIGH();
        twi_delay(twi_dcount);
    }
    return 0;
}
