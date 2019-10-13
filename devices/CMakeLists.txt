add_subdirectory(cpu)

add_library(utilities
    utilities.c
    utilities.h)
target_compile_options(
    utilities
    PRIVATE -Wno-pointer-sign)
