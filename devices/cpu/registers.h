#ifndef _REGISTERS_H_
#define _REGISTERS_H_

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "../../main.h"

/* Bits in SR */
#define SR_C            (1u<<0)     // Carry
#define SR_Z            (1u<<1)     // Zero
#define SR_N            (1u<<2)     // Negative
#define SR_V            (1u<<8)     // Overflow
#define SR_GIE          (1u<<3)     // Global interrupt enable
#define SR_CPU_OFF      (1u<<4)     // CPU off
#define SR_OSC_OFF      (1u<<5)     // OSC off
#define SR_SCG0         (1u<<6)     // System clock generator 0
#define SR_SCG1         (1u<<7)     // System clock generator 1
#define SR_FLAGS_MASK   (SR_C|SR_Z|SR_N|SR_V)

/* r2 or SR, the status register */
typedef struct Status_reg {
  uint8_t reserved : 7;   // Reserved bits    
  uint8_t overflow : 1;   // Overflow flag
  uint8_t SCG1 : 1;  // System Clock Generator SMCLK; ON = 0; OFF = 1;
  uint8_t SCG0 : 1;  // System Clock Generator DCOCLK DCO ON = 0; DCO OFF = 1;
  uint8_t OSCOFF : 1;    // Oscillator Off. LFXT1CLK ON = 0; LFXT1CLK OFF = 1; 
  uint8_t CPUOFF : 1;     // CPU off; CPU OFF = 1; CPU ON = 0;                
  uint8_t GIE : 1;    // General Inter enabl; Enbl maskable ints = 1; 0 = dont 
  uint8_t negative : 1;   // Negative flag                                  
  uint8_t zero : 1;       // Zero flag                                     
  uint8_t carry : 1;      // Carry flag; Set when result produces a carry   
} Status_reg;

// Main CPU structure //
typedef struct Cpu {
  bool running;      /* CPU running or not */

  uint16_t pc, sp, sr;   /* R0, R1 and R3 respectively */
//Status_reg sr;     /* Status register fields */
  int16_t cg2;       /* R3 or Constant Generator #2 */
  
  int16_t r2, r4, r5, r6, r7;   /* R4-R15 General Purpose Registers */
  int16_t r8, r9, r10, r11;
  int16_t r12, r13, r14, r15;

  Port_1 *p1;
  //Port_2 *p2;
  Usci *usci;
  Bcm *bcm;
  Timer_a *timer_a;
} Cpu;

void initialize_msp_registers (Emulator *emu);

void set_sr_flags(Cpu *cpu, bool C, bool Z, bool N, bool V);
bool get_carry(Cpu *cpu);
bool get_zero_flag(Cpu *cpu);
bool get_negative_flag(Cpu *cpu);
bool get_overflow_flag(Cpu *cpu);


#endif
