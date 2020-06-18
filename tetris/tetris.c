#include <stdio.h>
#include <curses.h>
#include <unistd.h> /* for sleep */
#include <stdlib.h>
#include <string.h>
#include <time.h>   /* for random */

// Made by Jordan Perry
// 16/06/2020 - 17/06/2020

// -- How To Run --
// Install ncurses:
// Debian: sudo apt-get install libncurses5-dev libncursesw5-dev
// Red Hat: sudo yum install ncurses-devel ncurses
// Compile binary:
// gcc -lncurses tetris.c -o tetris

// You can edit the configuration below. Be sure to compile again after making your changes.
// The default configuration is from the Tetris Standard.
// https://tetris.fandom.com/wiki/Tetris_Guideline

// Keyboard configuration
// Make sure that letter keys are in lowercase

#define rotate1 KEY_UP
#define rotate2 'x'
#define rrotate 'z' // Rotate counterclockwise. Cannot detect control, sorry!
#define left KEY_LEFT
#define right KEY_RIGHT
#define softdrop KEY_DOWN
#define harddrop ' ' // space
#define holdKey 'c' // I also cannot detect shift. Sorry about that!

// Other

#define len 20 // length of grid
#define wid 10 // width of grid
#define nextLen 6 // how many next tetriminos are shown
#define dropTime 5000 // time it takes for each tetromino to fall, in microseconds
#define lockDelay 10000 // lock delay time, in microseconds

// End of configuration

int w, h; // width and height of terminal screen
char grid[wid][len];
// alright, this will be more simple than snake.
// the grid buffer goes from left to right, top to bottom.
// the buffer will store the type of tetrominoes as integers.
// see tetrominoes below.
int next[nextLen];
// next is the same, uses tetrominoes defined below.

const char tetrominoes[] = {'I', 'J', 'L', 'O', 'S', 'T', 'Z'}; // contains all tetrominoes

const int tetShape[sizeof tetrominoes][3][2] = {
  { // I
    {-2, 0},
    {-1, 0},
    {1, 0}
  },
  { // J
    {-1, -1},
    {-1, 0},
    {1, 0}
  },
  { // L
    {1, -1},
    {1, 0},
    {-1, 0}
  },
  { // O
    {0, -1},
    {1, -1},
    {1, 0}
  },
  { // S
    {-1, 0},
    {0, -1},
    {1, -1}
  },
  { // T
    {0, -1},
    {-1, 0},
    {1, 0}
  },
  { // Z
    {-1, -1},
    {0, -1},
    {1, 0}
  }
};
// info about tetShape
// each tetromino is made from 4 blocks
// we already know there's a center
// we just need to know the offsets of the other blocks of the tetromino
// they are in the same order as the other array above.
// remember that x goes right, and y goes down.


int order[] = {0, 1, 2, 3, 4, 5, 6, 7}; // order of tetrominoes, will be shuffled.
int orderCount; // current place of order, array reshuffles when you go over the max.

int cur; // current tetromino
int dir; // direction of tetromino, range of 0-3
int tX, tY; // position of tetromino
int timeToDrop; // time until tetromino drops, in microseconds.
int blocks[4][2]; // contains other parts of tetrimino, ignoring center.
int ghostOffset; // ghost offset
int lD = 0; // boolean, for lock delay.
int hold; // hold tetrimino
int holdCooldown; // if hold needs to cool down

int finished; // variable used to check if a loop is finished or not

WINDOW *gameWin; // the game window
WINDOW *nextWin; // next tetrominoes
WINDOW *holdWin; // shows hold

int x, y; // temporary variables

void shuffle(int *array, size_t n) // copied from https://stackoverflow.com/questions/6127503/shuffle-array-in-c#6127606
{
    if (n > 1)
    {
        size_t i;
        for (i = 0; i < n - 1; i++)
        {
          size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
          int t = array[j];
          array[j] = array[i];
          array[i] = t;
        }
    }
}

int getTetromino(int d, int t, int blocks[4][2]) { // returns the other 3 tetromino blocks when given the center and type of tetrimino.
  blocks[3][0] = 0; // ok, change my mind
  blocks[3][1] = 0; // we are returning all the blocks of the tetrimino :p
  for (int i = 0; i < 3; i++) { // direction handler
    int xC = 1;
    int yC = 1;
    if (tetrominoes[t] != 'O') { // square tetriminoes don't rotate.
      if (d == 1 || d == 2) {
        yC = -1;
      }
      if (d < 2) {
        xC = -1;
      }
      blocks[i][0] = (tetShape[t][i][1 * (d % 2)] * xC);
      blocks[i][1] = (tetShape[t][i][1 * (d % 2 == 0)] * yC);
      if (tetrominoes[t] == 'I') { // the straight tetromino rotates kinda weird :p
        if (d == 1 || d == 2) {
          blocks[i][0]++;
        }
        if (d > 1) {
          y++;
          blocks[i][1]++;
        }
      }
    } else {
      blocks[i][0] = tetShape[t][i][0];
      blocks[i][1] = tetShape[t][i][1];
    }
  }
  if (tetrominoes[t] == 'I') { // the straight tetromino rotates kinda weird :p
    if (d == 1 || d == 2) {
      blocks[3][0]++;
    }
    if (d > 1) {
      blocks[3][1]++;
    }
  }
}

int popBag() {
  int pop = order[orderCount]; // pop tetromino from bag
  orderCount++; // increment "stack pointer"
  if (orderCount > 6) { // if reached end of bag
    orderCount = 0; // reset "stack pointer"
    shuffle(order, 7); // shuffle order
  }
  return pop;
}

int popNext() {
  int pop = next[0]; // pop tetromino from bag
  for (int i = 0; i < nextLen - 1; i++) {
    next[i] = next[i+1]; // move all tetrominoes up one
  }
  next[nextLen - 1] = popBag(); // get another tetromino into next

  // draw new next blocks

  werase(nextWin);

  wattron(nextWin, COLOR_PAIR(8));

  wprintw(nextWin, "-NEXT-");

  int nextBlock[4][2];

  for (int i = 0; i < nextLen; i++) { // draw next blocks
    getTetromino(0, next[i], nextBlock);
    wattron(nextWin, COLOR_PAIR(next[i]+1));
    for (int n = 0; n < 4; n++) {
      wmove(nextWin, 3 + (i*4) + nextBlock[n][1], 2 + nextBlock[n][0]);
      waddch(nextWin, tetrominoes[next[i]]);
    }
  }

  wrefresh(nextWin);

  return pop; // finally, return tetromino
}

void findGhost() {
  ghostOffset = 0;
  finished = 0;

  while (true) { // find the y offset for ghost piece
    for (int i = 0; i < 4; i++) {
      x = blocks[i][0] + tX;
      y = blocks[i][1] + ghostOffset + tY;
      if (y == len || (grid[x][y] != 0 && x >= 0 && y >= 0)) {
        finished = 1;
        break;
      }
    }
    if (finished) {
      break;
    }
    ghostOffset++;
  }

  ghostOffset--;

  if (ghostOffset == -1 && tY < 3) { // game over
    endwin();
    exit(0);
  }
}

void updateHold() {
  werase(holdWin);

  wmove(holdWin, 0, 0);

  wattron(holdWin, COLOR_PAIR(8));

  wprintw(holdWin, "-HOLD-");

  if (hold != 0) {
    int holdBlock[4][2];
    getTetromino(0, hold-1, holdBlock);
    wattron(holdWin, COLOR_PAIR(hold));
    for (int n = 0; n < 4; n++) {
      wmove(holdWin, 3 + holdBlock[n][1], 2 + holdBlock[n][0]);
      waddch(holdWin, tetrominoes[hold-1]);
    }
  } else {
    wmove(holdWin, 2, 1);
    wprintw(holdWin, "None");
  }

  wrefresh(holdWin);
}

void hdrop() { // hard drop function
  holdCooldown = 0; // reset hold
  dir = 0;
  lD = 0;
  findGhost();

  for (int i = 0; i < 4; i++) {
    x = blocks[i][0] + tX;
    y = blocks[i][1] + tY + ghostOffset;
    grid[x][y] = cur + 1; // draw tetromino to grid
  }

  cur = popNext();

  tX = wid / 2 - 1;
  tY = 0;

  int linesCleared; // number of lines cleared

  for (int y = 0; y < len; y++) { // loop to find cleared lines
    int success = 1;
    for (int x = 0; x < wid; x++) {
      if (grid[x][y] == 0) {
        success = 0;
        break;
      }
    }
    if (success) {
      for (int rY = y; rY > 0; rY--) {
        for (int rX = 0; rX < wid; rX++) {
          grid[rX][rY] = grid[rX][rY-1];
        }
      }
    }
  }
}

void sdrop() { // soft drop function
  if (lD) {
    hdrop();
  } else {
    tY++;

    findGhost();

    if (ghostOffset == 0) {
      lD = 1;
      timeToDrop = lockDelay;
    }
  }
}

void processKeys() {
  int input = getch();
  int reverseRotate = 0; // if rotated, how to reverse rotate

  if (input == rotate1 || input == rotate2) { // rotate tetromino
    dir = (dir + 1) % 4;
    reverseRotate = 3;
  }

  if (input == rrotate) {
    dir = (dir + 3) % 4;
    reverseRotate = 1;
  }

  if (input == left) { // go left
    tX--;
  }

  if (input == right) { // go right
    tX++;
  }

  if (input == softdrop) {
    sdrop();
  }

  if (input == harddrop) {
    hdrop();
  }

  if (input == holdKey) {
    if (!holdCooldown) {
      int swap; // temporary swap variable
      swap = hold;
      hold = cur + 1;
      if (swap == 0) {
        cur = popNext();
      } else {
        cur = swap - 1;
      }
      holdCooldown = 1;
      updateHold();
    }
  }

  for (int i = 0; i < 4; i++) { // grid check
    x = blocks[i][0] + tX;
    y = blocks[i][1] + tY;

    if (x < 0) { // left check
      tX++;
    }

    if (x > wid - 1) { // right check
      tX--;
    }
/*
    if (grid[x][y] != 0) {
      if (input == left) {
        tX++;
      } else if (input == right) {
        tX--;
      } else {

      }
    }*/
  }

  if (timeToDrop <= 0) { // soft drop
    timeToDrop = dropTime;
    sdrop();
  } else {
    timeToDrop--;
  }

  //if (input == )
}

void drawGame() {
  werase(gameWin); // clear screen

  wattron(gameWin, COLOR_PAIR(8));
  box(gameWin, 0, 0); // I need those cool borders

  if (orderCount >= 7) {
    orderCount = 0;
  }

  for (int x = 0; x < wid; x++) {
    for (int y = 0; y < len; y++) {
      if (grid[x][y] != 0) {
        wmove(gameWin, y + 1, x + 1);
        wattron(gameWin, COLOR_PAIR(grid[x][y]));
        waddch(gameWin, tetrominoes[grid[x][y]-1]);
      }
    }
  }

  getTetromino(dir, cur, blocks);

  findGhost();

  wattron(gameWin, COLOR_PAIR(9)); // ghost piece is gray

  if (ghostOffset > 0) {
    for (int i = 0; i < 4; i++) { // draw ghost piece
      wmove(gameWin, blocks[i][1] + tY + 1 + ghostOffset, blocks[i][0] + tX + 1);
      waddch(gameWin, tetrominoes[cur]);
    }
  }

  wattron(gameWin, COLOR_PAIR(cur+1));

  for (int i = 0; i < 4; i++) { // draw current tetris piece
    if (blocks[i][1] + tY >= 0 || i == 3) {
      wmove(gameWin, blocks[i][1] + tY + 1, blocks[i][0] + tX + 1);
      waddch(gameWin, tetrominoes[cur]);
    }
  }

  wmove(gameWin, 0, 0);

  wrefresh(gameWin);
}

void printCenter(char *s, int offset) { // prints text at center of screen
  move((h / 2) + offset, (w - strlen(s)) / 2);
  printw(s);
}

int main() {
  initscr(); // initialize ncurses

  getmaxyx(stdscr, h, w); // get width and height of screen

  if (w < wid + 3 || h < len + 3) {
    printCenter("Screen too small!", -1);
    printCenter("Press any key to quit", 1);
    getch(); // wait for input
  } else {
    srand(time(0)); // init random

    nodelay(stdscr, TRUE); // don't want delay with getch input
    noecho(); // also, don't echo output
    keypad(stdscr, TRUE); // I'm also using arrow keys.
    start_color(); // initialize colors

    init_color(8, 700, 500, 0); // I'm using 8 as orange
    init_color(9, 500, 500, 500); // I need a gray as well.

    init_pair(1, COLOR_CYAN, 0); // I
    init_pair(2, COLOR_BLUE, 0); // J
    init_pair(3, 8, 0);          // L
    init_pair(4, COLOR_YELLOW, 0); // O
    init_pair(5, COLOR_GREEN, 0); // S
    init_pair(6, COLOR_MAGENTA, 0); // T
    init_pair(7, COLOR_RED, 0); // Z
    init_pair(8, COLOR_WHITE, 0); // default
    init_pair(9, 9, 0); // gray is used for ghost piece

    gameWin = newwin(len + 2, wid + 2, (h-len) / 2, (w-wid) / 2);
    nextWin = newwin(nextLen * 4 + 4, 6, (h-len) / 2 - 1, (w+wid) / 2 + 3);
    holdWin = newwin(10, 6, (h-len) / 2, (w-wid) / 2 - 7);

    timeToDrop = dropTime; // init timeToDrop
    shuffle(order, 7); // shuffle order

    for (int i = 0; i < nextLen; i++) { // fill next array
      next[i] = popBag();
    }

    getch(); // first run of getch, to make sure nothing is cleared for no reason.

    updateHold();

    cur = popNext(); // get current tetromino

    tX = wid / 2 - 1; // init x position

    while (1) { // game loop
      processKeys();

      drawGame();

      usleep(1);
    }
  }

  endwin();
}
