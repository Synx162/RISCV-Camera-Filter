// Host preview: runs the board's filter code on a PC.
//
// Loads a 320x240 binary PPM, packs it into an RGB565 buffer laid out like
// the DE1-SoC video buffer (512-pixel row stride), runs every filter from
// src/filters.c and src/dither.c, and writes one PPM per filter. The filter
// sources are compiled unchanged; only the MMIO helpers (swapVGA/swapVID)
// are linked in and never called.
//
//   make            build ./preview
//   make gallery    build, run on test_scene.ppm, write PNGs to ../doc/gallery

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/config.h"

// Prototypes for filters defined in src/filters.c and src/dither.c.
// main.c on the board relies on implicit declarations for some of these.
void displayImage(volatile unsigned short*, volatile unsigned short*);
void mirrorImage(volatile unsigned short*, volatile unsigned short*);
void invertImage(volatile unsigned short*, volatile unsigned short*);
void kaleidoImage(volatile unsigned short*, volatile unsigned short*);
void asciiImage(volatile unsigned short*, volatile unsigned short*);
void lilaFilter(volatile unsigned short*, volatile unsigned short*);
void glitchFilter(volatile unsigned short*, volatile unsigned short*);
void grayFilter(volatile unsigned short*, volatile unsigned short*);

// From prototype/cpulator_sobel.c (8-bit grayscale, 320-pixel stride)
void sobel_filter(unsigned char* imgInput, unsigned char* imgOutput);

#define BUF_PIXELS (IMG_HEIGHT * VGA_WIDTH)

static unsigned char rgb_in[IMG_HEIGHT][IMG_WIDTH][3];
static unsigned short in_buf[BUF_PIXELS];
static unsigned short out_buf[BUF_PIXELS];

static void read_ppm(const char* path) {
  FILE* f = fopen(path, "rb");
  if (!f) { perror(path); exit(1); }
  int w, h, maxv;
  if (fscanf(f, "P6 %d %d %d", &w, &h, &maxv) != 3 || w != IMG_WIDTH ||
      h != IMG_HEIGHT || maxv != 255) {
    fprintf(stderr, "%s: expected a 320x240 P6 PPM\n", path);
    exit(1);
  }
  fgetc(f);  // single whitespace after header
  if (fread(rgb_in, 1, sizeof rgb_in, f) != sizeof rgb_in) {
    fprintf(stderr, "%s: short read\n", path);
    exit(1);
  }
  fclose(f);
}

static void write_rgb565_ppm(const char* path, const unsigned short* buf) {
  FILE* f = fopen(path, "wb");
  if (!f) { perror(path); exit(1); }
  fprintf(f, "P6\n%d %d\n255\n", IMG_WIDTH, IMG_HEIGHT);
  for (int y = 0; y < IMG_HEIGHT; y++) {
    for (int x = 0; x < IMG_WIDTH; x++) {
      unsigned short p = buf[y * VGA_WIDTH + x];
      unsigned char r = ((p >> 11) & 0x1F) << 3;
      unsigned char g = ((p >> 5) & 0x3F) << 2;
      unsigned char b = (p & 0x1F) << 3;
      fputc(r, f); fputc(g, f); fputc(b, f);
    }
  }
  fclose(f);
}

static void write_gray_ppm(const char* path, const unsigned char* buf) {
  FILE* f = fopen(path, "wb");
  if (!f) { perror(path); exit(1); }
  fprintf(f, "P6\n%d %d\n255\n", IMG_WIDTH, IMG_HEIGHT);
  for (int i = 0; i < IMG_WIDTH * IMG_HEIGHT; i++) {
    fputc(buf[i], f); fputc(buf[i], f); fputc(buf[i], f);
  }
  fclose(f);
}

typedef void (*filter_fn)(volatile unsigned short*, volatile unsigned short*);

int main(int argc, char** argv) {
  if (argc != 3) {
    fprintf(stderr, "usage: %s input.ppm out_dir\n", argv[0]);
    return 1;
  }
  read_ppm(argv[1]);

  // Pack input into RGB565 with the board's 512-pixel row stride
  for (int y = 0; y < IMG_HEIGHT; y++)
    for (int x = 0; x < IMG_WIDTH; x++) {
      unsigned char* p = rgb_in[y][x];
      in_buf[y * VGA_WIDTH + x] =
          ((p[0] >> 3) << 11) | ((p[1] >> 2) << 5) | (p[2] >> 3);
    }

  struct { const char* name; filter_fn fn; } filters[] = {
      {"0_passthrough", displayImage}, {"1_mirror", mirrorImage},
      {"2_invert", invertImage},       {"3_kaleidoscope", kaleidoImage},
      {"4_grayscale", grayFilter},     {"5_duotone_dither", lilaFilter},
      {"6_ascii", asciiImage},         {"9_glitch", glitchFilter},
  };

  char path[512];
  for (size_t i = 0; i < sizeof filters / sizeof filters[0]; i++) {
    memset(out_buf, 0, sizeof out_buf);
    filters[i].fn(in_buf, out_buf);
    snprintf(path, sizeof path, "%s/%s.ppm", argv[2], filters[i].name);
    write_rgb565_ppm(path, out_buf);
  }

  // Software Sobel prototype runs on 8-bit grayscale
  static unsigned char gray[IMG_WIDTH * IMG_HEIGHT], edges[IMG_WIDTH * IMG_HEIGHT];
  for (int y = 0; y < IMG_HEIGHT; y++)
    for (int x = 0; x < IMG_WIDTH; x++) {
      unsigned char* p = rgb_in[y][x];
      gray[y * IMG_WIDTH + x] = (unsigned char)((299 * p[0] + 587 * p[1] + 114 * p[2]) / 1000);
    }
  sobel_filter(gray, edges);
  snprintf(path, sizeof path, "%s/sobel_prototype.ppm", argv[2]);
  write_gray_ppm(path, edges);

  printf("wrote %zu filter outputs + sobel_prototype to %s\n",
         sizeof filters / sizeof filters[0], argv[2]);
  return 0;
}
