// Workaround - bug in cam library - enable bits are set without using bitwise OR logic -> only latest enable setting is used
// Reference: https://esp32.com/viewtopic.php?f=19&t=14376#p93178
/* The memory structure is as follows for
   byte_0 = enable_bits
      byte_0->bit0 = enable saturation and hue --> OK
      byte_0->bit1 = enable saturation --> OK
      byte_0->bit2 = enable brightness and contrast --> OK
      byte_0->bit3 = enable green -> blue spitial effect (Antique and blunish and greenish and reddish and b&w) enable
      byte_0->bit4 = anable gray -> red spitial effect (Antique and blunish and greenish and reddish and b&w) enable
      byte_0->bit5 = remove (UV) in YUV color system
      byte_0->bit6 = enable negative
      byte_0->bit7 = remove (Y) in YUV color system
   byte_1  = saturation1 0-255 --> ?
   byte_2  = hue 0-255 --> OK
   byte_3  = saturation2 0-255 --> OK
   byte_4  = reenter saturation2 in documents --> ?
   byte_5  = spital effect green -> blue 0-255 --> ?
   byte_6  = spital effect gray -> red 0-255 --> ?
   byte_7  = contrast lower byte 0-255 --> OK
   byte_8  = contrast higher byte 0-255 --> OK
   byte_9  = brightness 0-255 --> OK
   byte_10 = if byte_10==4 contrast effective --> ?
*/

#include <stdint.h>
#include "esp_camera.h"
#include "ov2640_specialEffect.h"

static const uint8_t special_effects_regs[8][5] = {
    {0x7C, 0x7D, 0x7C, 0x7D, 0x7D},
    {0x00, 0X00, 0x05, 0X80, 0X80}, /* no effect */
    {0x00, 0X40, 0x05, 0X80, 0X80}, /* negative */
    {0x00, 0X18, 0x05, 0X80, 0X80}, /* black and white */
    {0x00, 0X18, 0x05, 0X40, 0XC0}, /* reddish */
    {0x00, 0X18, 0x05, 0X40, 0X40}, /* greenish */
    {0x00, 0X18, 0x05, 0XA0, 0X40}, /* blue */
    {0x00, 0X18, 0x05, 0X40, 0XA6}, /* retro */
};

int ov2640_set_special_effect(sensor_t *sensor, int effect)
{
    int ret = 0;
    effect++;

    if (effect <= 0 || effect > 7)
    {
        effect = 1;
    }

    sensor->status.special_effect = effect - 1;

    int registerValue = 0x06; // enable saturation, contrast, brightness
    registerValue |= special_effects_regs[effect][1];

    // sensor->set_reg(sensor, int reg, int mask, int value)
    sensor->set_reg(sensor, 0xFF, 0x01, 0x00); // Select DSP bank
    sensor->set_reg(sensor, special_effects_regs[0][0], 0xFF, 0x00);
    sensor->set_reg(sensor, special_effects_regs[0][1], 0x5E, registerValue);

    for (int i = 2; i < 5; i++)
    {
        sensor->set_reg(sensor, special_effects_regs[0][i], 0xFF, special_effects_regs[effect][i]);
    }

    return ret;
}
