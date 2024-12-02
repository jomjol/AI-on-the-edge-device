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
#include "ov2640_contrast_brightness.h"

static const uint8_t brightness_regs[6][5] = {
    {0x7C, 0x7D, 0x7C, 0x7D, 0x7D },
    {0x00, 0x04, 0x09, 0x00, 0x00 }, /* -2 */
    {0x00, 0x04, 0x09, 0x10, 0x00 }, /* -1 */
    {0x00, 0x04, 0x09, 0x20, 0x00 }, /*  0 */
    {0x00, 0x04, 0x09, 0x30, 0x00 }, /* +1 */
    {0x00, 0x04, 0x09, 0x40, 0x00 }, /* +2 */
};

static const uint8_t contrast_regs[6][7] = {
    {0x7C, 0x7D, 0x7C, 0x7D, 0x7D, 0x7D, 0x7D },
    {0x00, 0x04, 0x07, 0x20, 0x18, 0x34, 0x06 }, /* -2 */
    {0x00, 0x04, 0x07, 0x20, 0x1c, 0x2a, 0x06 }, /* -1 */
    {0x00, 0x04, 0x07, 0x20, 0x20, 0x20, 0x06 }, /*  0 */
    {0x00, 0x04, 0x07, 0x20, 0x24, 0x16, 0x06 }, /* +1 */
    {0x00, 0x04, 0x07, 0x20, 0x28, 0x0c, 0x06 }, /* +2 */
};

int ov2640_set_contrast_brightness(sensor_t *sensor, int _contrast, int _brightness)
{
    int ret=0;

    _contrast += 3;
    if (_contrast <= 0) {
        _contrast = 3;
    }
	else if (_contrast > 5)
	{
		_contrast = 5;
	}
    sensor->status.contrast = _contrast-3;

    _brightness += 3;
    if (_brightness <= 0) {
        _brightness = 3;
    }
	else if (_brightness > 5)
	{
		_brightness = 5;
	}
	int brightness = brightness_regs[_brightness][3];
    sensor->status.brightness = _brightness-3;

    // sensor->set_reg(sensor, int reg, int mask, int value)
    sensor->set_reg(sensor, 0xFF, 0x01, 0x00); // Select DSP bank

    for (int i=0; i<7; i++) 
	{
		if (i == 5)
		{
			sensor->set_reg(sensor, contrast_regs[0][i], 0xFF, (brightness | contrast_regs[_contrast][i]));
		}
		else
		{
			sensor->set_reg(sensor, contrast_regs[0][i], 0xFF, contrast_regs[_contrast][i]);
		}
    }

    return ret;
}
