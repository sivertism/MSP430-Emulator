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

//##########+++ Decode Format I Instructions +++##########
//# Format I are double operand of the form:
//# [CCCC][SSSS][ABaa][DDDD]
//#
//# Where C = Opcode, B = Byte/Word flag,
//# A = Addressing mode for destination
//# a = Addressing mode for s_reg_name
//# S = S_Reg_Name, D = Destination
//########################################################

#include <stdio.h>
#include "formatI.h"
#include "decoder.h"
#include "opcodes.h"

void decode_formatI(Cpu *cpu, uint16_t instruction, char *disas)
{
    int is_saddr_virtual;
    int is_daddr_virtual;
    uint16_t source_vaddress;
    uint16_t dest_vaddress;

    uint8_t opcode = (instruction & 0xF000) >> 12;
    uint8_t source = (instruction & 0x0F00) >> 8;
    uint8_t as_flag = (instruction & 0x0030) >> 4;
    uint8_t destination = (instruction & 0x000F);
    uint8_t ad_flag = (instruction & 0x0080) >> 7;
    uint8_t bw_flag = (instruction & 0x0040) >> 6;

    char s_reg_name[10], d_reg_name[10];

    /* String to show hex value of instruction */
    char hex_str[100] = {0};
    char hex_str_part[10] = {0};

    sprintf(hex_str, "%04X", instruction);

    /* Source Register pointer */
    int16_t *s_reg = get_reg_ptr(cpu, source);

    /* Destination Register pointer */
    int16_t *d_reg = get_reg_ptr(cpu, destination);

    reg_num_to_name(source, s_reg_name);      /* Get source register name */
    reg_num_to_name(destination, d_reg_name); /* Get destination register name */

    uint8_t constant_generator_active = 0;    /* Specifies if CG1/CG2 active */
    int16_t immediate_constant = 0;           /* Generated Constant */

    /* Spot CG1 and CG2 Constant generator instructions */
    if ( (source == 2 && as_flag > 1) || source == 3 ) {
        constant_generator_active = 1;
        immediate_constant = run_constant_generator(source, as_flag);
    }
    else {
        constant_generator_active = 0;
    }

    /* Identify the nature of instruction operand addressing modes */
    int16_t source_value, source_offset;
    int16_t dest_value;
    int16_t destination_offset;
    uint16_t *destination_addr;
    char asm_operands[40] = {0};
    char asm_op2[20] = {0};

    /* Register - Register;     Ex: MOV Rs, Rd */
    /* Constant Gen - Register; Ex: MOV #C, Rd */ /* 0 */
    if (as_flag == 0 && ad_flag == 0) {
        if (constant_generator_active) {   /* Source Constant */
            source_value = immediate_constant;

            sprintf(asm_operands, "C#0x%04X, %s",
                    (uint16_t) source_value, d_reg_name);
        }
        else {                             /* Source register */
            source_value = *s_reg;
            sprintf(asm_operands, "%s, %s", s_reg_name, d_reg_name);
        }

        if (destination == REG_PC) {
            consume_cycles_cb(constant_generator_active ? 1 : 2);
        }

        destination_addr = d_reg;          /* Destination Register */
        dest_value = *d_reg;
        is_daddr_virtual = 0;
        is_saddr_virtual = 0;
    }

    /* Register - Indexed;      Ex: MOV Rs, 0x0(Rd) */
    /* Register - Symbolic;     Ex: MOV Rs, 0xD     */
    /* Register - Absolute;     Ex: MOV Rs, &0xD    */
    /* Constant Gen - Indexed;  Ex: MOV #C, 0x0(Rd) */ /* 0 */
    /* Constant Gen - Symbolic; Ex: MOV #C, 0xD     */ /* 0 */
    /* Constant Gen - Absolute; Ex: MOV #C, &0xD    */ /* 0 */
    else if (as_flag == 0 && ad_flag == 1) {
        destination_offset = fetch(cpu);

        sprintf(hex_str_part, "%04X", (uint16_t) destination_offset);
        strncat(hex_str, hex_str_part, sizeof hex_str_part);

        if (constant_generator_active) {   /* Source Constant */
            source_value = immediate_constant;
            sprintf(asm_operands, "C#0x%04X, ", source_value);
        }
        else {                             /* Source from register */
            source_value = *s_reg;
            sprintf(asm_operands, "%s, ", s_reg_name);
        }

        if (destination == 0) {            /* Destination Symbolic */
            uint16_t virtual_addr = *d_reg + destination_offset - 2;
            dest_vaddress = virtual_addr;

            sprintf(asm_op2, "0x%04X", (uint16_t) virtual_addr);
        }
        else if (destination == 2) {       /* Destination Absolute */
            dest_vaddress = destination_offset;
            sprintf(asm_op2, "&0x%04X", (uint16_t) destination_offset);
        }
        else {                             /* Destination Indexed */
            sprintf(asm_op2, "0x%04X(%s)",
                    (uint16_t) destination_offset, d_reg_name);

            dest_vaddress = (*d_reg + destination_offset);
        }

        if (opcode != OP_MOV) {
            dest_value = read_memory_cb(dest_vaddress, bw_flag);
        }

        is_daddr_virtual = 1;
        is_saddr_virtual = 0;

        strncat(asm_operands, asm_op2, sizeof asm_op2);
    }

    /* Indexed - Register;      Ex: MOV 0x0(Rs), Rd */
    /* Symbolic - Register;     Ex: MOV 0xS, Rd     */
    /* Absolute - Register;     Ex: MOV &0xS, Rd    */
    /* Constant Gen - Register; Ex: MOV #C, Rd      */ /* 1 */
    else if (as_flag == 1 && ad_flag == 0) {
        if (constant_generator_active) {   /* Source Constant */
            source_value = immediate_constant;
            sprintf(asm_operands, "C#0x%04X, %s", source_value, d_reg_name);
            is_saddr_virtual = 0;
        }
        else if (source == 0) {            /* Source Symbolic */
            source_offset = fetch(cpu);
            uint16_t virtual_addr = *s_reg + source_offset - 2;

            source_vaddress = virtual_addr;
            source_value = read_memory_cb(source_vaddress, bw_flag);
            is_saddr_virtual = 1;

            sprintf(hex_str_part, "%04X", (uint16_t) source_offset);
            strncat(hex_str, hex_str_part, sizeof hex_str);
            sprintf(asm_operands, "0x%04X, %s", virtual_addr, d_reg_name);
        }
        else if (source == 2) {            /* Source Absolute */
            source_offset = fetch(cpu);
            source_vaddress = source_offset;
            source_value = read_memory_cb(source_vaddress, bw_flag);
            is_saddr_virtual = 1;


            sprintf(hex_str_part, "%04X", (uint16_t) source_offset);
            strncat(hex_str, hex_str_part, sizeof hex_str);

            sprintf(asm_operands, "&0x%04X, %s",
                    (uint16_t) source_offset, d_reg_name);
        }
        else {                             /* Source Indexed */
            source_offset = fetch(cpu);

            source_vaddress = *s_reg + source_offset;
            source_value = read_memory_cb(source_vaddress, bw_flag);
            is_saddr_virtual = 1;

            sprintf(hex_str_part, "%04X", (uint16_t) source_offset);
            strncat(hex_str, hex_str_part, sizeof hex_str);

            sprintf(asm_operands, "0x%04X(%s), %s",
                    (uint16_t) source_offset, s_reg_name, d_reg_name);
        }

        if (destination == REG_PC) {
            consume_cycles_cb(constant_generator_active ? 1 : 2);
        }

        destination_addr = d_reg;          /* Destination register */
        dest_value = *d_reg;
        is_daddr_virtual = 0;
    }

    /* Indexed - Indexed;       Ex: MOV 0x0(Rs), 0x0(Rd) */
    /* Symbolic - Indexed;      Ex: MOV 0xS, 0x0(Rd)     */
    /* Indexed - Symbolic;      Ex: MOV 0x0(Rd), 0xD     */
    /* Symbolic - Symbolic;     Ex: MOV 0xS, 0xD         */
    /* Absolute - Indexed;      Ex: MOV &0xS, 0x0(Rd)    */
    /* Indexed - Absolute;      Ex: MOV 0x0(Rs), &0xD    */
    /* Absolute - Absolute;     Ex: MOV &0xS, &0xD       */
    /* Absolute - Symbolic;     Ex: MOV &0xS, 0xD        */
    /* Symbolic - Absolute;     Ex: MOV 0xS, &0xD        */
    /* Constant Gen - Indexed;  Ex: MOV #C, 0x0(Rd)      */ /* 1 */
    /* Constant Gen - Symbolic; Ex: MOV #C, 0xD          */ /* 1 */
    /* Constant Gen - Absolute; Ex: MOV #C, &0xD         */ /* 1 */
    else if (as_flag == 1 && ad_flag == 1) {
        if (constant_generator_active) {   /* Source Constant */
            source_value = immediate_constant;
            sprintf(asm_operands, "C#0x%04X, ", source_value);
            is_saddr_virtual = 0;
        }
        else if (source == 0) {            /* Source Symbolic */
            source_offset = fetch(cpu);
            uint16_t virtual_addr = cpu->pc + source_offset - 2;

            source_vaddress = virtual_addr;
            source_value = read_memory_cb(source_vaddress, bw_flag);
            is_saddr_virtual = 1;

            sprintf(hex_str_part, "%04X", (uint16_t) source_offset);
            strncat(hex_str, hex_str_part, sizeof hex_str);

            sprintf(asm_operands, "0x%04X, ", virtual_addr);
        }
        else if (source == 2) {            /* Source Absolute */
            source_offset = fetch(cpu);

            source_vaddress = source_offset;
            source_value = read_memory_cb(source_vaddress, bw_flag);
            is_saddr_virtual = 1;

            sprintf(hex_str_part, "%04X", (uint16_t) source_offset);
            strncat(hex_str, hex_str_part, sizeof hex_str);

            sprintf(asm_operands, "&0x%04X, ", (uint16_t) source_offset);
        }
        else {                             /* Source Indexed */
            source_offset = fetch(cpu);
            source_vaddress = *s_reg + source_offset;
            source_value = read_memory_cb(source_vaddress, bw_flag);
            is_saddr_virtual = 1;

            sprintf(hex_str_part, "%04X", (uint16_t)source_offset);
            strncat(hex_str, hex_str_part, sizeof hex_str);

            sprintf(asm_operands, "0x%04X(%s), ",
                    (uint16_t) source_offset, s_reg_name);
        }

        destination_offset = fetch(cpu);

        sprintf(hex_str_part, "%04X", (uint16_t) destination_offset);
        strncat(hex_str, hex_str_part, sizeof hex_str);

        if (destination == 0) {        /* Destination Symbolic */
            uint16_t virtual_addr = cpu->pc + destination_offset - 2;

            dest_vaddress = virtual_addr;
            sprintf(asm_op2, "0x%04X", virtual_addr);
        }
        else if (destination == 2) {   /* Destination Absolute */
            dest_vaddress = destination_offset;
            sprintf(asm_op2, "&0x%04X", (uint16_t) destination_offset);
        }
        else {                         /* Destination indexed */
            dest_vaddress = *d_reg + destination_offset;
            sprintf(asm_op2, "0x%04X(%s)", (uint16_t)destination_offset, d_reg_name);
        }

        is_daddr_virtual = 1;
        if (opcode != OP_MOV) {
            dest_value = read_memory_cb(dest_vaddress, bw_flag);
        }

        strncat(asm_operands, asm_op2, sizeof asm_op2);
    }

    /* Indirect - Register;     Ex: MOV @Rs, Rd */
    /* Constant Gen - Register; Ex: MOV #C, Rd  */ /* 2, 4 */
    else if (as_flag == 2 && ad_flag == 0) {
        if (constant_generator_active) {   /* Source Constant */
            source_value = immediate_constant;
            sprintf(asm_operands, "C#0x%04X, %s", immediate_constant, d_reg_name);
            is_saddr_virtual = 0;
        }
        else {                             /* Source Indirect */
            is_saddr_virtual = 1;
            source_vaddress = *s_reg;
            source_value = read_memory_cb(source_vaddress, bw_flag);
            sprintf(asm_operands, "@%s, %s", s_reg_name, d_reg_name);
        }

        if (destination == REG_PC) {
            consume_cycles_cb(constant_generator_active ? 1 : 2);
        }

        destination_addr = d_reg;          /* Destination Register */
        dest_value = *d_reg;
        is_daddr_virtual = 0;
    }

    /* Indirect - Indexed;      Ex: MOV @Rs, 0x0(Rd)   */
    /* Indirect - Symbolic;     Ex: MOV @Rs, 0xD       */
    /* Indirect - Absolute;     Ex: MOV @Rs, &0xD      */
    /* Constant Gen - Indexed;  Ex: MOV #C, 0x0(Rd)    */ /* 2, 4 */
    /* Constant Gen - Symbolic; Ex: MOV #C, 0xD        */ /* 2, 4 */
    /* Constant Gen - Absolute; Ex: MOV #C, &0xD       */ /* 2, 4 */
    else if (as_flag == 2 && ad_flag == 1) {
        destination_offset = fetch(cpu);

        sprintf(hex_str_part, "%04X", (uint16_t) destination_offset);
        strncat(hex_str, hex_str_part, sizeof hex_str);

        if (constant_generator_active) {   /* Source Constant */
            source_value = immediate_constant;
            is_saddr_virtual = 0;
            sprintf(asm_operands, "C#0x%04X, ", source_value);
        }
        else {                             /* Source Indirect */
            is_saddr_virtual = 1;
            source_vaddress = *s_reg;
            source_value = read_memory_cb(source_vaddress, bw_flag);

            sprintf(asm_operands, "@%s, ", s_reg_name);
        }

        if (destination == 0) {        /* Destination Symbolic */
            uint16_t virtual_addr = cpu->pc + destination_offset - 2;
            dest_vaddress = virtual_addr;

            sprintf(asm_op2, "0x%04X", virtual_addr);
        }
        else if (destination == 2) {   /* Destination Absolute */
            dest_vaddress = destination_offset;

            sprintf(asm_op2, "&0x%04X", destination_offset);
        }
        else {                         /* Destination Indexed */
            dest_vaddress = *d_reg + destination_offset;

            sprintf(asm_op2,
                    "0x%04X(%s)",
                    (uint16_t)destination_offset,
                    d_reg_name);
        }

        is_daddr_virtual = 1;
        if (opcode != OP_MOV) {
            dest_value = read_memory_cb(dest_vaddress, bw_flag);
        }

        strncat(asm_operands, asm_op2, sizeof asm_op2);
    }

    /* Indirect Inc - Register; Ex: MOV @Rs+, Rd */
    /* Immediate - Register;    Ex: MOV #S, Rd   */
    /* Constant Gen - Register; Ex: MOV #C, Rd   */ /* -1, 8 */
    else if (as_flag == 3 && ad_flag == 0) {
        if (destination == REG_PC) {
            consume_cycles_cb(1);
        }
        if (constant_generator_active) {   /* Source Constant */
            source_value = immediate_constant;
            is_saddr_virtual = 0;

            sprintf(asm_operands, "C#0x%04X, %s",
                    (uint16_t) source_value, d_reg_name);
        }
        else if (source == 0) {            /* Source Immediate */
            source_value = fetch(cpu);
            is_saddr_virtual = 0;

            sprintf(hex_str_part, "%04X", (uint16_t) source_value);
            strncat(hex_str, hex_str_part, sizeof hex_str);

            if (bw_flag == WORD) {
                sprintf(asm_operands, "#0x%04X, %s",
                        (uint16_t) source_value, d_reg_name);
            }
            else if (bw_flag == BYTE) {
                sprintf(asm_operands, "#0x%04X, %s",
                        (uint8_t) source_value, d_reg_name);
            }
        }
        else {                              /* Source Indirect Auto Increment */
            is_saddr_virtual = 1;
            source_vaddress = *s_reg;
            source_value = read_memory_cb(source_vaddress, bw_flag);
            if (destination == REG_PC) {
                consume_cycles_cb(1);
            }

            sprintf(asm_operands, "@%s+, %s", s_reg_name, d_reg_name);

            *s_reg += bw_flag ? 1 : 2;
        }

        is_daddr_virtual = 0;
        destination_addr = d_reg;           /* Destination Register */
        dest_value = *d_reg;
    }

    /* Indirect Inc - Indexed;  Ex: MOV @Rs+, 0x0(Rd) */
    /* Indirect Inc - Symbolic; Ex: MOV @Rs+, 0xD     */
    /* Indirect Inc - Absolute; Ex: MOV @Rs+, &0xD    */
    /* Immediate - Indexed;     Ex: MOV #S, 0x0(Rd)   */
    /* Immediate - Symbolic;    Ex: MOV #S, 0xD       */
    /* Immediate - Absolute;    Ex: MOV #S, &0xD      */
    /* Constant Gen - Indexed;  Ex: MOV #C, 0x0(Rd)   */ /* -1, 8 */
    /* Constant Gen - Symbolic; Ex: MOV #C, 0xD       */ /* -1, 8 */
    /* Constant Gen - Absolute; Ex: MOV #C, &0xD      */ /* -1, 8 */
    else if (as_flag == 3 && ad_flag == 1) {
        if (constant_generator_active) {   /* Source Constant */
            source_value = immediate_constant;
            is_saddr_virtual = 0;
            sprintf(asm_operands, "C#0x%04X, ", (uint16_t)source_value);
        }
        else if (source == 0) {            /* Source Immediate */
            source_value = fetch(cpu);
            is_saddr_virtual = 0;

            sprintf(hex_str_part, "%04X", (uint16_t) source_value);
            strncat(hex_str, hex_str_part, sizeof hex_str);

            sprintf(asm_operands, "#0x%04X, ", (uint16_t)source_value);
        }
        else {                             /* Source Indirect Auto Increment */
            is_saddr_virtual = 1;
            source_vaddress = *s_reg;
            source_value = read_memory_cb(source_vaddress, bw_flag);

            sprintf(asm_operands, "@%s+, ", s_reg_name);

            *s_reg += bw_flag ? 1 : 2;
        }

        destination_offset = fetch(cpu);

        sprintf(hex_str_part, "%04X", (uint16_t) destination_offset);
        strncat(hex_str, hex_str_part, sizeof hex_str);

        if (destination == 0) {        /* Destination Symbolic */
            uint16_t virtual_addr = cpu->pc + destination_offset - 2;
            dest_vaddress = virtual_addr;
            sprintf(asm_op2, "0x%04X", virtual_addr);
        }
        else if (destination == 2) {   /* Destination Absolute */
            dest_vaddress = destination_offset;
            sprintf(asm_op2, "&0x%04X", (uint16_t) destination_offset);
        }
        else {                         /* Destination Indexed */
            dest_vaddress = *d_reg + destination_offset;

            sprintf(asm_op2, "0x%04X(%s)",
                    (uint16_t) destination_offset, d_reg_name);
        }

        is_daddr_virtual = 1;
        if (opcode != OP_MOV) {
            dest_value = read_memory_cb(dest_vaddress, bw_flag);
        }

        strncat(asm_operands, asm_op2, sizeof asm_op2);
    }


    int16_t result;
    switch (opcode) {

    /* MOV SOURCE, DESTINATION
       *   Ex: MOV #4, R6
       *
       * SOURCE = DESTINATION
       *
       * The source operand is moved to the destination. The source operand is
       * not affected. The previous contents of the destination are lost.
       *
       */
    case 0x4: {

        result = bw_flag ? source_value & 0xFF : source_value;

        if (is_daddr_virtual){
            write_memory_cb(dest_vaddress, result, bw_flag);
        } else {
            *destination_addr = result;
        }

        break;
    }

        /* ADD SOURCE, DESTINATION
       *   Ex: ADD R5, R4
       *
       * The source operand is added to the destination operand. The source op
       * is not affected. The previous contents of the destination are lost.
       *
       * DESTINATION = SOURCE + DESTINATION
       *
       * N: Set if result is negative, reset if positive
       * Z: Set if result is zero, reset otherwise
       * C: Set if there is a carry from the result, cleared if not
       * V: Set if an arithmetic overflow occurs, otherwise reset
       *
       */
    case 0x5:{

        if (bw_flag) {
            dest_value = truncate_byte(dest_value);
            source_value = truncate_byte(source_value);
        }

        result = dest_value + source_value;

        if (is_daddr_virtual){
            write_memory_cb(dest_vaddress, result, bw_flag);
        } else {
            *destination_addr = result;
        }

        bool c, z, n, v;
        z = is_zero(result, bw_flag);
        n = is_negative(result, bw_flag);
        c = is_add_carry(dest_value, source_value, 0, bw_flag);
        v = is_add_overflow(dest_value, source_value, 0, bw_flag);
        set_sr_flags(cpu, c, z, n, v);
        break;
    }

        /* ADDC SOURCE, DESTINATION
       *   Ex: ADDC R5, R4
       *
       * DESTINATION += (SOURCE + C)
       *
       * N: Set if result is negative, reset if positive
       * Z: Set if result is zero, reset otherwise
       * C: Set if there is a carry from the result, cleared if not
       * V: Set if an arithmetic overflow occurs, otherwise reset
       *
       */
    case 0x6:{

        if (bw_flag) {
            dest_value = truncate_byte(dest_value);
            source_value = truncate_byte(source_value);
        }

        result = source_value + dest_value + get_carry(cpu);

        if (is_daddr_virtual){
            write_memory_cb(dest_vaddress, result, bw_flag);
        } else {
            *destination_addr = bw_flag ? result & 0xFF : result;
        }

        bool c, z, n, v;
        z = is_zero(result, bw_flag);
        n = is_negative(result, bw_flag);
        c = is_add_carry(
                    dest_value, source_value, get_carry(cpu), bw_flag);
        v = is_add_overflow(
                    dest_value, source_value, get_carry(cpu), bw_flag);
        set_sr_flags(cpu, c, z, n, v);

        break;
    }

        /* SUBC SOURCE, DESTINATION
       *   Ex: SUB R4, R5
       *
       *   DST += ~SRC + C
       *
       *  N: Set if result is negative, reset if positive
       *  Z: Set if result is zero, reset otherwise
       *  C: Set if there is a carry from the MSB of the result, reset otherwise.
       *     Set to 1 if no borrow, reset if borrow.
       *  V: Set if an arithmetic overflow occurs, otherwise reset
       *
       *
       */
    case 0x7:{

        if (bw_flag) {
            dest_value = truncate_byte(dest_value);
            source_value = truncate_byte(source_value);
        }

        result = dest_value - (source_value - 1) + get_carry(cpu);

        if (is_daddr_virtual){
            write_memory_cb(dest_vaddress, result, bw_flag);
        } else {
            *destination_addr = bw_flag ? result & 0xFF : result;
        }

        bool c, z, n, v;
        z = is_zero(result, bw_flag);
        n = is_negative(result, bw_flag);
        c = is_sub_carry(
                    dest_value, source_value, get_carry(cpu));
        v = is_sub_overflow(
                    dest_value, source_value, get_carry(cpu), bw_flag);
        set_sr_flags(cpu, c, z, n, v);
        break;
    }

        /* SUB SOURCE, DESTINATION
       *   Ex: SUB R4, R5
       *
       *   DST -= SRC
       *
       *  N: Set if result is negative, reset if positive
       *  Z: Set if result is zero, reset otherwise
       *  C: Set if there is a carry from the MSB of the result, reset otherwise.
       *     Set to 1 if no borrow, reset if borrow.
       *  V: Set if an arithmetic overflow occurs, otherwise reset
       */

    case 0x8:{

        if (bw_flag) {
            dest_value = truncate_byte(dest_value);
            source_value = truncate_byte(source_value);
        }

        result = dest_value - source_value;

        if (is_daddr_virtual){
            write_memory_cb(dest_vaddress, result, bw_flag);
        } else {
            *destination_addr = bw_flag ? result & 0xFF : result;
        }

        bool c, z, n, v;
        z = is_zero(result, bw_flag);
        n = is_negative(result, bw_flag);
        c = is_sub_carry(dest_value, source_value, 1);
        v = is_sub_overflow(dest_value, source_value, 1, bw_flag);
        set_sr_flags(cpu, c, z, n, v);
        break;
    }

        /* CMP SOURCE, DESTINATION
       *
       * N: Set if result is negative, reset if positive (src ≥ dst)
       * Z: Set if result is zero, reset otherwise (src = dst)
       * C: Set if there is a carry from the MSB of the result, reset otherwise
       * V: Set if an arithmetic overflow occurs, otherwise reset
       * TODO: Fix overflow error
       */
    case 0x9:{

        if (bw_flag) {
            dest_value = truncate_byte(dest_value);
            source_value = truncate_byte(source_value);
        }

        result = dest_value - source_value;

        bool c, z, n, v;
        n = is_negative(result, bw_flag);
        z = is_zero(result, bw_flag);
        c = is_sub_carry(dest_value, source_value, 1);
        v = is_sub_overflow(dest_value, source_value, 1, bw_flag);
        set_sr_flags(cpu, c, z, n, v);
        break;
    }

        /* DADD SOURCE, DESTINATION
       *
       */
    case 0xA:{
        fprintf(stderr, "ERROR: DADD INSTRUCTION NOT IMPLEMENTED");
        exit(1);
        if (bw_flag == WORD) {

        }
        else if (bw_flag == BYTE) {

        }

        break;
    }

        /* BIT SOURCE, DESTINATION
       *
       * N: Set if MSB of result is set, reset otherwise
       * Z: Set if result is zero, reset otherwise
       * C: Set if result is not zero, reset otherwise (.NOT. Zero)
       * V: Reset
      */
    case 0xB:{

        bool c, z, n, v;
        result = source_value & dest_value;
        n = is_negative(result, bw_flag);
        z = is_zero(result, bw_flag);
        c = !z;
        v = false;
        set_sr_flags(cpu, c, z, n, v);

        break;
    }

        /* BIC SOURCE, DESTINATION
       *
       * No status bits affected
       */
    case 0xC:{

        result = dest_value & ~source_value;

        if (is_daddr_virtual){
            write_memory_cb(dest_vaddress, result, bw_flag);
        } else {
            *destination_addr = bw_flag ? result & 0xFF : result;
        }

        break;
    }

        /* BIS SOURCE, DESTINATION
       * No flags affected
       */
    case 0xD:{

        result = dest_value | source_value;

        if (is_daddr_virtual){
            write_memory_cb(dest_vaddress, result, bw_flag);
        } else {
            *destination_addr = bw_flag ? result & 0xFF : result;
        }

        break;
    }

        /* XOR SOURCE, DESTINATION
       *
       * N: Set if result MSB is set, reset if not set
       * Z: Set if result is zero, reset otherwise
       * C: Set if result is not zero, reset otherwise ( = .NOT. Zero)
       * V: Set if both operands are negative
       */
    case 0xE:{

        result = dest_value ^ source_value;

        bool c, z, n, v;
        n = is_negative(result, bw_flag);
        z = is_zero(result, bw_flag);
        c = !z;
        v = is_negative(dest_value, bw_flag) &&
                is_negative(source_value, bw_flag);
        set_sr_flags(cpu, c, z, n, v);

        if (is_daddr_virtual){
            write_memory_cb(dest_vaddress, result, bw_flag);
        } else {
            *destination_addr = bw_flag ? result & 0xFF : result;
        }

        break;
    }

        /* AND SOURCE, DESTINATION
       *
       *  N: Set if result MSB is set, reset if not set
       *  Z: Set if result is zero, reset otherwise
       *  C: Set if result is not zero, reset otherwise ( = .NOT. Zero)
       *  V: Reset
       */
    case 0xF:{

        result = dest_value & source_value;

        bool c, z, n, v;
        n = is_negative(result, bw_flag);
        z = is_zero(result, bw_flag);
        c = !z;
        v = false;
        set_sr_flags(cpu, c, z, n, v);

        if (is_daddr_virtual){
            write_memory_cb(dest_vaddress, result, bw_flag);
        } else {
            *destination_addr = bw_flag ? result & 0xFF : result;
        }

        break;
    }
    default: {
        fprintf(stderr, "INVALID FORMAT I OPCODE, EXITING.");
        exit(1);
    }

    } //# End of switch

    if (disas != NULL) {
        switch (opcode) {
        case 0x4: {
            bw_flag == WORD ?
                        strncpy(disas, "MOV", DISAS_STR_LEN) :
                        strncpy(disas, "MOV.B", DISAS_STR_LEN);

            break;
        }
        case 0x5: {
            bw_flag == WORD ?
                        strncpy(disas, "ADD", DISAS_STR_LEN) :
                        strncpy(disas, "ADD.B", DISAS_STR_LEN);

            break;
        }
        case 0x6: {
            bw_flag == WORD ?
                        strncpy(disas, "ADDC", DISAS_STR_LEN) :
                        strncpy(disas, "ADDC.B", DISAS_STR_LEN);

            break;
        }
        case 0x7: {
            bw_flag == WORD ?
                        strncpy(disas, "SUBC", DISAS_STR_LEN) :
                        strncpy(disas, "SUBC.B", DISAS_STR_LEN);

            break;
        }
        case 0x8: {
            bw_flag == WORD ?
                        strncpy(disas, "SUB", DISAS_STR_LEN) :
                        strncpy(disas, "SUB.B", DISAS_STR_LEN);

            break;
        }
        case 0x9: {
            bw_flag == WORD ?
                        strncpy(disas, "CMP", DISAS_STR_LEN) :
                        strncpy(disas, "CMP.B", DISAS_STR_LEN);

            break;
        }
        case 0xA: {
            bw_flag == WORD ?
                        strncpy(disas, "DADD", DISAS_STR_LEN) :
                        strncpy(disas, "DADD.B", DISAS_STR_LEN);

            break;
        }
        case 0xB: {
            bw_flag == WORD ?
                        strncpy(disas, "BIT", DISAS_STR_LEN) :
                        strncpy(disas, "BIT.B", DISAS_STR_LEN);

            break;
        }
        case 0xC: {
            bw_flag == WORD ?
                        strncpy(disas, "BIC", DISAS_STR_LEN) :
                        strncpy(disas, "BIC.B", DISAS_STR_LEN);

            break;
        }
        case 0xD: {
            bw_flag == WORD ?
                        strncpy(disas, "BIS", DISAS_STR_LEN) :
                        strncpy(disas, "BIS.B", DISAS_STR_LEN);

            break;
        }
        case 0xE: {
            bw_flag == WORD ?
                        strncpy(disas, "XOR", DISAS_STR_LEN) :
                        strncpy(disas, "XOR.B", DISAS_STR_LEN);

            break;
        }
        case 0xF: {
            bw_flag == WORD ?
                        strncpy(disas, "AND", DISAS_STR_LEN) :
                        strncpy(disas, "AND.B", DISAS_STR_LEN);

            break;
        }

        } //# End of switch

        strncat(disas, " ", DISAS_STR_LEN);
        strncat(disas, asm_operands, DISAS_STR_LEN);
    }
}
