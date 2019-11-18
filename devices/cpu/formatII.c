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

//##########+++ Decode Format II Instructions +++#########
//# Format II are single operand of the form:
//#   [0001][00CC][CBAA][SSSS]
//#
//# Where C = Opcode, B = Byte/Word flag,
//#       A = Addressing mode for source
//#       S = Source
//########################################################

#include "formatII.h"
#include "../utilities.h"
#include "decoder.h"
#include "opcodes.h"

void decode_formatII(Cpu *cpu, uint16_t instruction, char *disas,
                     instruction_t *instr) {
  int is_saddr_virtual = 0; /// Indicate if source source address is virtual

  uint8_t opcode = (instruction & 0x0380) >> 7;
  uint8_t bw_flag = (instruction & 0x0040) >> 6;
  uint8_t as_flag = (instruction & 0x0030) >> 4;
  uint8_t source = (instruction & 0x000F);

  char reg_name[10];
  reg_num_to_name(source, reg_name);

  uint16_t *reg = get_reg_ptr(cpu, source);
  uint16_t bogus_reg; /* For immediate values to be operated on */

  uint8_t constant_generator_active = 0; /* Specifies if CG1/CG2 active */
  int16_t immediate_constant = 0;        /* Generated Constant */

  /* String to show hex value of instruction */
  char hex_str[100] = {0};
  char hex_str_part[10] = {0};

  sprintf(hex_str, "%04X", instruction);

  /* Spot CG1 and CG2 Constant generator instructions */
  if ((source == 2 && as_flag > 1) || source == 3) {
    constant_generator_active = 1;
    immediate_constant = run_constant_generator(source, as_flag);
  } else {
    constant_generator_active = 0;
  }

  /* Identify the nature of instruction operand addressing modes */
  int16_t source_value, source_offset;
  uint16_t *source_address;
  uint16_t source_vaddress;
  char asm_operand[50] = {0};

  /* Register;     Ex: PUSH Rd */
  /* Constant Gen; Ex: PUSH #C */ /* 0 */
  if (as_flag == 0) {
    if (constant_generator_active) { /* Source Constant */
      source_value = bogus_reg = immediate_constant;
      register_read_notify_cb(1);
      source_address = &bogus_reg;
      is_saddr_virtual = 0;

      sprintf(asm_operand, "#C0x%04X", (uint16_t)source_value);
    } else { /* Source Register */
      source_value = *reg;
      register_read_notify_cb(1);
      source_address = reg;
      is_saddr_virtual = 0;
      sprintf(asm_operand, "%s", reg_name);
      if (opcode == OP_PUSH) {
        consume_cycles_cb(1);
      }
    }
  }

  /* Indexed;      Ex: PUSH 0x0(Rs) */
  /* Symbolic;     Ex: PUSH 0xS     */
  /* Absolute:     Ex: PUSH &0xS    */
  /* Constant Gen; Ex: PUSH #C      */ /* 1 */
  else if (as_flag == 1) {
    if (constant_generator_active) { /* Source Constant */
      source_value = bogus_reg = immediate_constant;
      source_address = &bogus_reg;
      register_read_notify_cb(1);
      is_saddr_virtual = 0;

      sprintf(asm_operand, "C#0x%04X", source_value);
    } else if (source == 0) { /* Source Symbolic */
      source_offset = fetch(cpu);
      source_vaddress = cpu->pc + source_offset - 2;
      register_read_notify_cb(1);

      if (opcode == OP_CALL) {
        // Special case for CALL instruction!
        source_value = source_vaddress;
      } else {
        source_value = mem_read(source_vaddress, bw_flag);
      }

      is_saddr_virtual = 1;

      sprintf(hex_str_part, "%04X", (uint16_t)source_offset);
      strncat(hex_str, hex_str_part, sizeof hex_str_part);
      sprintf(asm_operand, "0x%04X", source_vaddress);
    } else if (source == 2) { /* Source Absolute */
      source_offset = fetch(cpu);
      source_vaddress = source_offset;
      if (opcode == OP_CALL) {
        // Special case for CALL instruction!
        source_value = source_vaddress;
      } else {
        source_value = mem_read(source_vaddress, bw_flag);
      }
      is_saddr_virtual = 1;

      sprintf(hex_str_part, "%04X", (uint16_t)source_value);
      strncat(hex_str, hex_str_part, sizeof hex_str_part);
      sprintf(asm_operand, "&0x%04X", (uint16_t)source_offset);
    } else { /* Source Indexed */
      source_offset = fetch(cpu);
      source_vaddress = *reg + source_offset;
      register_read_notify_cb(1);
      source_value = mem_read(source_vaddress, bw_flag);
      is_saddr_virtual = 1;

      sprintf(hex_str_part, "%04X", (uint16_t)source_offset);
      strncat(hex_str, hex_str_part, sizeof hex_str_part);
      sprintf(asm_operand, "0x%04X(%s)", (uint16_t)source_offset, reg_name);
    }
  }

  /* Indirect;     Ex: PUSH @Rs */
  /* Constant Gen; Ex: PUSH #C */ /* 2, 4 */
  else if (as_flag == 2) {
    if (constant_generator_active) { /* Source Constant */
      source_value = bogus_reg = immediate_constant;
      register_read_notify_cb(1);
      source_address = &bogus_reg;
      is_saddr_virtual = 0;

      sprintf(asm_operand, "C#0x%04X", immediate_constant);
    } else { /* Source Indirect */
      source_vaddress = *reg;
      register_read_notify_cb(1);
      source_value = mem_read(source_vaddress, bw_flag);
      is_saddr_virtual = 1;

      sprintf(asm_operand, "@%s", reg_name);
    }
  }

  /* Indirect AutoIncrement; Ex: PUSH @Rs+ */
  /* Immediate;              Ex: PUSH #S   */
  /* Constant Gen;           Ex: PUSH #C   */ /* -1, 8 */
  else if (as_flag == 3) {
    if (constant_generator_active) { /* Source Constant */
      source_value = bogus_reg = immediate_constant;
      register_read_notify_cb(1);
      source_address = &bogus_reg;
      is_saddr_virtual = 0;

      sprintf(asm_operand, "C#0x%04X", (uint16_t)source_value);
    } else if (source == 0) { /* Source Immediate */
      source_value = bogus_reg = fetch(cpu);
      source_address = &bogus_reg;
      is_saddr_virtual = 0;

      sprintf(hex_str_part, "%04X", (uint16_t)source_value);
      strncat(hex_str, hex_str_part, sizeof hex_str_part);

      if (bw_flag == WORD) {
        sprintf(asm_operand, "#0x%04X", (uint16_t)source_value);
      } else if (bw_flag == BYTE) {
        sprintf(asm_operand, "#0x%04X", (uint8_t)source_value);
      }
    } else { /* Source Indirect AutoIncrement */
      source_vaddress = *reg;
      register_read_notify_cb(1);
      source_value = mem_read(source_vaddress, bw_flag);
      is_saddr_virtual = 1;

      sprintf(asm_operand, "@%s+", reg_name);

      *reg += bw_flag ? 1 : 2;
      register_write_notify_cb(1);
    }
  }

  int16_t result;
  switch (opcode) {

  /*  RRC Rotate right through carry
   *    C → MSB → MSB-1 .... LSB+1 → LSB → C
   *
   *  Description The destination operand is shifted right one position
   *  as shown in Figure 3-18. The carry bit (C) is shifted into the MSB,
   *  the LSB is shifted into the carry bit (C).
   *
   * N: Set if result is negative, reset if positive
   * Z: Set if result is zero, reset otherwise
   * C: Loaded from the LSB
   * V: Reset
   */
  case 0x0: {
    uint16_t CF = get_carry(cpu);
    bool c, z, n, v;

    result = source_value;
    result >>= 1;

    if (bw_flag == WORD) {
      result &= ~(1u << 15); // Clear MSB
      result |= (CF << 15);  // Insert carry on MSB

    } else if (bw_flag == BYTE) {
      result &= ~(1u << 7); // Clear MSB
      result |= (CF << 7);  // Insert carry on MSB
    }

    if (is_saddr_virtual) { // Write result to memory
      mem_write(source_vaddress, result, bw_flag);
    } else { // Write result to register
      *source_address = bw_flag ? result & 0xFF : result;
      register_write_notify_cb(1);
    }

    // Make sure only relevant part of

    c = source_value & 1u; // set next c from LSB
    n = CF;                // Previous CF is now MSB
    z = is_zero(result, bw_flag);
    v = false;
    set_sr_flags(cpu, c, z, n, v);
    strncpy(instr->mnemonic, "RRC", sizeof(instr->mnemonic) - 1);

    break;
  }

    /* SWPB Swap bytes
     * bw flag always 0 (word)
     * Bits 15 to 8 ↔ bits 7 to 0
     * No flags affected
     */
  case 0x1: {
    uint16_t upper, lower;

    upper = (source_value & 0xFF00);
    lower = (source_value & 0x00FF);
    result = (lower << 8) | (upper >> 8);

    if (is_saddr_virtual) { // Write result to memory
      mem_write(source_vaddress, result, WORD);
    } else { // Write result to register
      *source_address = result;
      register_write_notify_cb(1);
    }

    strncpy(instr->mnemonic, "SWPB", sizeof(instr->mnemonic) - 1);
    break;
  }

    /* RRA Rotate right arithmetic
     *   MSB → MSB, MSB → MSB-1, ... LSB+1 → LSB, LSB → C
     *
     * N: Set if result is negative, reset if positive
     * Z: Set if result is zero, reset otherwise
     * C: Loaded from the LSB
     * V: Reset
     */
  case 0x2: {

    bool c, z, n, v;

    if (bw_flag == WORD) {
      result = (source_value & (1 << 15)) | // MSB
               (source_value >> 1);
    } else if (bw_flag == BYTE) {
      result = (source_value & (1 << 7)) | // MSB
               (source_value >> 1);
    }

    if (is_saddr_virtual) { // Write result to memory
      mem_write(source_vaddress, result, bw_flag);
    } else { // Write result to register
      *source_address = bw_flag ? result & 0xFF : result;
      register_write_notify_cb(1);
    }

    c = source_value & 1;
    v = false;
    n = is_negative(result, bw_flag);
    z = is_zero(result, bw_flag);
    set_sr_flags(cpu, c, z, n, v);
    strncpy(instr->mnemonic, "RRA", sizeof(instr->mnemonic) - 1);

    break;
  }

    /* SXT Sign extend byte to word
     *   bw flag always 0 (word)
     *
     * Bit 7 → Bit 8 ......... Bit 15
     *
     * N: Set if result is negative, reset if positive
     * Z: Set if result is zero, reset otherwise
     * C: Set if result is not zero, reset otherwise (.NOT. Zero)
     * V: Reset
     */

  case 0x3: {
    result = (source_value & (1 << 7)) ? source_value | 0xFF00
                                       : source_value & 0x00FF;

    if (is_saddr_virtual) { // Write result to memory
      mem_write(source_vaddress, result, WORD);
    } else { // Write result to register
      *source_address = result;
      register_write_notify_cb(1);
    }

    bool c, z, n, v;
    z = is_zero(result, bw_flag);
    n = is_negative(result, bw_flag);
    c = !z;
    v = false;
    set_sr_flags(cpu, c, z, n, v);
    strncpy(instr->mnemonic, "SXT", sizeof(instr->mnemonic) - 1);

    break;
  }

    /* PUSH push value on to the stack
     *
     *   SP - 2 → SP
     *   src → @SP
     *
     */
  case 0x4: {

    cpu->sp -= 2; /* Yes, even for BYTE Instructions */
    register_write_notify_cb(1);

    // Write result to memory
    mem_write(cpu->sp, source_value, bw_flag);

    strncpy(instr->mnemonic, "PUSH", sizeof(instr->mnemonic) - 1);
    break;
  }

    /* CALL SUBROUTINE:
     *     PUSH PC and PC = SRC
     *
     *     This is always a word instruction. Supporting all addressing modes
     */

  case 0x5: {
    // Push PC
    cpu->sp -= 2;
    register_write_notify_cb(1);
    consume_cycles_cb(1);
    mem_write(cpu->sp, cpu->pc, WORD);

    // Jump
    cpu->pc = source_value;
    register_write_notify_cb(1);
    instr->isDestPC = true;
    strncpy(instr->mnemonic, "CALL", sizeof(instr->mnemonic) - 1);
    break;
  }

    //# RETI Return from interrupt: Pop SR then pop PC
  case 0x6: {
    // 1 Pop SR from the stack
    cpu->sr = mem_read(cpu->sp, WORD);
    cpu->sp += 2;
    register_write_notify_cb(2);

    // 2 Pop PC from stack
    cpu->pc = mem_read(cpu->sp, WORD);
    cpu->sp += 2;
    instr->isDestPC = true;
    register_write_notify_cb(2);

    consume_cycles_cb(2);
    strncpy(instr->mnemonic, "RETI", sizeof(instr->mnemonic) - 1);
    break;
  }
  default: {
    fprintf(stderr, "INVALID FORMAT II OPCODE, EXITING.");
    exit(1);
  }

  } //# End of Switch

  if (disas != NULL) {
    switch (opcode) {
    case 0x0: {
      bw_flag == WORD ? strncpy(disas, "RRC", DISAS_STR_LEN)
                      : strncpy(disas, "RRC.B", DISAS_STR_LEN);

      break;
    }
    case 0x1: {
      strncpy(disas, "SWPB", DISAS_STR_LEN);
      break;
    }
    case 0x2: {
      bw_flag == WORD ? strncpy(disas, "RRA", DISAS_STR_LEN)
                      : strncpy(disas, "RRA.B", DISAS_STR_LEN);

      break;
    }
    case 0x3: {
      strncpy(disas, "SXT", DISAS_STR_LEN);
      break;
    }
    case 0x4: {
      bw_flag == WORD ? strncpy(disas, "PUSH", DISAS_STR_LEN)
                      : strncpy(disas, "PUSH.B", DISAS_STR_LEN);

      break;
    }
    case 0x5: {
      strncpy(disas, "CALL", DISAS_STR_LEN);
      break;
    }
    case 0x6: {
      strncpy(disas, "RETI", DISAS_STR_LEN);
      break;
    }
    default: {
      printf("Unknown Single operand instruction.\n");
    }

    } //# End of Switch

    strncat(disas, " ", DISAS_STR_LEN);
    strncat(disas, asm_operand, DISAS_STR_LEN);
  }
}
