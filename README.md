# Terminal Games

I'm stuck at home and have nothing to do. I thought I might as well make some games!

# Dependencies

Unfortunately, none of these games can run on windows. These games require [ncurses](http://www.cs.ukzn.ac.za/~hughm/os/notes/ncurses.html), which can only run on UNIX-based operating systems. Sorry!

If you are on a UNIX-based operating system, you can use [these instructions](https://stackoverflow.com/questions/8792317/where-is-the-conio-h-header-file-on-linux-why-cant-i-find-conio-h#8792443) to install ncurses.

After you install that, you can compile all the games using gcc.

This is what the command should look like:

```
gcc game.c -o game -lncurses
```

After you compile the program, all you have to do is run it and enjoy the game!

```
./game
```

Yes, I know there's something called `conio.h` that can be used on windows, and I could have just used that. But I hate windows, so sue me.
