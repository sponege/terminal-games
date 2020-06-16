# Snake

[![asciicast](https://asciinema.org/a/s1cvI1I5PoahPCJBKbxwZadnn.svg)](https://asciinema.org/a/s1cvI1I5PoahPCJBKbxwZadnn)

I thought it would be pretty fun to write Snake in C, so I did.

I'm actually pretty proud of the result. I probably will make more games like this in the near future.

I decided used the text cursor as the food, because you can't move the cursor off the screen, but it actually looks pretty cool!

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
gcc -lncurses snake.c -o snake
```

You can then execute the binary and play! You can control the snake by using the arrow or WASD keys.
