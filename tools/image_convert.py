from PIL import Image
import numpy as np

# === CONFIG ===
INPUT_IMAGE = "image.png"
OUTPUT_IMAGE = "processed.png"
OUTPUT_HEADER = "image.h"
WIDTH = 320
HEIGHT = 240

# === LOAD IMAGE ===
img = Image.open(INPUT_IMAGE)

# === PROCESS ===
img = img.resize((WIDTH, HEIGHT), Image.BILINEAR)
img = img.convert("L")

# === SAVE PROCESSED IMAGE (PERMANENT) ===
img.save(OUTPUT_IMAGE)

# === CONVERT TO ARRAY ===
arr = np.array(img)
flat = arr.flatten()

# === WRITE HEADER FILE ===
with open(OUTPUT_HEADER, "w") as f:
    f.write("#ifndef IMAGE_H\n#define IMAGE_H\n\n")

    f.write("unsigned char imageInput[] = {\n")

    for i, val in enumerate(flat):
        f.write(f"0x{val:02X}, ")
        if (i + 1) % 16 == 0:
            f.write("\n")

    f.write("};\n\n")

    f.write("\n#endif\n")

print("Done.")
print(f"- Saved image: {OUTPUT_IMAGE}")
print(f"- Generated header: {OUTPUT_HEADER}")