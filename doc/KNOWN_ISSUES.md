# Known issues

[← README](../README.md)

Found while documenting the project. None of them stop the demo from running, but each is worth fixing.

### 1. The software Sobel is not in the live path
Live edge detection (SW0) uses the DE1-SoC Computer's hardware edge-detect stage in the video-in pipeline. `sobelImage()` in `src/filters.c` is never called, and it convolves raw RGB565 words instead of a luminance value, so on colour frames it would mix the red, green and blue fields together. The software Sobel was only validated on 8-bit grayscale, in the CPULATOR prototype.
**Fix:** convert each pixel to luminance first (the `luminance()` helper in `dither.c` already does this), then add the filter to a key.

### 2. Video input is effectively single-buffered
`VID_BUFFER_A` and `VID_BUFFER_B` in `config.h` are both `0x08000000`, so `swapVID()` swaps a buffer with itself. The CPU can read a frame while the video DMA is overwriting it, which can show as tearing on fast motion. The VGA output side is properly double-buffered.
**Fix:** point `VID_BUFFER_B` at a second 320×240 region.

### 3. Duotone highlight renders pink instead of cream
`dither.h` picks the duotone colours as if green were 5 bits, but RGB565 green is 6 bits. The "cream" highlight `(28, 26, 18)` comes out as roughly `(224, 104, 144)`, which is pink. The dark tone has the same scaling error. You can see it in the gallery.
**Fix:** double the green values (`LITE_G 52`, `DARK_G 16`).

### 4. Filters called without prototypes
`main.c` calls `asciiImage`, `lilaFilter`, `glitchFilter` and `grayFilter` without a declaration in scope, so the compiler falls back to implicit declarations (an error by default in newer GCC). `dither.h` also declares `static inline` helpers that are only defined in `dither.c`.
**Fix:** add the prototypes to `filters.h` / `dither.h` and move the `static inline` declarations into `dither.c`.

### 5. Two filters process the row padding
`invertImage` and `lilaFilter` loop over all 512 columns of each row instead of the 320 visible ones, so they do about 60% more work than needed.
**Fix:** loop to `IMG_WIDTH` like the other filters.

### Minor
- The CPULATOR prototype declares its output as `unsigned char* imageOutput[...]` (an array of pointers) instead of `unsigned char imageOutput[...]`. It works, but uses four times the memory.
- The glitch filter writes 8-bit luminance straight into a 16-bit RGB565 word, so brightness lands in the blue and low green bits. That's where its false-colour look comes from; it's kept as an effect.
