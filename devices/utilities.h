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

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "cpu/registers.h"
//#include "memory/memspace.h"

void reg_num_to_name(uint8_t source_reg, char *reg_name);
int16_t *get_reg_ptr(Cpu *cpu, uint8_t reg);
void set_write_memory_cb(void (*functionPtr)(uint16_t,uint16_t,uint8_t));
void set_read_memory_cb(uint16_t (*functionPtr)(uint16_t,uint8_t));

extern void (*write_memory_cb)(uint16_t,uint16_t,uint8_t);
extern uint16_t (*read_memory_cb)(uint16_t,uint8_t);

#endif
