#include "memspace.h"

uint8_t *MEMSPACE;   /* Memory Space */
uint8_t *CODE;       // Code Memory
uint8_t *INFO;       // Info memory
uint8_t *IVT;        /* Interrupt Vector Table {Within ROM} */
uint8_t *ROM;        /* Flash/Read-Only memory */
uint8_t *RAM;        /* Random Access Memory */
uint8_t *PER16;      /* 16-bit peripherals */
uint8_t *PER8;       /* 8-bit peripherals */
uint8_t *SFRS;       /* Special Function Registers */

uint8_t blc[] = { // Official TI bootloader code
  0x06, 0x0c, 0x1e, 0x0c, 0xff, 0x3f, 0xb2, 0x40,
  0x40, 0xa5, 0x2c, 0x01, 0xb2, 0x90, 0x55, 0xaa,
  0xde, 0xff, 0xff, 0x27, 0x31, 0x40, 0x20, 0x02,
  0x0b, 0x43, 0xc0, 0x43, 0xfa, 0xf5, 0x32, 0xc2,
  0xf2, 0xc0, 0x32, 0x00, 0x00, 0x00, 0xb2, 0x40,
  0x80, 0x5a, 0x20, 0x01, 0xf2, 0x40, 0x88, 0x00,
  0x57, 0x00, 0xf2, 0x40, 0x80, 0x00, 0x56, 0x00,
  0xe2, 0xd3, 0x21, 0x00, 0xe2, 0xd3, 0x22, 0x00,
  0xf2, 0xc0, 0x22, 0x00, 0x26, 0x00, 0xf2, 0xc0,
  0x20, 0x00, 0x22, 0x00, 0xb2, 0x40, 0x00, 0xa5,
  0x28, 0x01, 0x3b, 0xc0, 0x7a, 0x00, 0xb0, 0x12,
  0x7e, 0x0e, 0x82, 0x43, 0x12, 0x02, 0x09, 0x43,
  0x36, 0x40, 0x0a, 0x02, 0x37, 0x42, 0xb0, 0x12,
  0x52, 0x0f, 0xc6, 0x4c, 0x00, 0x00, 0x16, 0x53,
  0x17, 0x83, 0xf9, 0x23, 0x2b, 0xb3, 0x26, 0x20,
  0x55, 0x42, 0x0b, 0x02, 0x75, 0x90, 0x12, 0x00,
  0x75, 0x24, 0x75, 0x90, 0x10, 0x00, 0x54, 0x24,
  0xb0, 0x12, 0x50, 0x0e, 0x55, 0x42, 0x0b, 0x02,
  0x75, 0x90, 0x18, 0x00, 0x2e, 0x24, 0x2b, 0xb2,
  0x15, 0x24, 0x75, 0x90, 0x20, 0x00, 0x15, 0x24,
  0x75, 0x90, 0x16, 0x00, 0x22, 0x24, 0x75, 0x90,
  0x14, 0x00, 0x9d, 0x24, 0x75, 0x90, 0x1a, 0x00,
  0x18, 0x24, 0x75, 0x90, 0x1c, 0x00, 0x2f, 0x24,
  0x05, 0x3c, 0x2b, 0xd2, 0xb0, 0x12, 0xe4, 0x0e,
  0xc5, 0x3f, 0x21, 0x53, 0xb0, 0x12, 0xea, 0x0e,
  0xc1, 0x3f, 0xb0, 0x12, 0xe4, 0x0e, 0xd2, 0x42,
  0x0e, 0x02, 0x56, 0x00, 0xd2, 0x42, 0x0f, 0x02,
  0x57, 0x00, 0xd2, 0x42, 0x10, 0x02, 0x16, 0x02,
  0xb5, 0x3f, 0xb0, 0x12, 0xe4, 0x0e, 0x10, 0x42,
  0x0e, 0x02, 0x16, 0x42, 0x0e, 0x02, 0x15, 0x43,
  0x07, 0x3c, 0x36, 0x40, 0xfe, 0xff, 0xb2, 0x40,
  0x06, 0xa5, 0x10, 0x02, 0x35, 0x40, 0x0c, 0x00,
  0x92, 0x42, 0x10, 0x02, 0x28, 0x01, 0xb6, 0x43,
  0x00, 0x00, 0x92, 0xb3, 0x2c, 0x01, 0xfd, 0x23,
  0x15, 0x83, 0xf6, 0x23, 0xd3, 0x3f, 0x16, 0x42,
  0x0e, 0x02, 0x17, 0x42, 0x10, 0x02, 0x35, 0x43,
  0x75, 0x96, 0x03, 0x20, 0x17, 0x83, 0xfc, 0x23,
  0xc9, 0x3f, 0x82, 0x46, 0x00, 0x02, 0xca, 0x3f,
  0x36, 0x40, 0xe0, 0xff, 0x37, 0x40, 0x20, 0x00,
  0x0a, 0x12, 0x0a, 0x43, 0xb0, 0x12, 0x52, 0x0f,
  0x7c, 0xe6, 0x4a, 0xdc, 0x17, 0x83, 0xfa, 0x23,
  0xb0, 0x12, 0x50, 0x0e, 0x4a, 0x93, 0x3a, 0x41,
  0xb4, 0x27, 0x82, 0x93, 0xde, 0xff, 0x07, 0x24,
  0x31, 0x40, 0x00, 0x10, 0x04, 0x12, 0x31, 0x90,
  0x22, 0x02, 0xfc, 0x23, 0xc6, 0x3f, 0xb0, 0x12,
  0xea, 0x0e, 0x70, 0x3f, 0x16, 0x42, 0x0e, 0x02,
  0x17, 0x42, 0x10, 0x02, 0x2b, 0xb2, 0x30, 0x24,
  0x3b, 0xd0, 0x10, 0x00, 0xb0, 0x12, 0x52, 0x0f,
  0x2b, 0xb3, 0x2c, 0x20, 0x36, 0x90, 0x00, 0x10,
  0x06, 0x2c, 0x36, 0x90, 0x00, 0x01, 0x06, 0x2c,
  0xc6, 0x4c, 0x00, 0x00, 0x1b, 0x3c, 0xb2, 0x40,
  0x40, 0xa5, 0x28, 0x01, 0x16, 0xb3, 0x03, 0x20,
  0xc2, 0x4c, 0x14, 0x02, 0x13, 0x3c, 0xc2, 0x4c,
  0x15, 0x02, 0x86, 0x9a, 0xfd, 0xff, 0x01, 0x24,
  0x2b, 0xd3, 0x36, 0x90, 0x01, 0x02, 0x04, 0x28,
  0x3b, 0xd2, 0x3b, 0xb0, 0x10, 0x00, 0x02, 0x24,
  0x3b, 0xc0, 0x32, 0x00, 0x1a, 0x42, 0x14, 0x02,
  0x86, 0x4a, 0xff, 0xff, 0x16, 0x53, 0x17, 0x83,
  0xd5, 0x23, 0xb0, 0x12, 0x50, 0x0e, 0x72, 0x3f,
  0xb0, 0x12, 0x52, 0x0f, 0x17, 0x83, 0xfc, 0x23,
  0xb0, 0x12, 0x50, 0x0e, 0x6f, 0x3f, 0xb2, 0x40,
  0x80, 0x00, 0x0a, 0x02, 0xd2, 0x42, 0x10, 0x02,
  0x0c, 0x02, 0xd2, 0x42, 0x10, 0x02, 0x0d, 0x02,
  0x82, 0x43, 0x12, 0x02, 0x09, 0x43, 0x36, 0x40,
  0x0a, 0x02, 0x27, 0x42, 0x7c, 0x46, 0xb0, 0x12,
  0xee, 0x0e, 0x17, 0x83, 0xfb, 0x23, 0x16, 0x42,
  0x0e, 0x02, 0x17, 0x42, 0x10, 0x02, 0x36, 0x90,
  0x00, 0x01, 0x0a, 0x28, 0xb2, 0x46, 0x14, 0x02,
  0x5c, 0x42, 0x14, 0x02, 0xb0, 0x12, 0xee, 0x0e,
  0x17, 0x83, 0x5c, 0x42, 0x15, 0x02, 0x01, 0x3c,
  0x7c, 0x46, 0xb0, 0x12, 0xee, 0x0e, 0x17, 0x83,
  0xee, 0x23, 0xb2, 0xe3, 0x12, 0x02, 0x5c, 0x42,
  0x12, 0x02, 0xb0, 0x12, 0xee, 0x0e, 0x5c, 0x42,
  0x13, 0x02, 0xb0, 0x12, 0xee, 0x0e, 0xfe, 0x3e,
  0x18, 0x42, 0x12, 0x02, 0xb0, 0x12, 0x52, 0x0f,
  0xc2, 0x4c, 0x12, 0x02, 0xb0, 0x12, 0x52, 0x0f,
  0xc2, 0x4c, 0x13, 0x02, 0x38, 0xe3, 0x3b, 0xb2,
  0x04, 0x24, 0x86, 0x9a, 0xfe, 0xff, 0x01, 0x24,
  0x2b, 0xd3, 0x18, 0x92, 0x12, 0x02, 0x25, 0x23,
  0x2b, 0xb3, 0x23, 0x23, 0x30, 0x41, 0xf2, 0xb0,
  0x20, 0x00, 0x20, 0x00, 0xfc, 0x27, 0xf2, 0xb0,
  0x20, 0x00, 0x20, 0x00, 0xfc, 0x23, 0xb2, 0x40,
  0x24, 0x02, 0x60, 0x01, 0xf2, 0xb0, 0x20, 0x00,
  0x20, 0x00, 0xfc, 0x27, 0x15, 0x42, 0x70, 0x01,
  0x05, 0x11, 0x05, 0x11, 0x05, 0x11, 0x82, 0x45,
  0x02, 0x02, 0x05, 0x11, 0x82, 0x45, 0x04, 0x02,
  0xb2, 0x80, 0x1e, 0x00, 0x04, 0x02, 0x57, 0x42,
  0x16, 0x02, 0x37, 0x80, 0x03, 0x00, 0x05, 0x11,
  0x05, 0x11, 0x17, 0x53, 0xfd, 0x23, 0x35, 0x50,
  0x40, 0xa5, 0x82, 0x45, 0x2a, 0x01, 0x35, 0x42,
  0xb2, 0x40, 0x24, 0x02, 0x60, 0x01, 0x92, 0x92,
  0x70, 0x01, 0x02, 0x02, 0xfc, 0x2f, 0x15, 0x83,
  0xf7, 0x23, 0x09, 0x43, 0x7c, 0x40, 0x90, 0x00,
  0x02, 0x3c, 0x7c, 0x40, 0xa0, 0x00, 0x4d, 0x43,
  0xc9, 0xec, 0x12, 0x02, 0x19, 0xe3, 0x1b, 0xc3,
  0x45, 0x4d, 0x55, 0x45, 0x00, 0x0f, 0x00, 0x55,
  0x0c, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e, 0x2e,
  0x2e, 0x1a, 0x34, 0x34, 0x92, 0x42, 0x70, 0x01,
  0x72, 0x01, 0xb2, 0x50, 0x0c, 0x00, 0x72, 0x01,
  0x07, 0x3c, 0x1b, 0xb3, 0x0b, 0x20, 0x82, 0x43,
  0x62, 0x01, 0x92, 0xb3, 0x62, 0x01, 0xfd, 0x27,
  0xe2, 0xc3, 0x21, 0x00, 0x0a, 0x3c, 0x4c, 0x11,
  0xf6, 0x2b, 0x1b, 0xe3, 0x82, 0x43, 0x62, 0x01,
  0x92, 0xb3, 0x62, 0x01, 0xfd, 0x27, 0xe2, 0xd3,
  0x21, 0x00, 0x92, 0x52, 0x02, 0x02, 0x72, 0x01,
  0x5d, 0x53, 0x7d, 0x90, 0x0c, 0x00, 0xd4, 0x23,
  0x30, 0x41, 0x4e, 0x43, 0x1b, 0xc3, 0x45, 0x4e,
  0x55, 0x45, 0x5e, 0x0f, 0x00, 0x55, 0x0c, 0x5c,
  0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x5c, 0x3a,
  0x7c, 0x00, 0xf2, 0xb0, 0x20, 0x00, 0x20, 0x00,
  0xfc, 0x23, 0x92, 0x42, 0x70, 0x01, 0x72, 0x01,
  0x92, 0x52, 0x04, 0x02, 0x72, 0x01, 0x82, 0x43,
  0x62, 0x01, 0x92, 0xb3, 0x62, 0x01, 0xfd, 0x27,
  0xf2, 0xb0, 0x20, 0x00, 0x20, 0x00, 0x20, 0x28,
  0x2b, 0xd3, 0x1e, 0x3c, 0x4c, 0x10, 0x1c, 0x3c,
  0x82, 0x43, 0x62, 0x01, 0x92, 0xb3, 0x62, 0x01,
  0xfd, 0x27, 0xf2, 0xb0, 0x20, 0x00, 0x20, 0x00,
  0x01, 0x28, 0x1b, 0xe3, 0x1b, 0xb3, 0x01, 0x24,
  0x2b, 0xd3, 0xc9, 0xec, 0x12, 0x02, 0x19, 0xe3,
  0x0b, 0x3c, 0x82, 0x43, 0x62, 0x01, 0x92, 0xb3,
  0x62, 0x01, 0xfd, 0x27, 0xf2, 0xb0, 0x20, 0x00,
  0x20, 0x00, 0xe4, 0x2b, 0x4c, 0x10, 0x1b, 0xe3,
  0x92, 0x52, 0x02, 0x02, 0x72, 0x01, 0x5e, 0x53,
  0xbe, 0x3f, 0x82, 0x43, 0x62, 0x01, 0x92, 0xb3,
  0x62, 0x01, 0xfd, 0x27, 0xf2, 0xb0, 0x20, 0x00,
  0x20, 0x00, 0x01, 0x2c, 0x2b, 0xd3, 0x30, 0x41,
  0x25, 0x53, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x02, 0x03, 0x01, 0x00, 0x26, 0x30,
};

/* Initialize Address Space Locations
** 
** Allocate and set MSP430 Memory space
** Some of these locations vary by model
*/
uint8_t * initialize_msp_memspace()
{
  // MSP430g2553 Device Specific ...
  // 16 KB / 64 KB Addressable Space
  MEMSPACE = (uint8_t *) calloc(1, 0x10000);

  // (lower bounds, so increment upwards)

  // Info memory from 0x10FF - 0x1000 (256 Bytes)
  INFO = MEMSPACE + 0x1000;
  // Set it all to 0xFF by default...
  memset(INFO, 0xFF, 256);

  // Code Memory 0xFFFF - 0xC000;
  CODE = MEMSPACE + 0xC000;
  // Set it all to 0xFF by default...
  memset(CODE, 0xFF, 16384);
  
  // Interrupt Vector Table 0xFFFF - 0xFFC0
  IVT = MEMSPACE + 0xFFC0;
  
  // ROM // 0x400 - 0x1FFFF
  ROM = MEMSPACE + 0x0400;   

  // RAM from 0x3FF - 0x200
  RAM = MEMSPACE + 0x0200;   // 0x200 - 0x3FF
  PER16 = MEMSPACE + 0x0100;   // 0x0100 - 0x01FF
  PER8 = MEMSPACE + 0x0010;   // 0x0010 - 0x00FF
  SFRS = MEMSPACE + 0x0;      // 0x0 - 0x0F
  
  // Setup the calibration data in info memory

  const uint16_t CALDCO_16MHZ_VLOC = 0x10F8;
  *((uint8_t *) (MEMSPACE + CALDCO_16MHZ_VLOC)) = 0x95;

  const uint16_t CALBC1_16MHZ_VLOC = 0x10F9;
  *((uint8_t *) (MEMSPACE + CALBC1_16MHZ_VLOC)) = 0x8F;
  
  const uint16_t CALDCO_12MHZ_VLOC = 0x10FA;
  *((uint8_t *) (MEMSPACE + CALDCO_12MHZ_VLOC)) = 0x9E;
  
  const uint16_t CALBC1_12MHZ_VLOC = 0x10FB;
  *((uint8_t *) (MEMSPACE + CALBC1_12MHZ_VLOC)) = 0x8E;
  
  const uint16_t CALDCO_8MHZ_VLOC = 0x10FC;
  *((uint8_t *) (MEMSPACE + CALDCO_8MHZ_VLOC)) = 0x92;
  
  const uint16_t CALBC1_8MHZ_VLOC = 0x10FD;
  *((uint8_t *) (MEMSPACE + CALBC1_8MHZ_VLOC)) = 0x8D;
  
  const uint16_t CALDCO_1MHZ_VLOC = 0x10FE;
  *((uint8_t *) (MEMSPACE + CALDCO_1MHZ_VLOC)) = 0xD1;
  
  const uint16_t CALBC1_1MHZ_VLOC = 0x10FF;
  *((uint8_t *) (MEMSPACE + CALBC1_1MHZ_VLOC)) = 0x86;  

  return MEMSPACE;
}

/* 
** Free MSP430 virtual memory
*/
void uninitialize_msp_memspace()
{
  free(MEMSPACE);
}


