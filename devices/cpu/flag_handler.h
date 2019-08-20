﻿/*
  MSP430 Emulator
  Copyright (C) 2014, 2015 Rudolf Geosits (rgeosits@live.esu.edu)

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

#ifndef _FLAG_HANDLER_H_
#define _FLAG_HANDLER_H_

#include "decoder.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

uint8_t is_overflowed(uint16_t source, uint16_t original_destination,
                      uint16_t *result_addr, uint8_t bw_flag);

bool is_negative(uint16_t result, uint8_t bw_flag);
bool is_zero(uint16_t result, uint8_t bw_flag);

uint8_t is_carried(uint32_t original_dst_value, uint32_t source_value,
                   uint8_t bw_flag);

bool is_sub_carry(int32_t a, int32_t b, bool c, uint8_t bw_flag);
bool is_sub_overflow(int32_t a, int32_t b, bool c, uint8_t bw_flag);

bool is_add_overflow(int32_t a, int32_t b, bool c, uint8_t bw_flag);
bool is_add_carry(uint32_t a, uint32_t b, bool c, uint8_t bw_flag);

#endif
