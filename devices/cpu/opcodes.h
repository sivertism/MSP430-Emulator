#pragma once

/* ------ Format I Instructions ------ */
#define     OP_MOV      0x4
#define     OP_ADD      0x5
#define     OP_ADDC     0x6
#define     OP_SUBC     0x7
#define     OP_SUB      0x8
#define     OP_CMP      0x9
#define     OP_DADD     0xA
#define     OP_BIT      0xB
#define     OP_BIC      0xC
#define     OP_BIS      0xD
#define     OP_XOR      0xE
#define     OP_AND      0xF

/* ------ Format II Instructions ------ */
#define     OP_RRC      0x0
#define     OP_SWPB     0x1
#define     OP_RRA      0x2
#define     OP_SXT      0x3
#define     OP_PUSH     0x4
#define     OP_CALL     0x5
#define     OP_RETI     0x6
