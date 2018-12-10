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

void decode_formatI(Emulator *emu, uint16_t instruction, bool disassemble)
{
    int is_saddr_virtual;
    int is_daddr_virtual;
    uint16_t source_vaddress;
    uint16_t dest_vaddress;
    Cpu *cpu = emu->cpu;
    Debugger *debugger = emu->debugger;

    uint8_t opcode = (instruction & 0xF000) >> 12;
    uint8_t source = (instruction & 0x0F00) >> 8;
    uint8_t as_flag = (instruction & 0x0030) >> 4;
    uint8_t destination = (instruction & 0x000F);
    uint8_t ad_flag = (instruction & 0x0080) >> 7;
    uint8_t bw_flag = (instruction & 0x0040) >> 6;

    char s_reg_name[10], d_reg_name[10];

    char mnemonic[100] = {0};
    /* String to show hex value of instruction */
    char hex_str[100] = {0};
    char hex_str_part[10] = {0};

    sprintf(hex_str, "%04X", instruction);

    /* Source Register pointer */
    int16_t *s_reg = get_reg_ptr(emu, source);

    /* Destination Register pointer */
    int16_t *d_reg = get_reg_ptr(emu, destination);

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
    char asm_operands[20] = {0}, asm_op2[20] = {0};

    /* Register - Register;     Ex: MOV Rs, Rd */
    /* Constant Gen - Register; Ex: MOV #C, Rd */ /* 0 */
    if (as_flag == 0 && ad_flag == 0) {
        if (constant_generator_active) {   /* Source Constant */
            source_value = immediate_constant;

            sprintf(asm_operands, "#0x%04X, %s",
                    (uint16_t) source_value, d_reg_name);
        }
        else {                             /* Source register */
            source_value = *s_reg;
            sprintf(asm_operands, "%s, %s", s_reg_name, d_reg_name);
        }

        if (!disassemble) {
            bw_flag == BYTE ? *d_reg &= 0x00FF : 0;
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
        destination_offset = fetch(emu);

        sprintf(hex_str_part, "%04X", (uint16_t) destination_offset);
        strncat(hex_str, hex_str_part, sizeof hex_str);

        if (constant_generator_active) {   /* Source Constant */
            source_value = immediate_constant;
            sprintf(asm_operands, "#0x%04X, ", source_value);
        }
        else {                             /* Source from register */
            source_value = *s_reg;
            sprintf(asm_operands, "%s, ", s_reg_name);
        }

        if (destination == 0) {            /* Destination Symbolic */
            uint16_t virtual_addr = *d_reg + destination_offset - 2;
            destination_addr = get_addr_ptr(virtual_addr);
            dest_vaddress = virtual_addr;

            sprintf(asm_op2, "0x%04X", (uint16_t) virtual_addr);
        }
        else if (destination == 2) {       /* Destination Absolute */
            destination_addr = get_addr_ptr(destination_offset);
            dest_vaddress = destination_offset;
            sprintf(asm_op2, "&0x%04X", (uint16_t) destination_offset);
        }
        else {                             /* Destination Indexed */
            sprintf(asm_op2, "0x%04X(%s)",
                    (uint16_t) destination_offset, d_reg_name);

            destination_addr = get_addr_ptr(*d_reg + destination_offset);
            dest_vaddress = (*d_reg + destination_offset);
        }

        dest_value = read_memory_cb(dest_vaddress, bw_flag ? BYTE : WORD);
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
            sprintf(asm_operands, "#0x%04X, %s", source_value, d_reg_name);
            is_saddr_virtual = 0;
        }
        else if (source == 0) {            /* Source Symbolic */
            source_offset = fetch(emu);
            uint16_t virtual_addr = *s_reg + source_offset - 2;

//            source_value = *get_addr_ptr(virtual_addr);
            source_vaddress = virtual_addr;
            source_value = read_memory_cb(source_vaddress,
                                          bw_flag ? BYTE : WORD);
            is_saddr_virtual = 1;

            sprintf(hex_str_part, "%04X", (uint16_t) source_offset);
            strncat(hex_str, hex_str_part, sizeof hex_str);
            sprintf(asm_operands, "0x%04X, %s", virtual_addr, d_reg_name);
        }
        else if (source == 2) {            /* Source Absolute */
            source_offset = fetch(emu);
//            source_value = *get_addr_ptr(source_offset);
            source_vaddress = source_offset;
            source_value = read_memory_cb(source_vaddress,
                                          bw_flag ? BYTE : WORD);
            is_saddr_virtual = 1;


            sprintf(hex_str_part, "%04X", (uint16_t) source_offset);
            strncat(hex_str, hex_str_part, sizeof hex_str);

            sprintf(asm_operands, "&0x%04X, %s",
                    (uint16_t) source_offset, d_reg_name);
        }
        else {                             /* Source Indexed */
            source_offset = fetch(emu);
//            source_value = *get_addr_ptr(*s_reg + source_offset);

            source_vaddress = *s_reg + source_offset;
            source_value = read_memory_cb(source_vaddress,
                                          bw_flag ? BYTE : WORD);
            is_saddr_virtual = 1;

            sprintf(hex_str_part, "%04X", (uint16_t) source_offset);
            strncat(hex_str, hex_str_part, sizeof hex_str);

            sprintf(asm_operands, "0x%04X(%s), %s",
                    (uint16_t) source_offset, s_reg_name, d_reg_name);
        }

        if (!disassemble) {
            bw_flag == BYTE ? *d_reg &= 0x00FF : 0;
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
            sprintf(asm_operands, "#0x%04X, ", source_value);
            is_saddr_virtual = 0;
        }
        else if (source == 0) {            /* Source Symbolic */
            source_offset = fetch(emu);
            uint16_t virtual_addr = cpu->pc + source_offset - 4;

//            source_value = *get_addr_ptr(virtual_addr);
            source_vaddress = virtual_addr;
            source_value = read_memory_cb(source_vaddress,
                                          bw_flag ? BYTE : WORD);
            is_saddr_virtual = 1;

            sprintf(hex_str_part, "%04X", (uint16_t) source_offset);
            strncat(hex_str, hex_str_part, sizeof hex_str);

            sprintf(asm_operands, "0x%04X, ", virtual_addr);
        }
        else if (source == 2) {            /* Source Absolute */
            source_offset = fetch(emu);
//            source_value = *get_addr_ptr(source_offset);

            source_vaddress = source_offset;
            source_value = read_memory_cb(source_vaddress,
                                          bw_flag ? BYTE : WORD);
            is_saddr_virtual = 1;

            sprintf(hex_str_part, "%04X", (uint16_t) source_offset);
            strncat(hex_str, hex_str_part, sizeof hex_str);

            sprintf(asm_operands, "&0x%04X, ", (uint16_t) source_offset);
        }
        else {                             /* Source Indexed */
            source_offset = fetch(emu);
//            source_value = *get_addr_ptr(*s_reg + source_offset);
            source_vaddress = *s_reg + source_offset;
            source_value = read_memory_cb(source_vaddress,
                                          bw_flag ? BYTE : WORD);
            is_saddr_virtual = 1;

            sprintf(hex_str_part, "%04X", (uint16_t)source_offset);
            strncat(hex_str, hex_str_part, sizeof hex_str);

            sprintf(asm_operands, "0x%04X(%s), ",
                    (uint16_t) source_offset, s_reg_name);
        }

        destination_offset = fetch(emu);

        sprintf(hex_str_part, "%04X", (uint16_t) destination_offset);
        strncat(hex_str, hex_str_part, sizeof hex_str);

        if (destination == 0) {        /* Destination Symbolic */
            uint16_t virtual_addr = cpu->pc + destination_offset - 2;

            destination_addr = get_addr_ptr(virtual_addr);
            dest_vaddress = virtual_addr;
            dest_value = read_memory_cb(dest_vaddress, bw_flag ? BYTE : WORD);
            sprintf(asm_op2, "0x%04X", virtual_addr);
        }
        else if (destination == 2) {   /* Destination Absolute */
            destination_addr = get_addr_ptr(destination_offset);
            dest_vaddress = destination_offset;
            dest_value = read_memory_cb(dest_vaddress, bw_flag ? BYTE : WORD);
            sprintf(asm_op2, "&0x%04X", (uint16_t) destination_offset);
        }
        else {                         /* Destination indexed */
            destination_addr = get_addr_ptr(*d_reg + destination_offset);
            dest_vaddress = *d_reg + destination_offset;
            dest_value = read_memory_cb(dest_vaddress, bw_flag ? BYTE : WORD);
            sprintf(asm_op2, "0x%04X(%s)", (uint16_t)destination_offset, d_reg_name);
        }

        is_daddr_virtual = 1;

        strncat(asm_operands, asm_op2, sizeof asm_op2);
    }

    /* Indirect - Register;     Ex: MOV @Rs, Rd */
    /* Constant Gen - Register; Ex: MOV #C, Rd  */ /* 2, 4 */
    else if (as_flag == 2 && ad_flag == 0) {
        if (constant_generator_active) {   /* Source Constant */
            source_value = immediate_constant;
            sprintf(asm_operands, "#0x%04X, %s", immediate_constant, d_reg_name);
            is_saddr_virtual = 0;
        }
        else {                             /* Source Indirect */
//            source_value = *get_addr_ptr(*s_reg);
            is_saddr_virtual = 1;
            source_vaddress = *s_reg;
            source_value = read_memory_cb(source_vaddress,
                                          bw_flag ? BYTE : WORD);
            sprintf(asm_operands, "@%s, %s", s_reg_name, d_reg_name);
        }

        if (!disassemble) {
            bw_flag == BYTE ? *d_reg &= 0x00FF : 0;
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
        destination_offset = fetch(emu);

        sprintf(hex_str_part, "%04X", (uint16_t) destination_offset);
        strncat(hex_str, hex_str_part, sizeof hex_str);

        if (constant_generator_active) {   /* Source Constant */
            source_value = immediate_constant;
            is_saddr_virtual = 0;
            sprintf(asm_operands, "#0x%04X, ", source_value);
        }
        else {                             /* Source Indirect */
//            source_value = *get_addr_ptr(*s_reg);
            is_saddr_virtual = 1;
            source_vaddress = *s_reg;
            source_value = read_memory_cb(source_vaddress,
                                          bw_flag ? BYTE : WORD);

            sprintf(asm_operands, "@%s, ", s_reg_name);
        }

        if (destination == 0) {        /* Destination Symbolic */
            uint16_t virtual_addr = cpu->pc + destination_offset - 2;

            dest_vaddress = virtual_addr;
            dest_value = read_memory_cb(dest_vaddress, bw_flag ? BYTE : WORD);

            destination_addr = get_addr_ptr(virtual_addr);
            sprintf(asm_op2, "0x%04X", virtual_addr);
        }
        else if (destination == 2) {   /* Destination Absolute */
            destination_addr = get_addr_ptr(destination_offset);

            dest_vaddress = destination_offset;
            dest_value = read_memory_cb(dest_vaddress, bw_flag ? BYTE : WORD);

            sprintf(asm_op2, "&0x%04X", destination_offset);
        }
        else {                         /* Destination Indexed */
            destination_addr = get_addr_ptr(*d_reg + destination_offset);

            dest_vaddress = *d_reg + destination_offset;
            dest_value = read_memory_cb(dest_vaddress, bw_flag ? BYTE : WORD);

            sprintf(asm_op2, "0x%04X(%s)", (uint16_t)destination_offset, d_reg_name);
        }

        is_daddr_virtual = 1;

        strncat(asm_operands, asm_op2, sizeof asm_op2);
    }

    /* Indirect Inc - Register; Ex: MOV @Rs+, Rd */
    /* Immediate - Register;    Ex: MOV #S, Rd   */
    /* Constant Gen - Register; Ex: MOV #C, Rd   */ /* -1, 8 */
    else if (as_flag == 3 && ad_flag == 0) {
        if (constant_generator_active) {   /* Source Constant */
            source_value = immediate_constant;
            is_saddr_virtual = 0;
            sprintf(asm_operands, "#0x%04X, %s",
                    (uint16_t) source_value, d_reg_name);
        }
        else if (source == 0) {            /* Source Immediate */
            source_value = fetch(emu);
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
        else {                              /* Source Indirect AutoIncrement */
//            source_value = *get_addr_ptr(*s_reg);

            is_saddr_virtual = 1;
            source_vaddress = *s_reg;
            source_value = read_memory_cb(source_vaddress,
                                          bw_flag ? BYTE : WORD);

            sprintf(asm_operands, "@%s+, %s", s_reg_name, d_reg_name);

            if (!disassemble) {
                bw_flag == WORD ? *s_reg += 2 : (*s_reg += 1);
            }
        }

        if (!disassemble) {
            bw_flag == BYTE ? *d_reg &= 0x00FF : 0;
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
            sprintf(asm_operands, "#0x%04X, ", (uint16_t)source_value);
        }
        else if (source == 0) {            /* Source Immediate */
            source_value = fetch(emu);
            is_saddr_virtual = 0;

            sprintf(hex_str_part, "%04X", (uint16_t) source_value);
            strncat(hex_str, hex_str_part, sizeof hex_str);

            sprintf(asm_operands, "#0x%04X, ", (uint16_t)source_value);
        }
        else {                             /* Source Indirect Auto Increment */
//            source_value = *get_addr_ptr(*s_reg);
            is_saddr_virtual = 1;
            source_vaddress = *s_reg;
            source_value = read_memory_cb(source_vaddress,
                                          bw_flag ? BYTE : WORD);

            sprintf(asm_operands, "@%s+, ", s_reg_name);

            if (!disassemble) {
                bw_flag == WORD ? *s_reg += 2 : (*s_reg += 1);
            }
        }

        destination_offset = fetch(emu);

        sprintf(hex_str_part, "%04X", (uint16_t) destination_offset);
        strncat(hex_str, hex_str_part, sizeof hex_str);

        if (destination == 0) {        /* Destination Symbolic */
            uint16_t virtual_addr = cpu->pc + destination_offset - 2;
            dest_vaddress = virtual_addr;
            destination_addr = get_addr_ptr(virtual_addr);
            sprintf(asm_op2, "0x%04X", virtual_addr);
        }
        else if (destination == 2) {   /* Destination Absolute */
            destination_addr = get_addr_ptr(destination_offset);
            dest_vaddress = destination_offset;
            sprintf(asm_op2, "&0x%04X", (uint16_t) destination_offset);
        }
        else {                         /* Destination Indexed */
            destination_addr = get_addr_ptr(*d_reg + destination_offset);
            dest_vaddress = *d_reg + destination_offset;

            sprintf(asm_op2, "0x%04X(%s)",
                    (uint16_t) destination_offset, d_reg_name);
        }

        // Fetch destination value
        dest_value = read_memory_cb(dest_vaddress, bw_flag ? BYTE : WORD);

        is_daddr_virtual = 1;

        strncat(asm_operands, asm_op2, sizeof asm_op2);
    }


    if (!disassemble) {
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
                write_memory_cb(dest_vaddress, result, bw_flag ? BYTE : WORD);
            } else {
                if (bw_flag==WORD){
                    *destination_addr = result;
                } else if (bw_flag == BYTE){
                    *((uint8_t *) destination_addr) = (uint8_t) result;
                }
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
            result = bw_flag ?
                        (uint8_t)((uint8_t)dest_value + (uint8_t)source_value)
                      : dest_value + source_value;

            if (is_daddr_virtual){
                write_memory_cb(dest_vaddress, result, bw_flag ? BYTE : WORD);
            } else {
                if (bw_flag==WORD){
                    *destination_addr = result;
                } else if (bw_flag == BYTE){
                    *((uint8_t *) destination_addr) = (uint8_t) result;
                }
            }

            bool c, z, n, v;
            z = result==0;
            n = result<0;
            c = is_carried(dest_value, source_value, bw_flag);

            if (dest_value>0 && source_value>0){
                v = result<0;
            } else if (dest_value<0 && source_value<0){
                v = result>0;
            } else {
                v = false;
            }

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

            result = bw_flag ? (uint8_t)source_value + get_carry(cpu) :
                               source_value + get_carry(cpu);

            if (is_daddr_virtual){
                write_memory_cb(dest_vaddress, result, bw_flag ? BYTE : WORD);
            } else {
                if (bw_flag==WORD){
                    *destination_addr = result;
                } else if (bw_flag == BYTE){
                    *((uint8_t *) destination_addr) = (uint8_t) result;
                }
            }

            bool c, z, n, v;
            z = result==0;
            n = result<0;
            c = is_carried(dest_value, source_value, bw_flag);

            if (dest_value>0 && source_value>0){
                v = result<0;
            } else if (dest_value<0 && source_value<0){
                v = result>0;
            } else {
                v = false;
            }

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

            if (bw_flag==WORD) {
                result = dest_value - (source_value-1) + get_carry(cpu);
            } else if (bw_flag == BYTE){
                result = (int8_t)((int8_t)dest_value
                                  - ((int8_t)source_value - 1)
                                  + get_carry(cpu));
            }

            if (is_daddr_virtual){
                write_memory_cb(dest_vaddress, result, bw_flag ? BYTE : WORD);
            } else {
                if (bw_flag==WORD){
                    *destination_addr = result;
                } else if (bw_flag == BYTE){
                    *((uint8_t *) destination_addr) = (uint8_t) result;
                }
            }

            bool c, z, n, v;
            z = result==0;
            n = result<0;
            c = is_carried(dest_value, source_value, bw_flag);

            if (source_value < 0 && dest_value > 0){
                v = result < 0;
            } else if (source_value > 0 && dest_value < 0){
                v = result > 0;
            } else {
                v = false;
            }
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

            result = bw_flag ?
                        (int8_t)((int8_t)dest_value - (int8_t)source_value)
                      : dest_value - source_value;

            if (is_daddr_virtual){
                write_memory_cb(dest_vaddress, result, bw_flag ? BYTE : WORD);
            } else {
                if (bw_flag==WORD){
                    *destination_addr = result;
                } else if (bw_flag == BYTE){
                    *((uint8_t *) destination_addr) = (uint8_t) result;
                }
            }

            bool c, z, n, v;
            z = result==0;
            n = result<0;

            c = is_carried(dest_value, source_value, bw_flag);

            if (source_value < 0 && dest_value > 0){
                v = result < 0;
            } else if (source_value > 0 && dest_value < 0){
                v = result > 0;
            } else {
                v = false;
            }

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

            int16_t tmp_src, tmp_dst, result;

            tmp_src = bw_flag ? (int8_t)source_value : source_value;
            tmp_dst = bw_flag ? (int8_t)dest_value : dest_value;

            result = tmp_src - tmp_dst;

            bool c, z, n, v;

            n = result<0;
            z = result==0;


            if(bw_flag == WORD){
                unsigned tmp = ((uint16_t)~source_value) + 1 +
                                (uint16_t)dest_value;
                c = (tmp > 0xffff);
            } else if (bw_flag == BYTE){
                unsigned tmp = ((uint8_t)~source_value)+1+(uint8_t)dest_value;
                c = (tmp > 0xff);
            }

            if (tmp_src < 0 && tmp_dst > 0){
                v = result < 0;
            } else if (tmp_src > 0 && tmp_dst < 0){
                v = result > 0;
            } else {
                v = false;
            }
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

            if (bw_flag == WORD) {
                result = source_value & dest_value;
                set_sr_flags(cpu,
                             result != 0,
                             result == 0,
                             (result>>15)>0,
                             false);
            }
            else if (bw_flag == BYTE) {
                uint8_t res = (uint8_t) source_value & (uint8_t)dest_value;
                set_sr_flags(cpu,
                             res != 0,
                             res == 0,
                             (res>>7)>0,
                             false);
            }

            break;
        }

            /* BIC SOURCE, DESTINATION
       *
       * No status bits affected
       */
        case 0xC:{

            if (bw_flag == WORD) {
//                *destination_addr &= (uint16_t) ~source_value;
                result = dest_value & source_value;
            }
            else if (bw_flag == BYTE) {
//                *(uint8_t *) destination_addr &= (uint8_t) ~source_value;
                result = (dest_value & source_value) & 0x00FF;
            }

            if (is_daddr_virtual){
                write_memory_cb(dest_vaddress, result, bw_flag ? BYTE : WORD);
            } else {
                if (bw_flag==WORD){
                    *destination_addr = result;
                } else if (bw_flag == BYTE){
                    *((uint8_t *) destination_addr) = (uint8_t) result;
                }
            }

            break;
        }

            /* BIS SOURCE, DESTINATION
       * No flags affected
       */
        case 0xD:{

            if (bw_flag == WORD) {
                result = dest_value | source_value;
            }
            else if (bw_flag == BYTE) {
                result = (dest_value | source_value) & 0x00FF;
            }

            if (is_daddr_virtual){
                write_memory_cb(dest_vaddress, result, bw_flag ? BYTE : WORD);
            } else {
                if (bw_flag==WORD){
                    *destination_addr = result;
                } else if (bw_flag == BYTE){
                    *((uint8_t *) destination_addr) = (uint8_t) result;
                }
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

            if (bw_flag == WORD) {
                result = dest_value ^ source_value;
                set_sr_flags(cpu,
                             result != 0,
                             result == 0,
                             result < 0,
                             (dest_value<0) && (source_value<0));
            }
            else if (bw_flag == BYTE) {
                result = (int8_t)dest_value ^ (int8_t)source_value;
                set_sr_flags(cpu,
                             result != 0,
                             result == 0,
                             result < 0,
                             ((int8_t)dest_value<0) &&
                             ((int8_t)source_value<0));
            }

            if (is_daddr_virtual){
                write_memory_cb(dest_vaddress, result, bw_flag ? BYTE : WORD);
            } else {
                if (bw_flag==WORD){
                    *destination_addr = result;
                } else if (bw_flag == BYTE){
                    *((uint8_t *) destination_addr) = (uint8_t) result;
                }
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

            if (bw_flag == WORD) {
//                *destination_addr &= (uint16_t)source_value;

                result = dest_value & source_value;

                set_sr_flags(cpu,
                             result != 0,
                             result == 0,
                             result < 0,
                             false);
            }
            else if (bw_flag == BYTE) {
//                *(uint8_t *) destination_addr &= (uint8_t) source_value;

                result = (int8_t)dest_value & (int8_t)source_value;

                set_sr_flags(cpu,
                             (int8_t)result != 0,
                             (int8_t)result == 0,
                             (int8_t)result < 0,
                             false);
            }

            if (is_daddr_virtual){
                write_memory_cb(dest_vaddress, result, bw_flag ? BYTE : WORD);
            } else {
                if (bw_flag==WORD){
                    *destination_addr = result;
                } else if (bw_flag == BYTE){
                    *((uint8_t *) destination_addr) = (uint8_t) result;
                }
            }

            break;
        }

        } //# End of switch

    } // End of if



    // DISASSEMBLY MODE
//    else {
        switch (opcode) {
        case 0x4: {
            bw_flag == WORD ?
                        strncpy(mnemonic, "MOV", sizeof mnemonic) :
                        strncpy(mnemonic, "MOV.B", sizeof mnemonic);

            break;
        }
        case 0x5: {
            bw_flag == WORD ?
                        strncpy(mnemonic, "ADD", sizeof mnemonic) :
                        strncpy(mnemonic, "ADD.B", sizeof mnemonic);

            break;
        }
        case 0x6: {
            bw_flag == WORD ?
                        strncpy(mnemonic, "ADDC", sizeof mnemonic) :
                        strncpy(mnemonic, "ADDC.B", sizeof mnemonic);

            break;
        }
        case 0x7: {
            bw_flag == WORD ?
                        strncpy(mnemonic, "SUBC", sizeof mnemonic) :
                        strncpy(mnemonic, "SUBC.B", sizeof mnemonic);

            break;
        }
        case 0x8: {
            bw_flag == WORD ?
                        strncpy(mnemonic, "SUB", sizeof mnemonic) :
                        strncpy(mnemonic, "SUB.B", sizeof mnemonic);

            break;
        }
        case 0x9: {
            bw_flag == WORD ?
                        strncpy(mnemonic, "CMP", sizeof mnemonic) :
                        strncpy(mnemonic, "CMP.B", sizeof mnemonic);

            break;
        }
        case 0xA: {
            bw_flag == WORD ?
                        strncpy(mnemonic, "DADD", sizeof mnemonic) :
                        strncpy(mnemonic, "DADD.B", sizeof mnemonic);

            break;
        }
        case 0xB: {
            bw_flag == WORD ?
                        strncpy(mnemonic, "BIT", sizeof mnemonic) :
                        strncpy(mnemonic, "BIT.B", sizeof mnemonic);

            break;
        }
        case 0xC: {
            bw_flag == WORD ?
                        strncpy(mnemonic, "BIC", sizeof mnemonic) :
                        strncpy(mnemonic, "BIC.B", sizeof mnemonic);

            break;
        }
        case 0xD: {
            bw_flag == WORD ?
                        strncpy(mnemonic, "BIS", sizeof mnemonic) :
                        strncpy(mnemonic, "BIS.B", sizeof mnemonic);

            break;
        }
        case 0xE: {
            bw_flag == WORD ?
                        strncpy(mnemonic, "XOR", sizeof mnemonic) :
                        strncpy(mnemonic, "XOR.B", sizeof mnemonic);

            break;
        }
        case 0xF: {
            bw_flag == WORD ?
                        strncpy(mnemonic, "AND", sizeof mnemonic) :
                        strncpy(mnemonic, "AND.B", sizeof mnemonic);

            break;
        }

        } //# End of switch

        strncat(mnemonic, "\t", sizeof mnemonic);
        strncat(mnemonic, asm_operands, sizeof mnemonic);
        strncat(mnemonic, "\n", sizeof mnemonic);

//        if (disassemble && emu->debugger->debug_mode) {
        if (emu->debugger->debug_mode){ //disassemble && emu->debugger->debug_mode) {
            int i;
            char one = 0, two = 0;

            // Make little endian big endian
            for (i = 0;i < strlen(hex_str);i += 4) {
                one = hex_str[i];
                two = hex_str[i + 1];

                hex_str[i] = hex_str[i + 2];
                hex_str[i + 1] = hex_str[i + 3];

                hex_str[i + 2] = one;
                hex_str[i + 3] = two;
            }

            printf("%s", hex_str);
            //      print_console(emu, hex_str);

            for (i = strlen(hex_str);i < 12;i++) {
                printf(" ");
                //print_console(emu, " ");
            }

            printf("\t%s", mnemonic);

            //print_console(emu, "\t");
            //print_console(emu, mnemonic);
        }

//    }
}
