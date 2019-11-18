add_subdirectory(cpu)

add_library(msp-utilities
    utilities.c
    utilities.h)
target_compile_options(
    msp-utilities
    PRIVATE -Wno-pointer-sign)
