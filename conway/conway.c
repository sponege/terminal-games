#include <stdio.h>
#include <curses.h>
#include <unistd.h> /* for sleep */
#include <stdlib.h>
#include <string.h>
#include <time.h>   /* for random */

// Made by Jordan Perry
// 17/07/2020

// -- How To Run --
// Install ncurses:
// Debian: sudo apt-get install libncurses5-dev libncursesw5-dev
// Red Hat: sudo yum install ncurses-devel ncurses
// Compile binary:
// gcc conway.c -o conway -lncurses

// Configuration is below

#define frameTime 100000 // time between each frame

int w, h; // width and height of terminal screen
int edit; // edit mode is on?

int *grid = NULL; // contains all the cells in the game of life
int *grid2 = NULL; // used as a temporary grid to calculate cell positions without overwriting other cells in the process
int t; // used as a time regulator

void drawCells() {
  erase();
  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      attron(COLOR_PAIR(grid[x+(y*w)]+1)); // white is alive, default is dead
      move(y, x);
      addch(' '); // you can't see spaces, adding color
    }
  }
}

int getCell(int x, int y) {
  if (x < 0 || x >= w || y < 0 || y >= h) { // if out of bounds
    return 0; // no cells here
  } else {
    return grid[x+(y*w)]; // return correct cell
  }
}

void gameOfLife() { // logic for game of life
  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      int totalNeighbors = 0;
      for (int nX = -1; nX < 2; nX++) { // neighbor x, range of -1 to 1
        for (int nY = -1; nY < 2; nY++) { // same for y
          totalNeighbors += getCell(x + nX, y + nY);
        }
      }
      totalNeighbors -= getCell(x, y); // you can't be your own neighbor, stupid

      // I love links
      // https://en.wikipedia.org/wiki/Conway's_Game_of_Life#Rules

      if (getCell(x, y)) { // if current cell is alive
        if (totalNeighbors < 2 || totalNeighbors > 3) {
          // Any live cell with fewer than two live neighbours dies, as if by underpopulation.
          // Any live cell with more than three live neighbours dies, as if by overpopulation.
          grid2[x+(y*w)] = 0; // kill current cell
        }

        // Any live cell with two or three live neighbours lives on to the next generation.

      } else if (totalNeighbors == 3) { // Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.
        grid2[x+(y*w)] = 1; // it's alive!
      } else {
        grid2[x+(y*w)] = 0; // kill current cell
      }
    }
  }

  for (int i = 0; i < w*h; i++) {
    grid[i] = grid2[i]; // copy grid 2 to grid 1
  }
}

int main(int argc, char *argv[]) {
  initscr(); // initialize ncurses
  srand(time(0)); // init random

  getmaxyx(stdscr, h, w); // get width and height of screen

  nodelay(stdscr, TRUE); // don't want delay with getch input
  noecho(); // also, don't echo output
  curs_set(0); // hides cursor

  start_color(); // initialize colors

  init_pair(1, COLOR_WHITE, 0); // 1 is black
  init_pair(2, 0, COLOR_WHITE); // 2 is white

  grid = realloc(grid, (w*h) * sizeof *grid); // allocate space for cells, will fill whole screen
  grid2 = realloc(grid2, (w*h) * sizeof *grid2); // same for grid2

  if (argc >= 2 && argv[1][0] == 'e') { // e is for edit
    // edit mode

    int x = w / 2; // edit x
    int y = h / 2; // edit y
    int input;

    keypad(stdscr, TRUE); // I also want to use arrow keys for controlling the snake

    while (input != '\n') { // enter to start simulation
      input = getch();

      if (input == KEY_UP && y > 0) {
        y--;
      } else if (input == KEY_DOWN && y < h - 1) {
        y++;
      } else if (input == KEY_LEFT && x > 0) {
        x--;
      } else if (input == KEY_RIGHT && x < w - 1) {
        x++;
      } else if (input == ' ') {
        grid[x+(y*w)] = !grid[x+(y*w)]; // flip current cell
      }

      drawCells();
      move(y, x);
      attron(COLOR_PAIR(grid[x+(y*w)]+1)); // white is alive, default is dead
      addch('#');
      refresh();
    }
  } else {
    int randomNum = rand();
    int i;

    for (int y = 0; y < h; y++) {
      for (int x = 0; x < w; x++) {
        grid[x+(y*w)] = (randomNum >> i) & 1;
        i++;
        if (i >= 32) {
          randomNum = rand();
          i = 0;
        }
      }
    }
  }

  for (int i = 0; i < w*h; i++) {
    grid2[i] = grid[i]; // init grid 2
  }

  while (1) {
    t = time(0);
    drawCells();
    refresh();
    gameOfLife();
    usleep(frameTime - (time(0) - t));
  }
}
