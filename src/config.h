#ifndef CONFIG_H
#define CONFIG_H

/* ── register offsets as word indices ─────────────────────────────────── */
#define VID_BUFFER_A 0x08000000
#define VID_BUFFER_B 0x08000000
#define VGA_BUFFER_A 0x08100000
#define VGA_BUFFER_B 0x08200000

/* Snapshot buffer in SDRAM (320*240*2 = 153600 bytes) */
#define SNAPSHOT_BUFFER 0x01000000 /* SDRAM address for snapshot storage  */

/* Video/VGA DMA Controller  */
// (base 0xFF203060)
#define V_DMA_BUF 0     /* 0xFF203060 – Buffer start address        */
#define V_DMA_BACKBUF 1 /* 0xFF203064 – BackBuffer start address    */
#define V_DMA_RES 2     /* 0xFF203068 – Resolution (read-only)      */
#define V_DMA_CTRL 3    /* 0xFF20306C – Status(R) / Control(W)      */

/* VGA DMA Controller */

/* Pixel Buffer Controller (base 0xFF203020)*/
#define PIX_BUF 0     /* 0xFF203020 – Buffer (front, read-only)   */
#define PIX_BACKBUF 1 /* 0xFF203024 – BackBuffer (R/W)            */
#define PIX_RES 2     /* 0xFF203028 – Resolution (read-only)      */
#define PIX_STAT 3    /* 0xFF20302C – Status / Control            */

/* Display Constance  */
#define VGA_WIDTH 512  // Width for the frame buffer rows (320 + padding)
#define IMG_WIDTH 320  // Width of input image, used for calculations
#define IMG_HEIGHT 240

#define PIXEL_CONTROL_ADDRESS \
  0xFF203020  // Address for the frame buffer currently used
#define KEY_CONTROL_ADDRESS 0xFF200050  // Address KEY Control

#define SOBEL_SIZE 3
#define PS2_RVALID 0x8000

/* ------- PS2 keyboard make codes --------- */

// Letters
#define PS2_KEY_A 0x1C
#define PS2_KEY_B 0x32
#define PS2_KEY_C 0x21
#define PS2_KEY_D 0x23
#define PS2_KEY_E 0x24
#define PS2_KEY_F 0x2B
#define PS2_KEY_G 0x34
#define PS2_KEY_H 0x33
#define PS2_KEY_I 0x43
#define PS2_KEY_J 0x3B
#define PS2_KEY_K 0x42
#define PS2_KEY_L 0x4B
#define PS2_KEY_M 0x3A
#define PS2_KEY_N 0x31
#define PS2_KEY_O 0x44
#define PS2_KEY_P 0x4D
#define PS2_KEY_Q 0x15
#define PS2_KEY_R 0x2D
#define PS2_KEY_S 0x1B
#define PS2_KEY_T 0x2C
#define PS2_KEY_U 0x3C
#define PS2_KEY_V 0x2A
#define PS2_KEY_W 0x1D
#define PS2_KEY_X 0x22
#define PS2_KEY_Y 0x35
#define PS2_KEY_Z 0x1A

// Numbers
#define PS2_KEY_0 0x45
#define PS2_KEY_1 0x16
#define PS2_KEY_2 0x1E
#define PS2_KEY_3 0x26
#define PS2_KEY_4 0x25
#define PS2_KEY_5 0x2E
#define PS2_KEY_6 0x36
#define PS2_KEY_7 0x3D
#define PS2_KEY_8 0x3E
#define PS2_KEY_9 0x46

// Special keys
#define PS2_KEY_ENTER 0x5A
#define PS2_KEY_SPACE 0x29
#define PS2_KEY_BACKSPACE 0x66

#endif
