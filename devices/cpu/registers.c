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
  along with this program. If not, see <http://www.gnu.org/licenses/>
*/

#include "registers.h"

//##########+++ MSP430 Register initialization +++##########
void
initialize_msp_registers(Emulator *emu)
 {
  Cpu *cpu = emu->cpu;
  Debugger *debugger = emu->debugger;

  cpu->running = false;
  cpu->cg2 = 0;

  // Initialise all regs to 0
  cpu->pc = cpu->sp = cpu->sr = cpu->r4 = cpu->r5 = cpu->r6 = cpu->r7 =
  cpu->r8 =  cpu->r9 = cpu->r10 = cpu->r11 = cpu->r12 = cpu->r13 =
    cpu->r14 = cpu->r15 = 0;
}

void
set_sr_flags(Cpu *cpu, bool C, bool Z, bool N, bool V) {
    cpu->sr &= ~SR_FLAGS_MASK;   // Clear existing flags
    // Set new flags
    cpu->sr |= C ? SR_C : 0;
    cpu->sr |= Z ? SR_Z : 0;
    cpu->sr |= N ? SR_N : 0;
    cpu->sr |= V ? SR_V : 0;
}

bool
get_carry(Cpu *cpu) {
    return ((cpu->sr & SR_C)>0);
}

bool
get_zero_flag(Cpu *cpu) {
    return((cpu->sr & SR_Z)>0);
}

bool
get_negative_flag(Cpu *cpu) {
    return((cpu->sr & SR_N)>0);
}

bool
get_overflow_flag(Cpu *cpu) {
    return((cpu->sr & SR_V)>0);
}

/**
 * @brief truncate_byte truncate 16-bit value to 8-bit as if only the
 * lower 8 bits were read.
 * Necessary because (u)int16_t can be represented by any size integer in the
 * host system
 * @param a
 * @return Truncated value
 */
int16_t
truncate_byte(uint16_t a) {
    bool sign = (a & (1u<<7)) > 0;
    if (sign) {
        return ((a & 0xffu) | 0xffffff00u);
    } else {
        return (a & 0xffu);
    }
}
