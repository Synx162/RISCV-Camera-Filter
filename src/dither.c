#include "dither.h"

#include <stdlib.h>  // for rand()

#include "config.h"
#include "filters.h"

// ============================================================
// Duotone colour constants in RGB565
// Tweak these to taste — these match the "Who's Lila?" palette
// Dark reddish-brown:  R=10, G=8,  B=4  → dark warm shadow
// Cream/beige:         R=28, G=26, B=18 → warm highlight
// ============================================================

// ============================================================
// Step 1: Extract approximate luminance from an RGB565 pixel
// Proper weights: Y = 0.299R + 0.587G + 0.114B
// Integer approximation: Y ≈ (2R + 5G + 1B) >> 3  (scaled to 0..255)
// ============================================================
static inline int luminance(unsigned short pixel) {
  int r = (pixel >> 11) & 0x1F;  // 5-bit: 0..31
  int g = (pixel >> 5) & 0x3F;   // 6-bit: 0..63
  int b = (pixel) & 0x1F;        // 5-bit: 0..31

  // Scale all to 8-bit range first for consistent weighting
  r = (r << 3);  // 0..248
  g = (g << 2);  // 0..252
  b = (b << 3);  // 0..248

  // Weighted sum, divide to keep in 0..255
  return (2 * r + 5 * g + 1 * b) >> 3;
}

// ============================================================
// Step 2: Contrast stretch — push midtones toward extremes
// Clamps luminance so that anything below `lo` becomes 0
// and anything above `hi` becomes 255, stretching the rest.
// Good starting values: lo=60, hi=200
// ============================================================
static inline int contrastStretch(int lum, int lo, int hi) {
  if (lum <= lo) return 0;
  if (lum >= hi) return 255;
  return (lum - lo) * 255 / (hi - lo);
}

// ============================================================
// Step 3: 4x4 Bayer ordered dither matrix
// Each value is a threshold 0..15. Compare against lum/16
// to decide dark vs light at each pixel position.
// ============================================================
static const int bayer4x4[4][4] = {
    {0, 8, 2, 10}, {12, 4, 14, 6}, {3, 11, 1, 9}, {15, 7, 13, 5}};

// ============================================================
// Step 4 (optional): simple noise via LCG random
// Returns a value roughly in range [-strength, +strength]
// ============================================================
static int noiseVal = 12345;  // seed
static inline int nextNoise(int strength) {
  noiseVal = noiseVal * 1664525 + 1013904223;  // LCG
  int n = (noiseVal >> 16) & 0xFF;             // 0..255
  return (n * strength) / 128 - strength;      // -strength..+strength
}

void lilaFilter(volatile unsigned short* imageIn,
                volatile unsigned short* imageOut) {
  const int CONTRAST_LO = 60;   // crush shadows below this
  const int CONTRAST_HI = 200;  // blow highlights above this
  const int NOISE_STR = 12;     // noise strength (0 = off)

  for (int y = 0; y < IMG_HEIGHT; y++) {
    for (int x = 0; x < VGA_WIDTH; x++) {
      unsigned short pixel = imageIn[y * VGA_WIDTH + x];
      int lum = luminance(pixel);

      // --- Step 2: Contrast stretch ---
      lum = contrastStretch(lum, CONTRAST_LO, CONTRAST_HI);

      // --- Step 3: Optional noise ---
      lum += nextNoise(NOISE_STR);
      if (lum < 0) lum = 0;
      if (lum > 255) lum = 255;

      // --- Step 4: Bayer dither ---
      // Scale lum to 0..15, compare against matrix threshold
      int threshold = bayer4x4[y & 3][x & 3];  // & 3 = mod 4, fast
      int isLight = (lum >> 4) > threshold;    // lum/16 > threshold

      // --- Step 5: Duotone map ---
      imageOut[y * VGA_WIDTH + x] = isLight ? COLOR_LITE : COLOR_DARK;
    }
  }
}

void glitchFilter(volatile unsigned short* imageIn,
                  volatile unsigned short* imageOut) {
  for (int y = 0; y < IMG_HEIGHT; y++) {
    for (int x = 0; x < IMG_WIDTH; x++) {
      unsigned short color = (short)luminance(imageIn[y * VGA_WIDTH + x]);
      imageOut[y * VGA_WIDTH + x] = color;  // Copy Each Pixel
    }
  }
}

void grayFilter(volatile unsigned short* imageIn,
                volatile unsigned short* imageOut) {
  for (int y = 0; y < IMG_HEIGHT; y++) {
    for (int x = 0; x < IMG_WIDTH; x++) {
      unsigned short color =
          grayToRgb565((char)luminance(imageIn[y * VGA_WIDTH + x]));
      imageOut[y * VGA_WIDTH + x] = color;  // Copy Each Pixel
    }
  }
}