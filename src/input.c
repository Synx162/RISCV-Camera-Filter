#include "input.h"

#include "address_map.h"
#include "config.h"

// ------ PS2 keyboard ----------

unsigned char get_key(void) {
  volatile int* ps2 = (int*)PS2_BASE;  // PS2 controller base address
  unsigned char key_press = 0;

  if (!(*ps2 & PS2_RVALID)) {  // check RVALID bit
    return 0xff;
  }

  key_press = *ps2 & 0xFF;  // read the raw byte from the FIFO

  // skip F0 (break code prefix) and E0 (extended key prefix)
  while (key_press == 0xF0 || key_press == 0xE0) {
    key_press = *ps2 & 0xFF;  // keep reading until we get a plain make code
  }

  return key_press;  // return the make code
}

// Display the last clicked key on teh HEX display as hex #s
void HEX_key(unsigned char make) {
  volatile int* HEX3_HEX0_ptr = (int*)HEX3_HEX0_BASE;  // HEX display base

  // 7-segment patterns for 0–F
  unsigned char seven_seg_decode_table[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D,
                                            0x7D, 0x07, 0x7F, 0x67, 0x77, 0x7C,
                                            0x39, 0x5E, 0x79, 0x71};

  unsigned char low = seven_seg_decode_table[make & 0x0F];  // low nibble → HEX0
  unsigned char high =
      seven_seg_decode_table[(make >> 4) & 0x0F];  // high nibble → HEX1

  *HEX3_HEX0_ptr =
      (high << 8) | low;  // concatenate both digits and write to HEX
}