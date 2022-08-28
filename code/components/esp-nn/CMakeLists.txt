idf_build_get_property(idf_target IDF_TARGET)

set(c_srcs
    "src/activation_functions/esp_nn_relu_ansi.c"
    "src/basic_math/esp_nn_add_ansi.c"
    "src/basic_math/esp_nn_mul_ansi.c"
    "src/convolution/esp_nn_conv_ansi.c"
    "src/convolution/esp_nn_conv_opt.c"
    "src/convolution/esp_nn_depthwise_conv_ansi.c"
    "src/convolution/esp_nn_depthwise_conv_opt.c"
    "src/fully_connected/esp_nn_fully_connected_ansi.c"
    "src/softmax/esp_nn_softmax_ansi.c"
    "src/softmax/esp_nn_softmax_opt.c"
    "src/pooling/esp_nn_avg_pool_ansi.c"
    "src/pooling/esp_nn_max_pool_ansi.c")

if(CONFIG_IDF_TARGET_ESP32S3)
    set(s3_srcs
        "src/common/esp_nn_common_functions_esp32s3.S"
        "src/common/esp_nn_multiply_by_quantized_mult_esp32s3.S"
        "src/common/esp_nn_multiply_by_quantized_mult_ver1_esp32s3.S"
        "src/activation_functions/esp_nn_relu_s8_esp32s3.S"
        "src/basic_math/esp_nn_add_s8_esp32s3.S"
        "src/basic_math/esp_nn_mul_s8_esp32s3.S"
        "src/convolution/esp_nn_conv_esp32s3.c"
        "src/convolution/esp_nn_depthwise_conv_s8_esp32s3.c"
        "src/convolution/esp_nn_conv_s16_mult8_esp32s3.S"
        "src/convolution/esp_nn_conv_s8_mult8_1x1_esp32s3.S"
        "src/convolution/esp_nn_conv_s16_mult4_1x1_esp32s3.S"
        "src/convolution/esp_nn_depthwise_conv_s8_mult1_3x3_padded_esp32s3.S"
        "src/convolution/esp_nn_depthwise_conv_s16_mult1_esp32s3.S"
        "src/convolution/esp_nn_depthwise_conv_s16_mult1_3x3_esp32s3.S"
        "src/convolution/esp_nn_depthwise_conv_s16_mult1_3x3_no_pad_esp32s3.S"
        "src/convolution/esp_nn_depthwise_conv_s16_mult8_3x3_esp32s3.S"
        "src/convolution/esp_nn_depthwise_conv_s16_mult4_esp32s3.S"
        "src/convolution/esp_nn_depthwise_conv_s16_mult8_esp32s3.S"
        "src/fully_connected/esp_nn_fully_connected_s8_esp32s3.S"
        "src/pooling/esp_nn_max_pool_s8_esp32s3.S"
        "src/pooling/esp_nn_avg_pool_s8_esp32s3.S")
endif()

idf_component_register(SRCS "${c_srcs}"
                            "${s3_srcs}"
                       INCLUDE_DIRS "include" "src/common")

if(CONFIG_IDF_TARGET_ESP32S3)
    target_compile_options(${COMPONENT_LIB} PRIVATE -mlongcalls -fno-unroll-loops -O2 -Wno-unused-function)
else()
    target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-unused-function)
endif()