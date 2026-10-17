"""Generate a synthetic 320x240 test scene for the host preview.

The scene has smooth gradients (for the dither and grayscale filters), hard
edges and thin lines (for edge detection), and saturated colours (for the
invert and glitch filters). Writes test_scene.ppm and test_scene.png.
"""
from PIL import Image, ImageDraw

W, H = 320, 240

img = Image.new("RGB", (W, H))
px = img.load()

# Sky-to-ground vertical gradient
for y in range(H):
    t = y / (H - 1)
    for x in range(W):
        if y < 150:
            px[x, y] = (int(40 + 60 * t), int(90 + 110 * t), int(200 - 40 * t))
        else:
            u = (y - 150) / (H - 151)
            px[x, y] = (int(70 + 60 * u), int(130 - 40 * u), int(50 + 10 * u))

d = ImageDraw.Draw(img)

# Sun with a radial falloff
for r in range(40, 0, -1):
    k = r / 40
    d.ellipse([240 - r, 55 - r, 240 + r, 55 + r],
              fill=(255, int(170 + 70 * (1 - k)), int(40 + 120 * (1 - k))))

# Hills
d.polygon([(0, 165), (70, 110), (140, 165)], fill=(60, 100, 70))
d.polygon([(90, 165), (180, 95), (270, 165)], fill=(45, 85, 60))

# House: hard edges and lines
d.rectangle([30, 150, 110, 215], fill=(200, 60, 50), outline=(20, 20, 20), width=2)
d.polygon([(25, 152), (70, 118), (115, 152)], fill=(110, 40, 30), outline=(20, 20, 20))
d.rectangle([60, 180, 80, 215], fill=(90, 55, 30))
d.rectangle([38, 162, 55, 177], fill=(250, 230, 120), outline=(20, 20, 20))
d.rectangle([86, 162, 103, 177], fill=(250, 230, 120), outline=(20, 20, 20))

# Colour bars
bars = [(255, 0, 0), (255, 160, 0), (255, 255, 0), (0, 200, 0),
        (0, 180, 255), (60, 60, 255), (200, 0, 200)]
for i, c in enumerate(bars):
    d.rectangle([150 + i * 22, 185, 150 + i * 22 + 20, 225], fill=c)

# Thin grid lines for edge detail
for x in range(150, 305, 11):
    d.line([(x, 175), (x, 180)], fill=(240, 240, 240))

img.save("test_scene.png")
with open("test_scene.ppm", "wb") as f:
    f.write(b"P6\n%d %d\n255\n" % (W, H))
    f.write(img.tobytes())
print("wrote test_scene.ppm / test_scene.png")
