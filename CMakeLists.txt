add_subdirectory(debugger)
add_subdirectory(devices)

set(ROOTDIR ${CMAKE_SOURCE_DIR}/MSP430-Emulator)

#include_directories(${ROOTDIR}/debugger)
#link_directories(${ROOTDIR}/debugger)
#
#include_directories(${ROOTDIR}/debugger/server)
#link_directories(${ROOTDIR}/debugger/server)

#include_directories(${ROOTDIR}/devices)
#link_directories(${ROOTDIR}/devices)
add_library(testfn testfn.c testfn.h)
message(STATUS ${ROOTDIR})

add_executable(MSP430 main.c main.h)
target_link_libraries(MSP430
    emu_server
    utilities
    memspace
    cpu
    debugger
    disassembler
    register_display
    usci
    port1
    bcm
    timer_a
    packet_queue
    readline
    websockets
    pthread
    rt
    ssl
    crypto
    )


