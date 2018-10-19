add_subdirectory(server)
add_subdirectory(websockets)

add_library(disassembler STATIC disassembler.c disassembler.h)
add_library(register_display STATIC register_display.c register_display.h)
add_library(debugger STATIC debugger.c debugger.h)
