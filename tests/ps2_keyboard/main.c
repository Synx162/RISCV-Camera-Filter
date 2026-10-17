#include "address_map.h"

void HEX_key(unsigned char make) {
  volatile int* HEX3_HEX0_ptr = (int*)HEX3_HEX0_BASE;

  unsigned char seven_seg_decode_table[] = {
      0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07,
      0x7F, 0x67, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71};  // From 0 to F in hex

  // show high nibble on HEX1, low nibble on HEX0
  unsigned char low = seven_seg_decode_table[make & 0x0F];
  unsigned char high = seven_seg_decode_table[(make >> 4) & 0x0F];

  *HEX3_HEX0_ptr = (high << 8) | low;  // concatenate and put onto the HE
}

unsigned char get_key(void) {
  volatile int* ps2 = (int*)PS2_BASE;
  int data = 0;
  unsigned char key_press = 0;

  // wait for a valid make code, skip F0 and E0 prefix bytes
  while (!(data & 0x8000) || key_press == 0xF0 || key_press == 0xE0) {
    data = *ps2;
    key_press = data & 0xFF;
  }

  // drain the F0/E0 break prefix
  data = 0;
  while (!(data & 0x8000)) data = *ps2;

  // drain the repeated make code after F0
  data = 0;
  while (!(data & 0x8000)) data = *ps2;

  // return the original make code
  return key_press;
}

int main(void) {
  volatile int* ps2 = (int*)PS2_BASE;

  /* reset PS/2 device */
  *ps2 = 0xFF;

  while (1) {
    unsigned char k = get_key();
    HEX_key(k);
  }

  return 0;
}