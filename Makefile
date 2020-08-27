CC      := gcc
LIBS    := -lncurses
OUT_DIR := build
TARGETS := tetris snake conway

.PHONY: $(TARGETS) clean install

all: $(TARGETS) | $(OUT_DIR)

$(OUT_DIR):
	mkdir $@

tetris: tetris/tetris.c tetris/config.h | $(OUT_DIR)
	$(CC) $< -o $(OUT_DIR)/$@ $(LIBS)

snake: snake/snake.c snake/config.h | $(OUT_DIR)
	$(CC) $< -o $(OUT_DIR)/$@ $(LIBS)

conway: conway/conway.c conway/config.h | $(OUT_DIR)
	$(CC) $< -o $(OUT_DIR)/$@ $(LIBS)

clean:
	rm -r $(OUT_DIR)

install: $(OUT_DIR)
	cp $(OUT_DIR)/* /bin
