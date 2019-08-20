/*
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

#include "flag_handler.h"
#include "decoder.h"
#include "registers.h"

/**
 * @brief is_negative Check if number is negative.
 * @param result
 * @param bw_flag
 * @return
 */
bool is_negative(uint16_t result, uint8_t bw_flag) {
  if (bw_flag == WORD) {
    return (result & (1u << 15)) != 0;
  } else {
    return (result & (1u << 7)) != 0;
  }
}

bool is_zero(uint16_t result, uint8_t bw_flag) {
  return bw_flag ? (result & 0xff) == 0 : (result & 0xffff) == 0;
}

/**
 * @brief Test if the result of the asm instruction WILL carry
 * @param original_dst_value The original value at the destination
 * @param source_value The value at the source location
 * @param bw_flag Byte or Word flag
 * @return true if zero, false otherwise
 */
uint8_t is_carried(uint32_t original_dst_value, uint32_t source_value,
                   uint8_t bw_flag) {
  if (bw_flag == WORD) {
    if ((65535 - (uint16_t)source_value) < (uint16_t)original_dst_value ||
        ((original_dst_value + source_value) >> 16) != 0) {
      return 1;
    }

    return 0;
  } else if (bw_flag == BYTE) {
    if ((255 - (uint8_t)source_value) < (uint8_t)original_dst_value ||
        ((original_dst_value + source_value) >> 8) != 0) {
      return 1;
    }

    return 0;
  }

  return false;
}

/**
 * @brief is_sub_carry Check if the subtraction of a - b will carry.
 * ref: https://en.wikipedia.org/wiki/Carry_flag
 * result = a - b
 * @param a
 * @param b
 * @param c Carry flag
 * @return
 */
bool is_sub_carry(int32_t a, int32_t b, bool c, uint8_t bw_flag) {

  // int32_t result = a + (~b) + c;
  return is_add_carry(a, (~b), c, bw_flag);
  // return (uint32_t)a > uintb;
  //  return a >= -((~b) + c);
}

bool is_sub_overflow(int32_t a, int32_t b, bool c, uint8_t bw_flag) {

  bool v;
  int32_t result = a + (~b) + c;

  bool aNeg = is_negative(a, bw_flag);
  bool bNeg = is_negative(b, bw_flag);

  if (!aNeg && bNeg) { // pos - neg = neg
    v = is_negative(result, bw_flag);
  } else if (aNeg && !bNeg) { // neg - pos = pos
    v = !is_negative(result, bw_flag);
  } else {
    v = false;
  }
  return v;
}

bool is_add_overflow(int32_t a, int32_t b, bool c, uint8_t bw_flag) {

  bool v;
  int32_t result = a + b + c;

  bool aNeg = is_negative(a, bw_flag);
  bool bNeg = is_negative(b, bw_flag);

  if (!aNeg && !bNeg) { // pos + pos = neg
    v = is_negative(result, bw_flag);
  } else if (aNeg && bNeg) { // neg + neg = pos
    v = !is_negative(result, bw_flag);
  } else {
    v = false;
  }
  return v;
}

bool is_add_carry(uint32_t a, uint32_t b, bool c, uint8_t bw_flag) {

  // Need to clear upper bits (in case of of signed values)
  if (bw_flag == WORD) {
    a = a & 0xffffu;
    b = b & 0xffffu;
  } else {
    a = a & 0xffu;
    b = b & 0xffu;
  }
  uint32_t res = a + b + c;
  return bw_flag ? res > 0xffu : res > 0xffffu;
}

/**
 * @brief Test if the result of the asm instruction is overflowed
 * @param source_value The value at the source operand
 * @param destination_value The value at the destination operand
 * @param result A pointer to the result of the operation
 * @param bw_flag Byte or Word flag
 * @return true if zero, false otherwise
 */
uint8_t is_overflowed(uint16_t source_value, uint16_t destination_value,
                      uint16_t *result, uint8_t bw_flag) {
  if (bw_flag == WORD) {
    if ((source_value >> 15) == (destination_value >> 15) &&
        (*result >> 15) != (destination_value >> 15)) {
      return 1;
    }

    return 0;
  } else if (bw_flag == BYTE) {
    uint8_t dst_prev_value = (uint8_t)destination_value;
    uint8_t src_value = (uint8_t)source_value;

    if ((src_value >> 7) == (dst_prev_value >> 7) &&
        (*(uint8_t *)result >> 7) != (dst_prev_value >> 7)) {
      return 1;
    }

    return 0;
  } else {
    printf("Error, overflowed function: invalid bw_flag\n");
  }

  return false;
}
