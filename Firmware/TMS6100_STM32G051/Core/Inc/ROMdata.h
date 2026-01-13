/*
 * ROMdata.h
 *
 *  Created: Dec 2025
 *  Author: Radiofan
 */

#define N_PHROMS 4

#define DLEN_ACORN 0x4000
#define DLEN_USA 0x3ede
#define DLEN_CLOCK 0x0f73
#define DLEN_Milit 0x3d96

extern const unsigned char phromData_Acorn[DLEN_ACORN];
extern const unsigned char phromData_USA[DLEN_USA];
extern const unsigned char PHROMdata_Clock[DLEN_CLOCK];
extern const unsigned char PHROMdata_Mil[DLEN_Milit];

typedef struct PHROM_desc {
  const unsigned char *address;
  const uint32_t rom_size;
  const uint32_t length;
} PHROM_desc;

typedef enum __attribute__((packed)) {
  RD_INIT,
  RD_ADDR,
  RD_DUMMY,
  RD_READY,
  RD_AOOB
} rd_state;

extern volatile uint8_t active;

