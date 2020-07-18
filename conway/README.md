# Conway's Game of Life

[![asciicast](https://asciinema.org/a/348234.svg)](https://asciinema.org/a/348234)

# Compilation

This game requires ncurses. You can install it like so:

### Debian
```
sudo apt-get install libncurses5-dev libncursesw5-dev
```
### Red Hat
```
sudo yum install ncurses-devel ncurses
```

Then, to compile the binary, you can use this command:

```
gcc conway.c -o conway -lncurses
```

# How to Play

You can execute the program to start the simulation.

```
./conway
```

All the cell states are completely randomized. If you want to be able to edit the grid, you can use this command:

```
./conway edit
```

Or, for short:

```
./conway e
```

You can use the arrow keys to move the cursor around, and space to flip the current cell state. When you are satisfied with the masterpiece you created, you can simply press the enter key to start the simulation.

You can press q to quit when you are finished playing. Have fun!
