# Host preview of the board's filters (gcc + Python 3 with Pillow).
#
#   make           build ./preview
#   make gallery   generate the test scene, run every filter, write PNGs
#                  to ../doc/gallery
#   make clean

CC      ?= gcc
CFLAGS  := -O2 -std=gnu99 -Wno-implicit-function-declaration
SRC     := ../src
OUT     := build
GALLERY := ../doc/gallery

# The CPULATOR prototype has its own main/displayImage/grayToRgb565;
# rename them so only sobel_filter() is used.
PROTO_DEFS := -Dmain=proto_main -DdisplayImage=proto_displayImage \
              -DgrayToRgb565=proto_grayToRgb565

.PHONY: all gallery clean

all: preview

$(OUT):
	@mkdir -p $(OUT)

$(OUT)/proto.o: ../prototype/cpulator_sobel.c | $(OUT)
	$(CC) $(CFLAGS) -w $(PROTO_DEFS) -c $< -o $@

preview: preview.c $(SRC)/filters.c $(SRC)/dither.c $(OUT)/proto.o
	$(CC) $(CFLAGS) -w -o $@ preview.c $(SRC)/filters.c $(SRC)/dither.c $(OUT)/proto.o

test_scene.ppm: make_test_scene.py
	python3 make_test_scene.py

gallery: preview test_scene.ppm | $(OUT)
	./preview test_scene.ppm $(OUT)
	@mkdir -p $(GALLERY)
	python3 -c "import glob,os;from PIL import Image;\
	[Image.open(p).save(os.path.join('$(GALLERY)',os.path.basename(p)[:-4]+'.png')) for p in glob.glob('$(OUT)/*.ppm')]"
	cp test_scene.png $(GALLERY)/input.png
	rm -f $(GALLERY)/0_passthrough.png

clean:
	rm -rf $(OUT) preview test_scene.ppm test_scene.png
