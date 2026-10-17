#include "filters.h"

#include <stdio.h>
#include <stdlib.h>

#include "config.h"

/* ===All different filters===============================
1. no filter
2. mirror from centre
3. software sobel
4. inverting
5. kaleidoscope
6. ascii filter
*/

// 1. no filter
void displayImage(volatile unsigned short* imageIn,
                  volatile unsigned short* ImageOut) {
  for (int y = 0; y < IMG_HEIGHT; y++) {
    for (int x = 0; x < IMG_WIDTH; x++) {
      unsigned short color = imageIn[y * VGA_WIDTH + x];
      ImageOut[y * VGA_WIDTH + x] = color;  // Copy Each Pixel
    }
  }
}

// 2. mirror from centre
void mirrorImage(volatile unsigned short* imageIn,
                 volatile unsigned short* ImageOut) {
  for (int y = 0; y < IMG_HEIGHT; y++) {
    for (int x = 0; x < (IMG_WIDTH / 2); x++) {
      unsigned short color = imageIn[y * VGA_WIDTH + x];
      // Mirroring from the middle
      ImageOut[y * VGA_WIDTH + x] = color;
      ImageOut[y * VGA_WIDTH + (IMG_WIDTH - x - 1)] = color;
    }
  }
}

// 3. sobel filtersa
void sobelImage(volatile unsigned short* imageIn,
                volatile unsigned short* ImageOut) {
  for (int h = 1; h < IMG_HEIGHT - 1; h++) {
    for (int w = 1; w < IMG_WIDTH - 1; w++) {
      int Gx = -imageIn[(h - 1) * VGA_WIDTH + (w - 1)] +
               imageIn[(h - 1) * VGA_WIDTH + (w + 1)] +
               -2 * imageIn[h * VGA_WIDTH + (w - 1)] +
               2 * imageIn[h * VGA_WIDTH + (w + 1)] +
               -imageIn[(h + 1) * VGA_WIDTH + (w - 1)] +
               imageIn[(h + 1) * VGA_WIDTH + (w + 1)];

      int Gy = -imageIn[(h - 1) * VGA_WIDTH + (w - 1)] -
               2 * imageIn[(h - 1) * VGA_WIDTH + w] -
               imageIn[(h - 1) * VGA_WIDTH + (w + 1)] +
               imageIn[(h + 1) * VGA_WIDTH + (w - 1)] +
               2 * imageIn[(h + 1) * VGA_WIDTH + w] +
               imageIn[(h + 1) * VGA_WIDTH + (w + 1)];

      int G = abs(Gx) + abs(Gy);

      if (G > 255) G = 255;
      ImageOut[h * VGA_WIDTH + w] = G;
    }
  }
  return;
}

// 4. invert Image
void invertImage(volatile unsigned short* imageIn,
                 volatile unsigned short* ImageOut) {
  for (int y = 0; y < IMG_HEIGHT; y++) {
    for (int x = 0; x < VGA_WIDTH; x++) {
      unsigned short color = imageIn[y * VGA_WIDTH + x];
      ImageOut[y * VGA_WIDTH + x] = (color ^ 0xffff);  // invert Each Pixel
    }
  }
}

// 5. kaleidoscope
void kaleidoImage(volatile unsigned short* imageIn,
                  volatile unsigned short* imageOut) {
  int halfH = 120;
  int halfW = 160;
  for (int y = 0; y < halfH; y++) {
    for (int x = 0; x < halfW; x++) {
      // Map x,y into a square by scaling, then enforce x >= y (upper triangle)
      // Scale y to match x's range so the diagonal makes sense
      int sx = x;
      int sy = y;

      if (sy * halfW > sx * halfH) {
        // Swap, then scale back to rectangle coords
        int tmp = sx;
        sx = sy * halfW / halfH;   // scale y → x range
        sy = tmp * halfH / halfW;  // scale x → y range
      }

      unsigned short color = imageIn[sy * VGA_WIDTH + sx];

      // Write all 4 quadrant mirrors
      imageOut[y * VGA_WIDTH + x] = color;                     // TL
      imageOut[y * VGA_WIDTH + (IMG_WIDTH - 1 - x)] = color;   // TR
      imageOut[(IMG_HEIGHT - 1 - y) * VGA_WIDTH + x] = color;  // BL
      imageOut[(IMG_HEIGHT - 1 - y) * VGA_WIDTH + (IMG_WIDTH - 1 - x)] =
          color;  // BR
    }
  }
}

// 6. ascii art
// divides the image into 4x8 pixel cells, maps average brightness to a
// character density ramp, draws that character's bitmap in the cell's color
void asciiImage(volatile unsigned short* imageIn,
                volatile unsigned short* imageOut) {
  // density ramp: index 0 = darkest (fewest lit pixels) → 7 = brightest
  // characters: ' ' '.' ':' '-' '=' '+' '#' '@'
  static const unsigned char font[8][8] = {
      {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},  // ' '
      {0x0, 0x0, 0x0, 0x6, 0x6, 0x0, 0x0, 0x0},  // '.'
      {0x0, 0x6, 0x6, 0x0, 0x0, 0x6, 0x6, 0x0},  // ':'
      {0x0, 0x0, 0x0, 0xF, 0x0, 0x0, 0x0, 0x0},  // '-'
      {0x0, 0x0, 0xF, 0x0, 0xF, 0x0, 0x0, 0x0},  // '='
      {0x0, 0x2, 0x2, 0xF, 0x2, 0x2, 0x0, 0x0},  // '+'
      {0x0, 0xA, 0xF, 0xA, 0xA, 0xF, 0xA, 0x0},  // '#'
      {0x6, 0x9, 0xB, 0xB, 0x8, 0x9, 0x6, 0x0},  // '@'
  };

  for (int row = 0; row < IMG_HEIGHT / 8; row++) {   // 30 character rows
    for (int col = 0; col < IMG_WIDTH / 4; col++) {  // 80 character columns
      // accumulate RGB components over the 4x8 = 32 pixel cell
      int sum_r = 0, sum_g = 0, sum_b = 0;
      for (int dy = 0; dy < 8; dy++) {
        for (int dx = 0; dx < 4; dx++) {
          unsigned short px =
              imageIn[(row * 8 + dy) * VGA_WIDTH + (col * 4 + dx)];
          sum_r += (px >> 11) & 0x1F;  // 5-bit red
          sum_g += (px >> 5) & 0x3F;   // 6-bit green
          sum_b += px & 0x1F;          // 5-bit blue
        }
      }
      // average keeping each channel in its native bit-depth range
      int avg_r = sum_r / 32;  // 0-31  (5-bit)
      int avg_g = sum_g / 32;  // 0-63  (6-bit)
      int avg_b = sum_b / 32;  // 0-31  (5-bit)

      // approximate luminance 0-249, scaled to index 0-7
      int lum = ((avg_r << 3) + (avg_g << 2) + (avg_b << 3)) / 3;
      int idx = (lum * 8) / 256;
      if (idx > 7) idx = 7;

      // pack average color back to RGB565 for drawing the character pixels
      unsigned short color = (avg_r << 11) | (avg_g << 5) | avg_b;

      // stamp the 4x8 character bitmap into the output buffer
      for (int dy = 0; dy < 8; dy++) {
        unsigned char bits = font[idx][dy];  // 4 bits = 4 pixels per row
        for (int dx = 0; dx < 4; dx++) {
          // bit 3 = leftmost pixel, bit 0 = rightmost
          unsigned short out = ((bits >> (3 - dx)) & 1) ? color : 0x0000;
          imageOut[(row * 8 + dy) * VGA_WIDTH + (col * 4 + dx)] = out;
        }
      }
    }
  }
}

/* ==========Helper Functions for filters and display============ */

void swapVGA() {
  volatile int* pixel_ctrl_ptr = (int*)0xFF203020;
  int status;
  *pixel_ctrl_ptr = 1;  // Trigger buffer swap
  status = *(pixel_ctrl_ptr + 3);
  while ((status & 0x01) != 0) {  // Wait for sync
    status = *(pixel_ctrl_ptr + 3);
  }
}

void swapVID() {
  volatile int* video_dma = (int*)0xFF203060;
  *video_dma = 1;  // Trigger buffer swap
}

// Function to convert the 8-bit grayscale pixels to 16-bit rgb pixels for the
// VGA
unsigned short grayToRgb565(unsigned char gray) {
  unsigned short r = (gray >> 3);
  unsigned short g = (gray >> 2);
  unsigned short b = (gray >> 3);
  return (r << 11) | (g << 5) |
         b;  // Concatenate all into one 16-bit pixel RRRRR GGGGGG BBBBB
}
