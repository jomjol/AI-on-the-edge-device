
set(COMPONENT_ADD_INCLUDEDIRS ./include/)
set(COMPONENT_SRCS "src/basic_math_test.c"
                   "src/convolution_test.c"
                   "src/fully_connected_test.c"
                   "src/pooling_test.c"
                   "src/relu_test.c"
                   "src/softmax_test.c")

set(COMPONENT_REQUIRES )
set(COMPONENT_PRIV_REQUIRES esp-nn)

register_component()

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-unused-function)
