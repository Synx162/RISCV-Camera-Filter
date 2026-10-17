#include "address_map.h"
#include "config.h"
#include "filters.h"
#include "input.h"

int main(void) {
  // Peripheral pointers
  volatile int* video_dma = (int*)VIDEO_IN_BASE;  // 0xFF203060

  volatile int* edge_ctrl = (int*)EDGE_DETECT_CTRL_BASE;  // 0xFF203070
  volatile int* pix_ctrl = (int*)PIXEL_BUF_CTRL_BASE;     // 0xFF203020

  volatile int* ledr = (int*)LEDR_BASE;                // 0xFF200000
  volatile int* HEX3_HEX0_ptr = (int*)HEX3_HEX0_BASE;  // 0xFF200020
  volatile int* HEX5_HEX4_ptr = (int*)HEX5_HEX4_BASE;  // 0xFF200030
  volatile int* sw = (int*)SW_BASE;                    // 0xFF200040
  volatile int* key = (int*)KEY_BASE;                  // 0xFF200050
  volatile int* ps2 = (int*)PS2_BASE;                  // 0xFF200100
  *ps2 = 0xFF;                                         // reset ps2 FIFO

  // Snapshot buffer (fixed SDRAM region)
  // 0x01000000 in SDRAM (SDRAM IS 0x00000000 TO 0x03FFFFFF)
  volatile unsigned short* snapshotBuf =
      (volatile unsigned short*)SNAPSHOT_BUFFER;

  /* ====Video Input Initialization=== */
  /* set front/back video buffer */
  *(video_dma + V_DMA_BACKBUF) = VID_BUFFER_A;  // store back buffer
  swapVID();                                    // swap
  *(video_dma + V_DMA_BACKBUF) = VID_BUFFER_B;  // store back buffer
  volatile short* vidBufferStart =
      (volatile short*)*(video_dma + V_DMA_BACKBUF);  // to draw on the back

  // Enable video input
  *(video_dma + V_DMA_CTRL) = (1 << 2);  // enable video input
  *(edge_ctrl) = 0;                      // turn off edge detection fliter

  /* =======Video Output Initialization=========== */
  /* set front/back pixel buffer */
  *(pix_ctrl + V_DMA_BACKBUF) = VGA_BUFFER_A;  // store back buffer
  swapVGA();                                   // Swap
  *(pix_ctrl + V_DMA_BACKBUF) = VGA_BUFFER_B;  // store back buffer
  volatile short* vgaBufferStart =
      (volatile short*)*(pix_ctrl + V_DMA_BACKBUF);  // to draw on the back

  // ---------- State variables ------------
  int edge_mode = 0;                   // 0 = SW edge detect off, 1 = on
  unsigned char key_pressed = 0;       // current PS2 key read this cycle
  unsigned char last_key_pressed = 1;  // last accepted key

  /* View_mode controls what is drawn each frame:
   *   0 = live camera  (default, also set by pressing C)
   *   1 = view snapshot (set by pressing 'V')              */
  int view_mode = 0;  // start in live-camera mode

  // has_snapshot tracks whether we have taken a picture at least once.
  // Used to prevent pressing 'V' before any snapshot exists.
  int has_snapshot = 0;  // no snapshot captured yet

  // ------ Main Loop ---------
  while (1) {
    // Read Values form SW (for now)
    int sw_val = *sw;

    // PS2 keyboard input
    key_pressed = get_key();  // returns 0xFF if no valid make code available

    // verify key
    if (key_pressed != 0xff && last_key_pressed != key_pressed) {
      // only latch filter keys, not mode-control keys
      if (key_pressed != PS2_KEY_SPACE && key_pressed != PS2_KEY_V &&
          key_pressed != PS2_KEY_C) {
        last_key_pressed = key_pressed;
      }
    }

    // ---------- Snapshot controls (Space_bar / V / C) -----------

    // 'Space_bar' pressed = save the current video frame into SDRAM.
    // stay in whatever display mode currently in.
    if (key_pressed == PS2_KEY_SPACE) {
      if (last_key_pressed == PS2_KEY_9)
        glitchFilter(vidBufferStart, (volatile short*)snapshotBuf);
      else if (last_key_pressed == PS2_KEY_6)
        asciiImage(vidBufferStart, (volatile short*)snapshotBuf);
      else if (last_key_pressed == PS2_KEY_5)
        lilaFilter(vidBufferStart, (volatile short*)snapshotBuf);
      else if (last_key_pressed == PS2_KEY_4)
        grayFilter(vidBufferStart, (volatile short*)snapshotBuf);
      else if (last_key_pressed == PS2_KEY_3)
        kaleidoImage(vidBufferStart, (volatile short*)snapshotBuf);
      else if (last_key_pressed == PS2_KEY_2)
        invertImage(vidBufferStart, (volatile short*)snapshotBuf);
      else if (last_key_pressed == PS2_KEY_1)
        mirrorImage(vidBufferStart, (volatile short*)snapshotBuf);
      else
        displayImage(vidBufferStart, (volatile short*)snapshotBuf);
      has_snapshot = 1;
      *ledr = 0x001;
    }
    // 'V' pressed: switch to frozen-snapshot display.
    else if (key_pressed == PS2_KEY_V && has_snapshot) {
      view_mode = 1;  // enter snapshot view mode
      *ledr = 0x002;  // light led1 = viewing snapshot

    } else if (key_pressed == PS2_KEY_C) {
      // 'C' pressed: return to live camera.
      view_mode = 0;  // back to live-camera mode
      has_snapshot =
          0;          // invalidate snapshot; must retake before viewing again
      *ledr = 0x000;  // clear LEDs
    }

    // HW Sobel Toggle -> SW0 - off: show raw input - on: show HW sobel
    int new_edge = (sw_val & 0b01) ? 1 : 0;
    if (new_edge != edge_mode) {
      edge_mode = new_edge;
      *edge_ctrl = edge_mode ? 1 : 0;
    }

    // --------- Draw frame -----------------
    // If view_mode == 1 show the snapshot regardless of which
    // filter key was last pressed.
    // When view_mode == 0  apply the selected filter to the live video input

    if (view_mode == 1) {
      // Snapshot mode, copy the saved SDRAM frame to the VGA back buffer
      displayImage((volatile unsigned short*)snapshotBuf,
                   (volatile unsigned short*)vgaBufferStart);
    } else {
      if (last_key_pressed == PS2_KEY_9)
        glitchFilter(vidBufferStart, vgaBufferStart);
      else if (last_key_pressed == PS2_KEY_6) {
        asciiImage(vidBufferStart, vgaBufferStart);
      } else if (last_key_pressed == PS2_KEY_5)
        lilaFilter(vidBufferStart, vgaBufferStart);
      else if (last_key_pressed == PS2_KEY_4)
        grayFilter(vidBufferStart, vgaBufferStart);
      // KEY3: kaleidoscope
      else if (last_key_pressed == PS2_KEY_3) {
        kaleidoImage(vidBufferStart, vgaBufferStart);
      }
      // KEY2: inverting
      else if (last_key_pressed == PS2_KEY_2) {
        invertImage(vidBufferStart, vgaBufferStart);
      }
      // KEY1: mirror
      else if (last_key_pressed == PS2_KEY_1) {
        mirrorImage(vidBufferStart, vgaBufferStart);
      }
      // default: display input
      else {
        displayImage(vidBufferStart, vgaBufferStart);
      }
    }

    swapVGA();  // flip VGA front/back; waits for vsync
    vgaBufferStart = (volatile short*)(*(pix_ctrl + V_DMA_BACKBUF));
    swapVID();  // flip video-input front/back
    vidBufferStart = (volatile short*)(*(video_dma + V_DMA_BACKBUF));
  }
}
