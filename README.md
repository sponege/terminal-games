# Terminal Games

A repository full of games for Linux terminals.

# Dependencies

Sorry, windows users! These games require [ncurses](https://invisible-island.net/ncurses/announce.html), which can only run on UNIX-based operating systems. Sorry!

Here's a [good tutorial](https://ostechnix.com/how-to-install-ncurses-library-in-linux/) for Linux users on how to install the ncurses library.

If you want to compile every game on here, you can simply type this command into your terminal:

```
make
```

If you only wish to compile certain games, you can specify each one you want to compile like this:

```
make snake tetris
```

If you want to install the games onto your machine, you can use this command:

```
sudo make install
```

Once the games are installed, you can just type the game name into your terminal to play:

```
tetris
```

If you want to clean up after installing, you can use this command:

```
make clean
```

There's some configuration you can modify for each game in `config.h`. Be sure to recompile your game for changes to take effect.
