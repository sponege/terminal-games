# Contributing

ASCII games are cool. No wonder you want to contribute!

Before you get started coding your awesome game, there's some things I want you to know:
- **These games are meant to be run on a Linux machine.** Write your game for Linux, not Windows. (If you want to make it for Windows however, do not let that deter you. You can make it and put it on your own repo.)
- The only dependency people should have to install on their machine should be ncurses. Nothing extra, except the standard C built-in libraries.
- Your game must be inside a folder in this repo with the same name as the game.
- You must add your game to the Makefile, so people can easily compile your game.
- Oh yeah, and your games must be **100% ASCII**. That means you can't use the ncurses `box()` to draw a fancy box to the screen. Some Linux Terminal VMs  completely ignore Unicode characters and instead simply displays them in ASCII. (The Linux VM for chromebooks does that, which sucks) So you might as well be consistent, because unicode won't work for everything.

# The Makefile

Let's assume there's already a game in here named Pong. Pong is the only game inside this repository currently. Here's what the makefile looks like so far:

```Makefile
CC      := gcc
LIBS    := -lncurses
OUT_DIR := build
TARGETS := pong

.PHONY: $(TARGETS) clean install

pong: pong/pong.c | $(OUT_DIR)
	$(CC) $< -o $(OUT_DIR)/$@ $(LIBS)

clean:
	rm -r $(OUT_DIR)

install: $(OUT_DIR)
	cp $(OUT_DIR)/* /bin
```

You decided to write Breakout. Here's what the Makefile should look like after you add your game in:

```Makefile
CC      := gcc
LIBS    := -lncurses
OUT_DIR := build
TARGETS := pong breakout

.PHONY: $(TARGETS) clean install

pong: pong/pong.c | $(OUT_DIR)
	$(CC) $< -o $(OUT_DIR)/$@ $(LIBS)

breakout: breakout/breakout.c | $(OUT_DIR)
	$(CC) $< -o $(OUT_DIR)/$@ $(LIBS)

clean:
	rm -r $(OUT_DIR)

install: $(OUT_DIR)
	cp $(OUT_DIR)/* /bin
```

You can notice that `breakout` is now in the `TARGETS`, and there's a section of the makefile telling the program how breakout should be compiled.

Here's what you need to do:

```Makefile
TARGETS := pong breakout
```

Add your game to the `TARGETS` inside the makefile, and make sure it is the exact name of your game.

```Makefile
breakout: breakout/breakout.c | $(OUT_DIR)
	$(CC) $< -o $(OUT_DIR)/$@ $(LIBS)
```

Tell `make` how to compile your program. `$(OUT_DIR)` is the build directory, all the game binaries should be in this folder once everything is compiled. If your game is more complex, you would have more here. But the games I wrote are fairly simple.

Make sure that your program compiles to one simgle standalone binary. That way, it's easy to install all the games to the user's machine. You just have to copy all the games to `/bin`

The last and final rule is that your game has to be fun to play, but I'll bet you could do that pretty easily.

If you're still wondering if you should make a game or not, I'll tell you this: Why not? Just do it for the heck of it, just for fun. You'll also get a lot of coding experience along the way.

C'mon, how cool would it be to have a repository full of 100+ ASCII-based games? That would be sweet!

So go ahead and start creating your awesome game! And once you're done, submit a pull request to show me your game, and I'll probably add it.

# Help and resources

Here's a [good C++ tutorial](https://www.w3schools.com/cpp/default.asp) you'll probably like. Just ignore the fact that classes exist at all in C++, and you're writing in C.

Here's an [excellent tutorial](http://www.cs.ukzn.ac.za/~hughm/os/notes/ncurses.html) about how to write programs for the ncurses library.

Everything else you need help with could probably be searched up easily. Now go ahead and create your awesome game!
