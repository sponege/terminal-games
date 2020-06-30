#include <stdio.h>
#include <curses.h>
#include <unistd.h> /* for sleep */
#include <stdlib.h>
#include <string.h>
#include <time.h>   /* for random */

// Made by Jordan Perry
// 16/06/2020 - 18/06/2020

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
#define nextLen 6 // how many next tetrominoes are shown
#define dropTime 20000 // time it takes for each tetromino to fall, in microseconds
                      // could not find a good standard for this, so adjust as needed
#define lockDelay 10000 // lock delay time, in microseconds.
                        // it doesn't look like microsecond though, so I don't know.
                        // the standard is 1 second lock dela, and I timed this to about a second.
#define specialLineValues 1 // search for "line values" using ctrl+f on https://tetris.fandom.com/wiki/Tetris_Guideline
#define secondsToStart 3 // number of seconds before game starts

// End of configuration

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

int w, h; // width and height of terminal screen
char grid[wid][len];
// alright, this will be more simple than snake.
// the grid buffer goes from left to right, top to bottom.
// the buffer will store the type of tetrominoes as integers.
// see tetrominoes below.
int next[nextLen];
// next is the same, uses tetrominoes defined below.


int order[] = {0, 1, 2, 3, 4, 5, 6, 7}; // order of tetrominoes, will be shuffled.
int orderCount; // current place of order, array reshuffles when you go over the max.

int cur; // current tetromino
int dir; // direction of tetromino, range of 0-3
int tX, tY; // position of tetromino
int timeToDrop; // time until tetromino drops, in microseconds.
int blocks[4][2]; // contains other parts of tetromino, ignoring center.
int ghostOffset; // ghost offset
int lD = 0; // boolean, for lock delay.
int hold; // hold tetromino
int holdCooldown; // if hold needs to cool down

int level = 1; // which level you are currently on
int lines; // progress towards your next level
int totalLines; // total lines completed

// these just count the types of line clears you got
int single;
int doubleCount;
int triple;
int tetris;

int finished; // variable used to check if a loop is finished or not

WINDOW *gameWin; // the game window
WINDOW *nextWin; // next tetrominoes
WINDOW *holdWin; // shows hold
WINDOW *statsWin; // shows stats, such as level and lines

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

int getTetromino(int d, int t, int blocks[4][2]) { // returns the other 3 tetromino blocks when given the center and type of tetromino.
  blocks[3][0] = 0; // ok, change my mind
  blocks[3][1] = 0; // we are returning all the blocks of the tetromino :p
  for (int i = 0; i < 3; i++) { // direction handler
    int xC = 1;
    int yC = 1;
    if (tetrominoes[t] != 'O') { // square tetrominoes don't rotate.
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

void updateNext() {
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

  updateNext();

  return pop; // finally, return tetromino
}

void wprintCenter(WINDOW *win, char *s, int w, int h, int offset) { // prints text at center of screen
  wmove(win, (h / 2) + offset, (w - strlen(s)) / 2);
  wprintw(win, s);
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
}

void updateStats() {
  char buf[6];
  werase(statsWin);

  wmove(statsWin, 0, 2);
  wprintw(statsWin, "LEVEL");

  sprintf(buf, "%d", level);
  wmove(statsWin, 2, 7 - strlen(buf));
  wprintw(statsWin, buf);

  wmove(statsWin, 4, 2);
  wprintw(statsWin, "LINES");

  sprintf(buf, "%d/%d", lines, level * 5);
  wmove(statsWin, 6, 7 - strlen(buf));
  wprintw(statsWin, buf);

  wrefresh(statsWin);
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

int collision() { // check for collision
  getTetromino(dir, cur, blocks);

  for (int i = 0; i < 4; i++) { // grid check
    x = blocks[i][0] + tX;
    y = blocks[i][1] + tY;

    if (x < 0) { // left check
      return 1;
    }

    if (x >= wid) { // right check
      return 1;
    }

    if (y >= len) { // bottom collision check
      return 1;
    }

    if (grid[x][y] != 0 && y > 0) {
      return 1;
    }
  }

  return 0;
}

void gameOver() {
  char buf[wid];
  werase(gameWin);
  wattron(gameWin, COLOR_PAIR(8));
  box(gameWin, 0, 0); // I need those cool borders
  wprintCenter(gameWin, "-STATS-", wid+2, len+2, -10);
  wprintCenter(gameWin, "Total", wid+2, len+2, -8);
  wprintCenter(gameWin, "Lines", wid+2, len+2, -7);
  sprintf(buf, "%d", totalLines);
  wprintCenter(gameWin, buf, wid+2, len+2, -6);
  wprintCenter(gameWin, "Single", wid+2, len+2, -4);
  sprintf(buf, "%d", single);
  wprintCenter(gameWin, buf, wid+2, len+2, -3);
  wprintCenter(gameWin, "Double", wid+2, len+2, -1);
  sprintf(buf, "%d", doubleCount);
  wprintCenter(gameWin, buf, wid+2, len+2, 0);
  wprintCenter(gameWin, "Triple", wid+2, len+2, 2);
  sprintf(buf, "%d", triple);
  wprintCenter(gameWin, buf, wid+2, len+2, 3);
  wprintCenter(gameWin, "Tetris", wid+2, len+2, 5);
  sprintf(buf, "%d", tetris);
  wprintCenter(gameWin, buf, wid+2, len+2, 6);

  wattron(gameWin, A_STANDOUT);
  wprintCenter(gameWin, "EXIT", wid+2, len+2, 8);

  wmove(gameWin, 0, 0);

  wrefresh(gameWin);

  while (getch() != '\n'){}; // wait for enter to be pressed
  endwin(); // end window
  exit(0); // exit
}

void hdrop() { // hard drop function
  holdCooldown = 0; // reset hold
  dir = 0;
  lD = 0;
  findGhost();

  for (int i = 0; i < 4; i++) {
    if (y < 1) {
      gameOver();
    }
    x = blocks[i][0] + tX;
    y = blocks[i][1] + tY + ghostOffset;
    grid[x][y] = cur + 1; // draw tetromino to grid
  }

  cur = popNext();

  tX = wid / 2 - 1; // reset x
  tY = 0; // and y position

  int linesCleared = 0; // number of lines cleared

  for (int y = 0; y < len; y++) { // loop to find cleared lines
    int success = 1;
    for (int x = 0; x < wid; x++) {
      if (grid[x][y] == 0) {
        success = 0;
        break;
      }
    }
    if (success) {
      for (int rY = y; rY > 0; rY--) { // loop to clear lines
        for (int rX = 0; rX < wid; rX++) {
          grid[rX][rY] = grid[rX][rY-1];
        }
      }
      linesCleared++;
    }
  }

  if (linesCleared > 0) {
    totalLines += linesCleared;

    switch (linesCleared) {
      case 1:
        single++;
        break;
      case 2:
        doubleCount++;
        break;
      case 3:
        triple++;
        break;
      case 4:
        tetris++;
        break;
    }

    if (specialLineValues) {
      switch (linesCleared) {
        case 1:
          lines += 1;
          break;
        case 2:
          lines += 3;
          break;
        case 3:
          lines += 5;
          break;
        case 4:
          lines += 8;
          break;
      }
    } else {
      lines += linesCleared;
    }

    while (lines >= level * 5) {
      lines = lines - (level * 5);
      level++;
    }

    updateStats();
  }
}

void sdrop(int a) { // soft drop function
  if (lD) {
    if (a) {
      hdrop();
    }
  } else {
    tY++;

    if (collision()) {
      tY--;
    }

    findGhost();

    if (ghostOffset == 0) {
      lD = 1;
      timeToDrop = lockDelay;
    }
  }
}

void resetLockDelay() {
  findGhost();

  if (ghostOffset > 0 && lD) {
    lD = 0;
    timeToDrop = dropTime / level;
  }
}

int LURotCheck() { // left up right rotation check
  // rotationg a tetromino can move the tetromino's position.
  // this check is used to see if it is possible to move a pieces position left, up, or down upon rotation.
  int tet = tetrominoes[cur]; // short for tetromino
  if (tet != 'O') { // squares don't rotate
    int moveAmount = 1;
    if (tet == 'I') {
      moveAmount = 2;
    }
    for (int i = 0; i < moveAmount; i++) { // left check
      tX -= 1;
      if (!collision()) break;
    }
    if (collision()) {
      tX += moveAmount; // undo
      for (int i = 0; i < moveAmount; i++) { // right check
        tX += 1;
        if (!collision()) break;
      }
      if (collision()) {
        tX -= moveAmount; // undo
        for (int i = 0; i < moveAmount; i++) { // top check
          tY -= 1;
          if (!collision()) break;
        }
        if (collision()) {
          tY += moveAmount; // undo
          return 1;
        }
      }
    }
    return 0;
  }
}

void processKeys() {
  int input = getch();

  if (input == rotate1 || input == rotate2) { // rotate tetromino
    dir = (dir + 1) % 4;
    if (collision() && LURotCheck()) {
      dir = (dir + 3) % 4;
    }
    resetLockDelay();
  }

  if (input == rrotate) {
    dir = (dir + 3) % 4;
    if (collision() && LURotCheck()) {
      dir = (dir + 1) % 4;
    }
    resetLockDelay();
  }

  if (input == left) { // go left
    tX--;
    if (collision()) {
      tX++;
    }
    resetLockDelay();
  }

  if (input == right) { // go right
    tX++;
    if (collision()) {
      tX--;
    }
    resetLockDelay();
  }

  if (input == softdrop) {
    sdrop(0);
  }

  if (input == harddrop) {
    hdrop();
  }

  if (input == holdKey) {
    if (!holdCooldown) {
      int swap; // temporary swap variable
      swap = hold;
      hold = cur + 1;

      tX = wid / 2 - 1; // reset x
      tY = 0; // and y position
      lD = 0; // reset lock delay because we aren't touching ground anymore.

      if (swap == 0) {
        cur = popNext();
      } else {
        cur = swap - 1;
      }
      holdCooldown = 1;
      updateHold();
    }
  }

  if (timeToDrop <= 0) { // soft drop
    timeToDrop = dropTime / level;
    sdrop(1);
  } else {
    timeToDrop--;
  }
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

  if (w < wid + 13 || h < len + 4) {
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
    holdWin = newwin(10, 6, (h-len) / 2, (w-wid) / 2 - 8);
    statsWin = newwin(8, 7, (h+len) / 2 - 6, (w-wid) / 2 - 8);

    timeToDrop = dropTime / level;
    shuffle(order, 7); // shuffle order

    for (int i = 0; i < nextLen; i++) { // fill next array
      next[i] = popBag();
    }

    tX = wid / 2 - 1; // init x position

    getch(); // first run of getch, to make sure nothing is cleared for no reason.

    updateHold();
    updateStats();
    updateNext();

    wattron(gameWin, COLOR_PAIR(8)); // default colors

    for (int i = secondsToStart; i > 0; i--) { // countdown
      char num[wid]; // for whatever reason, cannot have length of 1.
      sprintf(num, "%d", i); // format number
      werase(gameWin); // clear screen
      box(gameWin, 0, 0); // I need those cool borders
      wprintCenter(gameWin, num, wid+2, len+2, 0);
      wmove(gameWin, 0, 0);
      wrefresh(gameWin);
      sleep(1);
    }

    werase(gameWin); // clear screen
    box(gameWin, 0, 0); // I need those cool borders
    wprintCenter(gameWin, "START!", wid+2, len+2, 0);
    wmove(gameWin, 0, 0);
    wrefresh(gameWin);
    sleep(1);

    cur = popNext(); // get current tetromino

    while (1) { // game loop
      processKeys();

      drawGame();

      usleep(1);
    }
  }

  endwin();
}
