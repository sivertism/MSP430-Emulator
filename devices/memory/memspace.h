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

#ifndef _MEMSPACE_H_
#define _MEMSPACE_H_

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

extern uint8_t *MEMSPACE;   /* Memory Space */
extern uint8_t *CODE;       // Code Memory
extern uint8_t *INFO;       // Info memory
extern uint8_t *IVT;        /* Interrupt Vector Table {Within ROM} */
extern uint8_t *ROM;        /* Flash/Read-Only memory */
extern uint8_t *RAM;        /* Random Access Memory */
extern uint8_t *PER16;      /* 16-bit peripherals */
extern uint8_t *PER8;       /* 8-bit peripherals */
extern uint8_t *SFRS;       /* Special Function Registers */

uint8_t * initialize_msp_memspace();
void uninitialize_msp_memspace();

extern uint8_t blc [1024];
#endif
