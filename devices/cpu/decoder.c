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

#include "decoder.h"

/*##########+++ CPU Fetch Cycle  +++##########*/
uint16_t fetch(Cpu *cpu)
{
  uint16_t word = read_memory_cb(cpu->pc, WORD);
  cpu->pc += 2;
  return word;
}

/*##########+++ CPU Decode Cycle +++##########*/
void decode(Cpu *cpu, uint16_t instruction, bool disassemble)
{  
  uint8_t format_id;

  format_id = (uint8_t)(instruction >> 12);

  if (format_id == 0x1) {
    // format II (single operand) instruction //
    decode_formatII(cpu, instruction, disassemble);
  }    
  else if (format_id >= 0x2 && format_id <= 3) {
    // format III (jump) instruction //
    decode_formatIII(cpu, instruction, disassemble);
  }
  else if (format_id >= 0x4) {
    // format I (two operand) instruction //
    decode_formatI(cpu, instruction, disassemble);
  }
  else {
    char inv[100] = {0};

    sprintf(inv, "%04X\t[INVALID INSTRUCTION]\n", instruction);
    printf("%s", inv);
    
    cpu->pc -= 2;
    cpu->running = false;
  }
}

int16_t run_constant_generator(uint8_t source, uint8_t as_flag) 
{
  int16_t generated_constant = 0;

  switch (source) {
    case 2: {   /* Register R2/SR/CG1 */

      switch (as_flag) {
        case 0b10: {   /* +4, bit processing */
	  generated_constant = 4;
	  break;
        }
        case 0b11: {   /* +8, bit processing */
	  generated_constant = 8;
	  break;
	}
        default: {
	  printf("Invalid as_flag for CG1\n");
        }
      }
      
      break;
    }
      
    case 3: {   /* Register R3/CG2*/

      switch (as_flag) {
        case 0b00: {   /* 0, word processing */
	  generated_constant = 0;
	  break;
        }
        case 0b01: {   /* +1 */
	  generated_constant = 1;
	  break;
        }
        case 0b10: {   /* +2, bit processing */
	  generated_constant = 2;
	  break;
        }
        case 0b11: {   /* -1, word processing */
	  generated_constant = -1;
	  break;
        }
        default: {
	  printf("Invalid as_flag for CG2\n");
        }
      }

      break;
    }
      
    default: {
      printf("Invalid source register for constant generation.\n");
    }  
  }

  return generated_constant;
}
