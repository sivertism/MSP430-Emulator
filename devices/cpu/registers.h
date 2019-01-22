#ifndef _REGISTERS_H_
#define _REGISTERS_H_

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

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

// Register names
#define REG_PC          0u
#define REG_SP          1u
#define REG_SR          2u


// Main CPU structure //
typedef struct Cpu {
  bool running;      /* CPU running or not */

  uint16_t pc, sp, sr;   /* R0, R1 and R3 respectively */
  int16_t cg2;       /* R3 or Constant Generator #2 */
  
  int16_t r2, r4, r5, r6, r7;   /* R4-R15 General Purpose Registers */
  int16_t r8, r9, r10, r11;
  int16_t r12, r13, r14, r15;
} Cpu;

void initialize_msp_registers (Cpu *cpu);

void set_sr_flags(Cpu *cpu, bool C, bool Z, bool N, bool V);
bool get_carry(Cpu *cpu);
bool get_zero_flag(Cpu *cpu);
bool get_negative_flag(Cpu *cpu);
bool get_overflow_flag(Cpu *cpu);
int16_t truncate_byte(uint16_t source);

#endif
