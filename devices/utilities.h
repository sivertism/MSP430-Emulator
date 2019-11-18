/*
  This file is part of MSP430 Emulator

  MSP430 Emulator is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  MSP430 Emulator is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with MSP430 Emulator.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _UTILITIES_H_
#define _UTILITIES_H_

#include "cpu/registers.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum { WORD, BYTE } access_t;

struct istruct {
  int format; // Format I, II or III
  char mnemonic[10];
  bool isDestPC;
  char op1[10];
  char op2[10];
};
typedef struct istruct instruction_t;

void reg_num_to_name(uint8_t source_reg, char *reg_name);
int16_t *get_reg_ptr(Cpu *cpu, uint8_t reg);

void set_write_memory_cb(void (*fptr)(const uint32_t, uint8_t *const, size_t));
void set_read_memory_cb(void (*fptr)(const uint32_t, uint8_t *const, size_t));

void set_consume_cycles_cb(void (*functionPtr)(uint16_t));
void set_register_read_notify_cb(void (*functionPtr)(uint16_t));
void set_register_write_notify_cb(void (*functionPtr)(uint16_t));
uint16_t pack16(const uint8_t *const data);
void unpack16(uint8_t *const out, const uint16_t in);

extern void (*consume_cycles_cb)(uint16_t);
extern void (*register_read_notify_cb)(uint16_t);
extern void (*register_write_notify_cb)(uint16_t);

/**
 * @brief Read memory value from SystemC bus. Returns data in host endianness
 * @param address address to read from
 * @param atype access type (byte or word)
 * @return value in host endianness
 */
uint16_t mem_read(uint16_t address, access_t atype);

/**
 * @brief Write memory value to SystemC bus. Accepts data in host endianness
 * @param address address to read from
 * @param val value to write, in host endianness
 * @param atype access type (byte or word)
 */
void mem_write(uint16_t address, uint16_t val, access_t);

#endif
