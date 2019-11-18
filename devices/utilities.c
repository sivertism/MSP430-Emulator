/*
MSP430 Emulator
Copyright (C) 2016 Rudolf Geosits (rgeosits@live.esu.edu)
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses
*/

#include "utilities.h"
#include "assert.h"

// Writebacks for memory access
static void (*write_memory_cb)(const uint32_t, uint8_t *const, size_t) = NULL;
static void (*read_memory_cb)(const uint32_t, uint8_t *const, size_t) = NULL;

// Set callback to write data to memory
void set_write_memory_cb(void (*fptr)(const uint32_t, uint8_t *const, size_t)) {
  write_memory_cb = fptr;
}

// Set callback to read data from memory
void set_read_memory_cb(void (*fptr)(const uint32_t, uint8_t *const, size_t)) {
  read_memory_cb = fptr;
}

void (*consume_cycles_cb)(uint16_t);
void (*register_read_notify_cb)(uint16_t);
void (*register_write_notify_cb)(uint16_t);

void set_consume_cycles_cb(void (*functionPtr)(uint16_t)) {
  consume_cycles_cb = functionPtr;
}

void set_register_read_notify_cb(void (*functionPtr)(uint16_t)) {
  register_read_notify_cb = functionPtr;
}

void set_register_write_notify_cb(void (*functionPtr)(uint16_t)) {
  register_write_notify_cb = functionPtr;
}

uint16_t pack16(const uint8_t *const data) {
#ifdef TARGET_BIG_ENDIAN
  return ((uint16_t)data[0] << 8 | (uint16_t)data[1] << 0);
#elif defined(TARGET_LITTLE_ENDIAN)
  return ((uint16_t)data[1] << 8 | (uint16_t)data[0] << 0);
#else
#error Must define TARGET_BIG_ENDIAN or TARGET_LITTLE_ENDIAN
#endif
}

void unpack16(uint8_t *const out, const uint16_t in) {
#ifdef TARGET_BIG_ENDIAN
  out[0] = (0x0000ff00 & in) >> 8;
  out[1] = (0x000000ff & in) >> 0;
#elif defined(TARGET_LITTLE_ENDIAN)
  out[1] = (0x0000ff00 & in) >> 8;
  out[0] = (0x000000ff & in) >> 0;
#else
#error Must define TARGET_BIG_ENDIAN or TARGET_LITTLE_ENDIAN
#endif
}

uint16_t mem_read(uint16_t address, access_t atype) {
  assert(read_memory_cb != NULL);
  uint8_t tmp[2];
  uint16_t res;
  if (atype == WORD) {
    read_memory_cb(address, tmp, 2);
    res = pack16(tmp);
  } else if (atype == BYTE) {
    read_memory_cb(address, tmp, 1);
    res = tmp[0];
  }
  return res;
}

void mem_write(uint16_t address, uint16_t val, access_t atype) {
  assert(write_memory_cb != NULL);
  uint8_t data[2];
  if (atype == WORD) {
    unpack16(data, val);
    write_memory_cb(address, data, 2);
  } else if (atype == BYTE) {
    data[0] = val;
    write_memory_cb(address, data, 1);
  }
}

/**
 * @brief Get a pointer to the register specified by the numeric register
 * value
 * @param cpu A pointer to the CPU structure
 * @param reg The numeric value of the register
 * @return Pointer to the register in question, NULL if register doesn't exist
 */
int16_t *get_reg_ptr(Cpu *cpu, uint8_t reg) {

  switch (reg) {
  case 0x0:
    return &cpu->pc;
  case 0x1:
    return &cpu->sp;
  case 0x2:
    return &cpu->sr;
  case 0x3:
    return &cpu->cg2;
  case 0x4:
    return &cpu->r4;
  case 0x5:
    return &cpu->r5;
  case 0x6:
    return &cpu->r6;
  case 0x7:
    return &cpu->r7;
  case 0x8:
    return &cpu->r8;
  case 0x9:
    return &cpu->r9;
  case 0xA:
    return &cpu->r10;
  case 0xB:
    return &cpu->r11;
  case 0xC:
    return &cpu->r12;
  case 0xD:
    return &cpu->r13;
  case 0xE:
    return &cpu->r14;
  case 0xF:
    return &cpu->r15;

  default: {
    puts("Invalid Register Number");
    return 0;
  }
  }
}

/**
 * @brief Convert register ASCII name to it's respective numeric value
 * @param name The register's ASCII name
 * @return The numeric equivalent for the register on success, -1 if an
 * invalid name was supplied
 */
int8_t reg_name_to_num(char *name) {
  if (!strncasecmp("%r0", name, sizeof "%r0") ||
      !strncasecmp("r0", name, sizeof "r0") ||
      !strncasecmp("%pc", name, sizeof "%pc") ||
      !strncasecmp("pc", name, sizeof "pc")) {

    return 0;
  } else if (!strncasecmp("%r1", name, sizeof "%r1") ||
             !strncasecmp("r1", name, sizeof "r1") ||
             !strncasecmp("%sp", name, sizeof "%sp") ||
             !strncasecmp("sp", name, sizeof "sp")) {

    return 1;
  } else if (!strncasecmp("%r2", name, sizeof "%r2") ||
             !strncasecmp("r2", name, sizeof "r2") ||
             !strncasecmp("%sr", name, sizeof "%sr") ||
             !strncasecmp("sr", name, sizeof "sr")) {

    return 2;
  } else if (!strncasecmp("%r3", name, sizeof "%r3") ||
             !strncasecmp("r3", name, sizeof "r3") ||
             !strncasecmp("%cg2", name, sizeof "%cg2") ||
             !strncasecmp("cg2", name, sizeof "cg2")) {

    return 3;
  } else if (!strncasecmp("%r4", name, sizeof "%r4") ||
             !strncasecmp("r4", name, sizeof "r4")) {

    return 4;
  } else if (!strncasecmp("%r5", name, sizeof "%r5") ||
             !strncasecmp("r5", name, sizeof "r5")) {

    return 5;
  } else if (!strncasecmp("%r6", name, sizeof "%r6") ||
             !strncasecmp("r6", name, sizeof "r6")) {

    return 6;
  } else if (!strncasecmp("%r7", name, sizeof "%r7") ||
             !strncasecmp("r7", name, sizeof "r7")) {

    return 7;
  } else if (!strncasecmp("%r8", name, sizeof "%r8") ||
             !strncasecmp("r8", name, sizeof "r8")) {

    return 8;
  } else if (!strncasecmp("%r9", name, sizeof "%r9") ||
             !strncasecmp("r9", name, sizeof "r9")) {

    return 9;
  } else if (!strncasecmp("%r10", name, sizeof "%r10") ||
             !strncasecmp("r10", name, sizeof "r10")) {

    return 10;
  } else if (!strncasecmp("%r11", name, sizeof "%r11") ||
             !strncasecmp("r11", name, sizeof "r11")) {

    return 11;
  } else if (!strncasecmp("%r12", name, sizeof "%r12") ||
             !strncasecmp("r12", name, sizeof "r12")) {

    return 12;
  } else if (!strncasecmp("%r13", name, sizeof "%r13") ||
             !strncasecmp("r13", name, sizeof "r13")) {

    return 13;
  } else if (!strncasecmp("%r14", name, sizeof "%r14") ||
             !strncasecmp("r14", name, sizeof "r14")) {

    return 14;
  } else if (!strncasecmp("%r15", name, sizeof "%r15") ||
             !strncasecmp("r15", name, sizeof "r15")) {

    return 15;
  }

  return -1;
}

/**
 * @brief Convert register number into its ASCII name
 * @param number The register number (0, 1, 2, ...) associated with a
 * register's name like (R0, R1, R2, ...)
 * @param name A pointer to an allocated character array to fill up with
 * the register's ASCII name
 */
void reg_num_to_name(uint8_t number, char *name) {
  switch (number) {
  case 0x0: {
    strncpy(name, "PC\0", 3);
    return;
  }
  case 0x1: {
    strncpy(name, "SP\0", 3);
    return;
  }
  case 0x2: {
    strncpy(name, "SR\0", 3);
    return;
  }
  case 0x3: {
    strncpy(name, "R3\0", 3);
    return;
  }
  case 0x4: {
    strncpy(name, "R4\0", 3);
    return;
  }
  case 0x5: {
    strncpy(name, "R5\0", 3);
    return;
  }
  case 0x6: {
    strncpy(name, "R6\0", 3);
    return;
  }
  case 0x7: {
    strncpy(name, "R7\0", 3);
    return;
  }
  case 0x8: {
    strncpy(name, "R8\0", 3);
    return;
  }
  case 0x9: {
    strncpy(name, "R9\0", 3);
    return;
  }
  case 0xA: {
    strncpy(name, "R10\0", 4);
    return;
  }
  case 0xB: {
    strncpy(name, "R11\0", 4);
    return;
  }
  case 0xC: {
    strncpy(name, "R12\0", 4);
    return;
  }
  case 0xD: {
    strncpy(name, "R13\0", 4);
    return;
  }
  case 0xE: {
    strncpy(name, "R14\0", 4);
    return;
  }
  case 0xF: {
    strncpy(name, "R15\0", 4);
    return;
  }
  default: {
    strncpy(name, "???\0", 4);
    return;
  }
  }
}
