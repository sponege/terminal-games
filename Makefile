CC      := gcc
LIBS    := -lncurses
OUT_DIR := build
TARGETS := tetris snake conway

.PHONY: $(TARGETS) clean install

all: $(TARGETS) $(OUT_DIR)

$(OUT_DIR):
	mkdir $@

tetris: tetris/tetris.c tetris/config.h $(OUT_DIR)
	$(CC) $< -o $(OUT_DIR)/tetris $(LIBS)

snake: snake/snake.c snake/config.h $(OUT_DIR)
	$(CC) $< -o $(OUT_DIR)/snake $(LIBS)

conway: conway/conway.c conway/config.h $(OUT_DIR)
	$(CC) $< -o $(OUT_DIR)/conway $(LIBS)

clean:
	rm -r $(OUT_DIR)

install:
	cp $(OUT_DIR)/* /bin
