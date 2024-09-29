#include <stdint.h>
#include "esp_camera.h"
#include "ov2640_sharpness.h"


const static uint8_t OV2640_SHARPNESS_AUTO[]=
{
	//reg, val, mask
    0xFF, 0x00, 0xFF,
    0x92, 0x01, 0xFF,
    0x93, 0x20, 0x20,
    0x00, 0x00, 0x00
};

const static uint8_t OV2640_SHARPNESS_MANUAL[]=
{
	//reg, val, mask
    0xFF, 0x00, 0xFF,
    0x92, 0x01, 0xFF,
    0x93, 0x00, 0x20,
    0x00, 0x00, 0x00
};

const static uint8_t OV2640_SHARPNESS_LEVEL0[]=
{
	//reg, val, mask
    0xFF, 0x00, 0xFF,
    0x92, 0x01, 0xFF,
    0x93, 0xC0, 0x1F,
    0x00, 0x00, 0x00
};
const static uint8_t OV2640_SHARPNESS_LEVEL1[]=
{
	//reg, val, mask
    0xFF, 0x00, 0xFF,
    0x92, 0x01, 0xFF,
    0x93, 0xC1, 0x1F,
    0x00, 0x00, 0x00
};
const static uint8_t OV2640_SHARPNESS_LEVEL2[]=
{
	//reg, val, mask
    0xFF, 0x00, 0xFF,
    0x92, 0x01, 0xFF,
    0x93, 0xC2, 0x1F,
    0x00, 0x00, 0x00
};
const static uint8_t OV2640_SHARPNESS_LEVEL3[]=
{
	//reg, val, mask
    0xFF, 0x00, 0xFF,
    0x92, 0x01, 0xFF,
    0x93, 0xC4, 0x1F,
    0x00, 0x00, 0x00
};
const static uint8_t OV2640_SHARPNESS_LEVEL4[]=
{
	//reg, val, mask
    0xFF, 0x00, 0xFF,
    0x92, 0x01, 0xFF,
    0x93, 0xC8, 0x1F,
    0x00, 0x00, 0x00
};
const static uint8_t OV2640_SHARPNESS_LEVEL5[]=
{
	//reg, val, mask
    0xFF, 0x00, 0xFF,
    0x92, 0x01, 0xFF,
    0x93, 0xD0, 0x1F,
    0x00, 0x00, 0x00
};
const static uint8_t OV2640_SHARPNESS_LEVEL6[]=
{
	//reg, val, mask
    0xFF, 0x00, 0xFF,
    0x92, 0x01, 0xFF,
    0x93, 0xDF, 0x1F,
    0x00, 0x00, 0x00
};

const static uint8_t *OV2640_SETTING_SHARPNESS[]=
{
    OV2640_SHARPNESS_LEVEL0, // -3 sharpness
    OV2640_SHARPNESS_LEVEL1,
    OV2640_SHARPNESS_LEVEL2,
    OV2640_SHARPNESS_LEVEL3,
    OV2640_SHARPNESS_LEVEL4,
    OV2640_SHARPNESS_LEVEL5,
    OV2640_SHARPNESS_LEVEL6  // +3 sharpness
};

#define OV2640_MAXLEVEL_SHARPNESS 6


static int table_mask_write(sensor_t *sensor, const uint8_t* ptab)
{
    uint8_t address;
    uint8_t value;
    uint8_t orgval;
    uint8_t mask;
    const uint8_t *pdata = ptab;

    if (pdata == NULL)
	{
        return -1;
	}

    while (1)
    {   
        address = *pdata++;
        value = *pdata++;
        mask = *pdata++;
		
        if ((address == 0) && (value == 0) && (mask == 0))
		{
            break;
		}
		
        sensor->set_reg(sensor, address, mask, value);
    }   

    return 0;
}


int ov2640_enable_auto_sharpness(sensor_t *sensor)
{
    table_mask_write(sensor, OV2640_SHARPNESS_AUTO);
	
    return 0;
}


int ov2640_set_sharpness(sensor_t *sensor, int sharpness)
{
	int sharpness_temp = 0;
	
    if (sharpness < -3)
	{
        sharpness_temp = -3;
	}
	
    if (sharpness > OV2640_MAXLEVEL_SHARPNESS - 3)
	{
        sharpness_temp = OV2640_MAXLEVEL_SHARPNESS - 3;
	}	

    table_mask_write(sensor, OV2640_SHARPNESS_MANUAL);
    table_mask_write(sensor, OV2640_SETTING_SHARPNESS[sharpness_temp + 3]);
	
    return 0;
}
