CC      := gcc
LIBS    := -lncurses
OUT_DIR := build
TARGETS := tetris snake conway video

.PHONY: $(TARGETS) clean install

all: $(TARGETS) | $(OUT_DIR)

$(OUT_DIR):
	mkdir $@

tetris: tetris/tetris.c | $(OUT_DIR)
	$(CC) $< -o $(OUT_DIR)/$@ $(LIBS)

snake: snake/snake.c | $(OUT_DIR)
	$(CC) $< -o $(OUT_DIR)/$@ $(LIBS)

conway: conway/conway.c | $(OUT_DIR)
	$(CC) $< -o $(OUT_DIR)/$@ $(LIBS)

video: video-player/video-player.c | $(OUT_DIR)
	$(CC) $< -o $(OUT_DIR)/$@ $(LIBS) -lpng

clean:
	rm -r $(OUT_DIR)

install: $(OUT_DIR)
	cp $(OUT_DIR)/* /bin
