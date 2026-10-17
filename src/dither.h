#define DARK_R 10
#define DARK_G 8
#define DARK_B 4
#define LITE_R 28
#define LITE_G 26
#define LITE_B 18

// Pack R(5-bit), G(6-bit), B(5-bit) into RGB565
#define RGB565(r, g, b) (((r) << 11) | ((g) << 5) | (b))

#define COLOR_DARK RGB565(DARK_R, DARK_G, DARK_B)
#define COLOR_LITE RGB565(LITE_R, LITE_G, LITE_B)

static inline int luminance(unsigned short pixel);

static inline int contrastStretch(int lum, int lo, int hi);

void lilaFilter(volatile unsigned short* imageIn,
                volatile unsigned short* imageOut);

void glitchFilter(volatile unsigned short* imageIn,
                  volatile unsigned short* imageOut);

void grayFilter(volatile unsigned short* imageIn,
                volatile unsigned short* imageOut);
