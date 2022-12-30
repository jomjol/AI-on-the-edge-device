# ROI (Region of Interest) Configuration

General remark:
> You are using a neural network approach which is trained to fit as many different type of meters as possible. The accuracy will never be 100%. It is normal to see a missing reading once in a while. There there are several precautions to detect this. For details see the section `PostProcessing` on the configuration page.

The most critical settings for accurate detection are:

1. Correct setting of the **R**egions **O**f **I**nterest (ROIs) for detection of the image.
   > This must be done manually for each meter!
2. Number type is part of the training set.
   > Have a look on the [Digital Counters](https://jomjol.github.io/neural-network-digital-counter-readout/) resp. [Analog Needles](https://jomjol.github.io/neural-network-analog-needle-readout) to check if your types are contained. If your number types are **not** contained, you should take the effort to record them so we can add them to the training data. See: [Learn models with your own images](https://github.com/jomjol/AI-on-the-edge-device/wiki/Learn-models-with-your-own-images) on how to create new input.

_____

## 1. Correct Setup of ROI
Please proceed in the following order!

Don't forget to save after each step!

### 1. Image Sharpness
Ensure a sharp image of the camera by adjusting the focal length of the ESP OV2640 camera. 
**Adjust the focus for the clearest possible image** See [these instructions](https://github.com/jomjol/water-meter-picture-provider/blob/master/ESP32-CAM_Lens_Modification.md) for help.

### 2. Horizontal Alignment
Ensure an **exact horizontal alignment** of the number via the alignment / reference setup:

| :heavy_check_mark: Okay                         | :x: Not Okay                         |
| ------------------------------ | ---------------------------------- |
| [[/images/alignment_okay.jpg]] | [[/images/alignment_not_okay.jpg]] |

### 3. Correct Size for ROI
Choose the right size of the ROI:
> The configuration of ROIs differs a bit on the model you choose. Below you find the differences between the different AI models. Pick the one you think fits best your purpose. If you don't get to good result, try another model.

### 4. Model Selection
#### dig-class11 Configuration
dig-class11 - Models recognize the **complete digit only**. Here it is not relevant if the ROI fits the Border of the digit window.

For this model, there should be a border of 20% of the image size around the number itself. This border is shown in the ROI setup image by the inner thinner rectangle. This rectangle should fit perfectly around the number when the number has not started to rotate to the next position: 

<img width="300px" src=https://github.com/jomjol/AI-on-the-edge-device/wiki/images/ROI_drawing.jpg>

|              | Example 1                         | Example 2                         |
| ------------ | --------------------------------- | --------------------------------- |
| :heavy_check_mark: **Okay**     | [[/images/bw_okay.jpg]]           | [[/images/wb_okay.jpg]]           |
| :x: **Not** Okay | [[/images/bw_not_okay_small.jpg]] | [[/images/wb_not_okay_small.jpg]] |
| :x: **Not** Okay | [[/images/bw_not_okay_big.jpg]]   | [[/images/wb_not_okay_big.jpg]]   |



If you have perfect alignment you and are not getting satisfying results, most probably your numbers are not part of the training data yet. Read on [Learn models with your own images](https://github.com/jomjol/AI-on-the-edge-device/wiki/Learn-models-with-your-own-images) how to add your meter's type of numbers to the training set.


#### dig-class100 / dig-cont Configuration

These models recognize the tenths (fractions) between the numbers. This model requires a different ROI setup; the height must be set differently and more accurately.

First, the width can be set as for dig-class11, i.e. 20% margin left and right.

<img width="455" alt="ROI-setup" src="https://user-images.githubusercontent.com/412645/199028748-c48ef5bb-a8d4-4c77-9faf-763e6cf77351.png">

The height of the outer rectangle should be set to the upper and lower edge of the number window. To achieve this setting, you need to unlock the aspect ratio:

<img width="168" alt="unlockAspectRatio" src="https://user-images.githubusercontent.com/412645/199028590-21708ff3-15a3-4415-89b1-c2affcfce003.png">


Here an example:

|              | Example 1                         | 
| ------------ | --------------------------------- | 
| :heavy_check_mark: **Okay**     | <img width="125" alt="dig-class100_OK" src="https://user-images.githubusercontent.com/412645/199028380-7623776e-59b9-4356-ab55-3852253609df.png">          | 
| :x: **Not** Okay | <img width="125" alt="dig-class100_NOK" src="https://user-images.githubusercontent.com/412645/199028469-3a69ed31-e5c9-4038-a8dc-6d44a42437ed.png"> | 


