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

//##########+++ Decode Format III Instructions +++#########
//# Format III are jump instructions of the form:
//#   [001C][CCXX][XXXX][XXXX]
//# 
//# Where C = Condition, X = 10-bit signed offset 
//# 
//########################################################

#include "decoder.h"

void decode_formatIII(Cpu *cpu, uint16_t instruction, char *disas, instruction_t *instr)
{
    uint8_t condition = (instruction & 0x1C00) >> 10;
    int16_t signed_offset = (instruction & 0x03FF) * 2;
    bool negative = (instruction & (1u<<9)) > 0; // signed_offset >> 9;

    // All jumps take 2 cycles (1 for fetch and one for execute)
    consume_cycles_cb(1);

    if (negative) { /* Sign Extend for Arithmetic Operations */
        signed_offset |= 0xfffff800;
    }

    /* String to show hex value of instruction */
    char hex_str[100] = {0};

    sprintf(hex_str, "%04X", instruction);

    switch(condition){

    /* JNE/JNZ Jump if not equal/zero
          *
          * If Z = 0: PC + 2 offset → PC
          * If Z = 1: execute following instruction
          */
    case 0x0:{
        if (get_zero_flag(cpu) == false) {
            cpu->pc += signed_offset;
        }
        strncpy(instr->mnemonic, "JNZ", sizeof(instr->mnemonic)-1);
        break;
    }

        /* JEQ/JZ Jump is equal/zero
   * If Z = 1: PC + 2 offset → PC
   * If Z = 0: execute following instruction
  */
    case 0x1:{
        if (get_zero_flag(cpu) == true) {
            cpu->pc += signed_offset;
        }
        strncpy(instr->mnemonic, "JZ", sizeof(instr->mnemonic)-1);
        break;
    }

        /* JNC/JLO Jump if no carry/lower
  *
  *  if C = 0: PC + 2 offset → PC
  *  if C = 1: execute following instruction
  */
    case 0x2:{
        if (get_carry(cpu) == false) {
            cpu->pc += signed_offset;
        }
        strncpy(instr->mnemonic, "JNC", sizeof(instr->mnemonic)-1);
        break;
    }

        /* JC/JHS Jump if carry/higher or same
  *
  * If C = 1: PC + 2 offset → PC
  * If C = 0: execute following instruction
  */
    case 0x3:{
        if (get_carry(cpu) == true) {
            cpu->pc += signed_offset;
        }
        strncpy(instr->mnemonic, "JC", sizeof(instr->mnemonic)-1);
        break;
    }

        /* JN Jump if negative
  *
  *  if N = 1: PC + 2 ×offset → PC
  *  if N = 0: execute following instruction
  */
    case 0x4:{
        if (get_negative_flag(cpu) == true) {
            cpu->pc += signed_offset;
        }

        strncpy(instr->mnemonic, "JN", sizeof(instr->mnemonic)-1);
        break;
    }

        /* JGE Jump if greater or equal (N == V)
  *
  *  If (N .XOR. V) = 0 then jump to label: PC + 2 P offset → PC
  *  If (N .XOR. V) = 1 then execute the following instruction
  */
    case 0x5:{
        if ((get_negative_flag(cpu) ^ get_overflow_flag(cpu)) == false) {
            cpu->pc += signed_offset;
        }

        strncpy(instr->mnemonic, "JGE", sizeof(instr->mnemonic)-1);
        break;
    }

        /* JL Jump if less (N != V)
  *
  *  If (N .XOR. V) = 1 then jump to label: PC + 2 offset → PC
  *  If (N .XOR. V) = 0 then execute following instruction
  */
    case 0x6:{
        if ((get_negative_flag(cpu) ^ get_overflow_flag(cpu)) == true) {
            cpu->pc += signed_offset;
        }
        strncpy(instr->mnemonic, "JL", sizeof(instr->mnemonic)-1);

        break;
    }

        /* JMP Jump Unconditionally
   *
   *  PC + 2 × offset → PC
   *
   */
    case 0x7:{
        cpu->pc += signed_offset;
        strncpy(instr->mnemonic, "JMP", sizeof(instr->mnemonic)-1);
        break;
    }

    default: {
        fprintf(stderr, "INVALID FORMAT III OPCODE, EXITING.");
        exit(1);
    }

    } //# End of Switch

    if (disas != NULL) {
        char value[20];

        switch(condition){

        case 0x0:{
            sprintf(disas, "JNZ");
            sprintf(value, "0x%04X", cpu->pc + signed_offset);
            break;
        }
        case 0x1:{
            sprintf(disas, "JZ");
            sprintf(value, "0x%04X", cpu->pc + signed_offset);
            break;
        }
        case 0x2:{
            sprintf(disas, "JNC");
            sprintf(value, "0x%04X", cpu->pc + signed_offset);
            break;
        }
        case 0x3:{
            sprintf(disas, "JC");
            sprintf(value, "0x%04X", cpu->pc + signed_offset);
            break;
        }
        case 0x4:{
            sprintf(disas, "JN");
            sprintf(value, "0x%04X", cpu->pc + signed_offset);
            break;
        }
        case 0x5:{
            sprintf(disas, "JGE");
            sprintf(value, "0x%04X", cpu->pc + signed_offset);

            break;
        }
        case 0x6:{
            sprintf(disas, "JL");
            sprintf(value, "0x%04X", cpu->pc + signed_offset);

            break;
        }
        case 0x7:{
            sprintf(disas, "JMP");
            sprintf(value, "0x%04X", cpu->pc + signed_offset);
            break;
        }
        default:{
            puts("Undefined Jump operation!\n");
            return;
        }

        } //# End of Switch

        strncat(disas, " ", DISAS_STR_LEN);
        strncat(disas, value, DISAS_STR_LEN);
    }
}
