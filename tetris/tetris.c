#include <stdio.h>
#include <curses.h>
#include <unistd.h> /* for sleep */
#include <stdlib.h>
#include <string.h>
#include <time.h>   /* for random */
#include "config.h" /* user configuration */

// Made by Jordan Perry
// 16/06/2020 - 18/06/2020

// -- How To Run --
// Install ncurses:
// Debian: sudo apt-get install libncurses5-dev libncursesw5-dev
// Red Hat: sudo yum install ncurses-devel ncurses
// Compile binary:
// gcc tetris.c -o tetris -lncurses

const char tetrominoes[] = {'I', 'J', 'L', 'O', 'S', 'T', 'Z'}; // contains all tetrominoes

const int tetShape[sizeof tetrominoes][3][2] = {
  { // I
    {-2, 0},
    {-1, 0},
    {1, 0}
  },
  { // J
    {1, -1},
    {1, 0},
    {-1, 0}
  },
  { // L
    {-1, -1},
    {-1, 0},
    {1, 0}
  },
  { // O
    {0, -1},
    {1, -1},
    {1, 0}
  },
  { // S
    {-1, -1},
    {0, -1},
    {1, 0}
  },
  { // T
    {0, -1},
    {-1, 0},
    {1, 0}
  },
  { // Z
    {-1, 0},
    {0, -1},
    {1, -1}
  }
};
// info about tetShape
// each tetromino is made from 4 blocks
// we already know there's a center
// we just need to know the offsets of the other blocks of the tetromino
// they are in the same order as the other array above.
// remember that x goes right, and y goes down.

const int wallKickData[8][4][2] = {
  { // 0->R
    // 5 tests per rotation, but I only need to store four becauses the first test is always the origin
    {-1, 0}, // x and y positions
    {-1, 1},
    {0, -2},
    {-1, -2}
  },
  { // R->0
    {1, 0},
    {1, -1},
    {0, 2},
    {1, 2}
  },
  { // R->2
    {1, 0},
    {1, -1},
    {0, 2},
    {1, 2}
  },
  { // 2->R
    {-1, 0},
    {-1, 1},
    {0, -2},
    {-1, -2}
  },
  { // 2->L
    {1, 0},
    {1, 1},
    {0, -2},
    {1, -2}
  },
  { // L->2
    {-1, 0},
    {-1, -1},
    {0, 2},
    {-1, 2}
  },
  { // L->0
    {-1, 0},
    {-1, -1},
    {0, 2},
    {-1, 2}
  },
  { // 0->L
    {1, 0},
    {1, 1},
    {0, -2},
    {1, -2}
  }
};

const int stickKickData[8][4][2] = { // for I tetromino
  { // 0->R
    {-2, 0},
    {1, 0},
    {-2, -1},
    {1, 2}
  },
  { // R->0
    {2, 0},
    {-1, 0},
    {2, 1},
    {-1, -2}
  },
  { // R->2
    {-1, 0},
    {2, 0},
    {-1, 2},
    {2, -1}
  },
  { // 2->R
    {1, 0},
    {-2, 0},
    {1, -2},
    {-2, 1}
  },
  { // 2->L
    {2, 0},
    {-1, 0},
    {2, 1},
    {-1, -2}
  },
  { // L->2
    {-2, 0},
    {1, 0},
    {-2, -1},
    {1, 2}
  },
  { // L->0
    {1, 0},
    {-2, 0},
    {1, -2},
    {-2, 1}
  },
  { // 0->L
    {-1, 0},
    {2, 0},
    {-1, 2},
    {2, -1}
  }
};

// wall kick constants, used to check for wall kicks
// see https://tetris.wiki/Super_Rotation_System#Wall_Kicks for more info

struct tetPiece {
  int x, y; // position of tetromino
};

struct tetBoard {
  char grid[wid][len];
  // alright, this will be more simple than snake.
  // the grid buffer goes from left to right, top to bottom.
  // the buffer will store the type of tetrominoes as integers.
  // see tetrominoes below.
  int next[nextLen];
  // next is the same, uses tetrominoes defined below.


  int order[8]; // order of tetrominoes, will be shuffled.
  int orderCount; // current place of order, array reshuffles when you go over the max.

  int cur; // current tetromino
  int dir; // direction of tetromino, range of 0-3
  int timeToDrop; // time until tetromino drops, in microseconds.
  int blocks[4][2]; // contains other parts of tetromino, ignoring center.
  int ghostOffset; // ghost offset
  int lD; // boolean, for lock delay.
  int hold; // hold tetromino
  int holdCooldown; // if hold needs to cool down

  int level; // which level you are currently on
  int lines; // progress towards your next level
  int totalLines; // total lines completed

  // these just count the types of line clears you got
  int single;
  int doubleCount;
  int triple;
  int tetris;

  struct tetPiece piece;

  WINDOW *gameWin; // the game window
  WINDOW *nextWin; // next tetrominoes
  WINDOW *holdWin; // shows hold
  WINDOW *statsWin; // shows stats, such as level and lines
  WINDOW *lineClearWin; // shows line clears 
};

struct tetBoard newBoard() {
  struct tetBoard out = {
    .order = {0, 1, 2, 3, 4, 5, 6, 7},
    .lD = 0,
    .level = 1,
  };
  return out;
}

int w, h; // width and height of terminal screen


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

int getTetromino(int d, int t, int blocks[4][2]) { // returns the other 3 tetromino blocks when given the center and type of tetromino, and the pointer to the block array.
  int x, y; // temporary variables

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

void updateNext(struct tetBoard *board) {
  // draw new next blocks

  werase(board->nextWin);

  wattron(board->nextWin, COLOR_PAIR(8));

  wprintw(board->nextWin, "-NEXT-");

  int nextBlock[4][2];

  for (int i = 0; i < nextLen; i++) { // draw next blocks
    getTetromino(0, board->next[i], nextBlock);
    wattron(board->nextWin, COLOR_PAIR(board->next[i]+1));
    for (int n = 0; n < 4; n++) {
      wmove(board->nextWin, 3 + (i*4) + nextBlock[n][1], 2 + nextBlock[n][0]);
      waddch(board->nextWin, tetrominoes[board->next[i]]);
    }
  }

  wrefresh(board->nextWin);
}

int popBag(struct tetBoard *board) {
  int pop = board->order[board->orderCount]; // pop tetromino from bag
  board->orderCount++; // increment "stack pointer"
  if (board->orderCount > 6) { // if reached end of bag
    board->orderCount = 0; // reset "stack pointer"
    shuffle(board->order, 7); // shuffle order
  }
  return pop;
}

int popNext(struct tetBoard *board) {
  int pop = board->next[0]; // pop tetromino from bag
  for (int i = 0; i < nextLen - 1; i++) {
    board->next[i] = board->next[i+1]; // move all tetrominoes up one
  }
  board->next[nextLen - 1] = popBag(board); // get another tetromino into next

  updateNext(board);

  return pop; // finally, return tetromino
}

void wprintCenter(WINDOW *win, char *s, int offset) { // prints text at center of screen
  int w, h;
  getmaxyx(win, h, w);
  wmove(win, (h / 2) + offset, (w - strlen(s)) / 2);
  wprintw(win, s);
}

void findGhost(struct tetBoard *board) {
  int x, y; // temporary variables

  board->ghostOffset = 0;
  int finished = 0; // variable used to check if a loop is finished or not

  while (true) { // find the y offset for ghost piece
    for (int i = 0; i < 4; i++) {
      x = board->blocks[i][0] + board->piece.x;
      y = board->blocks[i][1] + board->ghostOffset + board->piece.y;
      if (y == len || (board->grid[x][y] != 0 && x >= 0 && y >= 0)) {
        finished = 1;
        break;
      }
    }
    if (finished) {
      break;
    }
    board->ghostOffset++;
  }

  board->ghostOffset--;
}

void updateStats(struct tetBoard *board) {
  char buf[6];
  werase(board->statsWin);

  wmove(board->statsWin, 0, 2);
  wprintw(board->statsWin, "LEVEL");

  sprintf(buf, "%d", board->level);
  wmove(board->statsWin, 2, 7 - strlen(buf));
  wprintw(board->statsWin, buf);

  wmove(board->statsWin, 4, 2);
  wprintw(board->statsWin, "LINES");

  sprintf(buf, "%d/%d", board->lines, board->level * 5);
  wmove(board->statsWin, 6, 7 - strlen(buf));
  wprintw(board->statsWin, buf);

  wrefresh(board->statsWin);
}

void updateHold(struct tetBoard *board) {
  werase(board->holdWin);

  wmove(board->holdWin, 0, 0);

  wattron(board->holdWin, COLOR_PAIR(8));

  wprintw(board->holdWin, "-HOLD-");

  if (board->hold != 0) {
    int holdBlock[4][2];
    getTetromino(0, board->hold-1, holdBlock);
    wattron(board->holdWin, COLOR_PAIR(board->hold));
    for (int n = 0; n < 4; n++) {
      wmove(board->holdWin, 3 + holdBlock[n][1], 2 + holdBlock[n][0]);
      waddch(board->holdWin, tetrominoes[board->hold-1]);
    }
  } else {
    wmove(board->holdWin, 2, 1);
    wprintw(board->holdWin, "None");
  }

  wrefresh(board->holdWin);
}

int collision(struct tetBoard *board) { // check for collision
  getTetromino(board->dir, board->cur, board->blocks);
  int x, y; // temporary variables

  for (int i = 0; i < 4; i++) { // grid check
    x = board->blocks[i][0] + board->piece.x;
    y = board->blocks[i][1] + board->piece.y;

    if (x < 0) { // left check
      return 1;
    }

    if (x >= wid) { // right check
      return 1;
    }

    if (y >= len) { // bottom collision check
      return 1;
    }

    if (board->grid[x][y] != 0 && y > 0) {
      return 1;
    }
  }

  return 0;
}

void gameOver(struct tetBoard *board) {
  char buf[wid];
  werase(board->gameWin);
  wattron(board->gameWin, COLOR_PAIR(8));
  wborder(board->gameWin, '|', '|', '-', '-', '+', '+', '+', '+'); // ascii borders
  wprintCenter(board->gameWin, "-STATS-", -10);
  wprintCenter(board->gameWin, "Total", -8);
  wprintCenter(board->gameWin, "Lines", -7);
  sprintf(buf, "%d", board->totalLines);
  wprintCenter(board->gameWin, buf, -6);
  wprintCenter(board->gameWin, "Single", -4);
  sprintf(buf, "%d", board->single);
  wprintCenter(board->gameWin, buf, -3);
  wprintCenter(board->gameWin, "Double", -1);
  sprintf(buf, "%d", board->doubleCount);
  wprintCenter(board->gameWin, buf, 0);
  wprintCenter(board->gameWin, "Triple", 2);
  sprintf(buf, "%d", board->triple);
  wprintCenter(board->gameWin, buf, 3);
  wprintCenter(board->gameWin, "Tetris", 5);
  sprintf(buf, "%d", board->tetris);
  wprintCenter(board->gameWin, buf, 6);

  wattron(board->gameWin, A_STANDOUT);
  wprintCenter(board->gameWin, "EXIT", 8);

  wrefresh(board->gameWin);

  while (getch() != '\n'){}; // wait for enter to be pressed
  endwin(); // end window
  exit(0); // exit
}

void hdrop(struct tetBoard *board) { // hard drop function
  int x, y; // temporary variables

  board->holdCooldown = 0; // reset hold
  board->dir = 0;
  board->lD = 0;
  findGhost(board);

  for (int i = 0; i < 4; i++) {
    if (y < 1) {
      gameOver(board);
    }
    x = board->blocks[i][0] + board->piece.x;
    y = board->blocks[i][1] + board->piece.y + board->ghostOffset;
    board->grid[x][y] = board->cur + 1; // draw tetromino to grid
  }

  board->cur = popNext(board);

  board->piece.x = wid / 2 - 1; // reset x
  board->piece.y = 0; // and y position

  int linesCleared = 0; // number of lines cleared

  for (int y = 0; y < len; y++) { // loop to find cleared lines
    int success = 1;
    for (int x = 0; x < wid; x++) {
      if (board->grid[x][y] == 0) {
        success = 0;
        break;
      }
    }
    if (success) {
      for (int rY = y; rY > 0; rY--) { // loop to clear lines
        for (int rX = 0; rX < wid; rX++) {
          board->grid[rX][rY] = board->grid[rX][rY-1];
        }
      }
      linesCleared++;
    }
  }

  werase(board->lineClearWin);

  if (linesCleared > 0) {
    board->totalLines += linesCleared;

    switch (linesCleared) {
      case 1:
        board->single++;
        wprintCenter(board->lineClearWin, "Single", 0);
        break;
      case 2:
        board->doubleCount++;
        wprintCenter(board->lineClearWin, "Double", 0);
        break;
      case 3:
        board->triple++;
        wprintCenter(board->lineClearWin, "Triple", 0);
        break;
      case 4:
        board->tetris++;
        wprintCenter(board->lineClearWin, "Tetris", 0);
        break;
    }

    if (specialLineValues) {
      switch (linesCleared) {
        case 1:
          board->lines += 1;
          break;
        case 2:
          board->lines += 3;
          break;
        case 3:
          board->lines += 5;
          break;
        case 4:
          board->lines += 8;
          break;
      }
    } else {
      board->lines += linesCleared;
    }

    while (board->lines >= board->level * 5) {
      board->lines = board->lines - (board->level * 5);
      board->level++;
    }

    updateStats(board);
  }

  wrefresh(board->lineClearWin);
}

void sdrop(int a, struct tetBoard *board) { // soft drop function
  if (board->lD) {
    if (a) {
      hdrop(board);
    }
  } else {
    board->piece.y++;

    if (collision(board)) {
      board->piece.y--;
    }

    findGhost(board);

    if (board->ghostOffset == 0) {
      board->lD = 1;
      board->timeToDrop = lockDelay;
    }
  }
}

void resetLockDelay(struct tetBoard *board) {
  findGhost(board);

  if (board->ghostOffset > 0 && board->lD) {
    board->lD = 0;
    board->timeToDrop = dropTime / board->level;
  }
}

int lookupKickData(int cur, int dirTurn, int test, int xory) {
  // helper function for wallKick()
  // xory is short for x or y
  // returns x/y offset from wall kick lookup table
  if (tetrominoes[cur] == 'I') { // if rotating stick piece
    return stickKickData[dirTurn][test][xory]; // use special stick kick data
  } else {
    return wallKickData[dirTurn][test][xory]; // use normal wall kick data
  }
}

int wallKick(int input, struct tetBoard *board) { // wall kick check
  // checks to see if a wall kick is possible
  // if it is, it performs the wall kick
  // function returns 1 if the kick was successful, 0 if it was not

  int dirTurn = (board->dir * 2) + (input == rrotate); // satisfies all cases from 0->R to 0->L

  for (int i = 0; i < 4; i++) {
    board->piece.x += lookupKickData(board->cur, dirTurn, i, 0);
    board->piece.y += lookupKickData(board->cur, dirTurn, i, 1);

    if (collision(board)) {
      board->piece.x -= lookupKickData(board->cur, dirTurn, i, 0); // undo move
      board->piece.y -= lookupKickData(board->cur, dirTurn, i, 1);
    } else {
      return 1; // success!
    }
  }

  return 0; // wall kick not possible
}

void processKeys(struct tetBoard *board) {
  int input = getch();

  if (input == rotate1 || input == rotate2) { // rotate tetromino
    board->dir = (board->dir + 1) % 4;
    if (collision(board) && !wallKick(input, board)) { // if there's a collision and a wall kick isn't possible
      board->dir = (board->dir + 3) % 4; // undo rotation
    }
    resetLockDelay(board);
  } else if (input == rrotate) { // rotate tetromino in reverse
    board->dir = (board->dir + 3) % 4;
    if (collision(board) && !wallKick(input, board)) {
      board->dir = (board->dir + 1) % 4;
    }
    resetLockDelay(board);
  } else if (input == rotate180) { // rotate tetromino 180 degrees
    for (int i = 0; i < 2; i++) { // rotating tetromino 90 degrees twice is the same as rotating a tetromino 180 degrees
      board->dir = (board->dir + 1) % 4;
      if (collision(board) && !wallKick(input, board)) {
        board->dir = (board->dir + 3) % 4;
        break;
      }
    }
    resetLockDelay(board);
  } else if (input == left) { // go left
    board->piece.x--;
    if (collision(board)) {
      board->piece.x++;
    }
    resetLockDelay(board);
  } else if (input == right) { // go right
    board->piece.x++;
    if (collision(board)) {
      board->piece.x--;
    }
    resetLockDelay(board);
  } else if (input == softdrop) {
    sdrop(0, board);
  } else if (input == harddrop) {
    hdrop(board);
  } else if (input == holdKey) {
    if (!board->holdCooldown) {
      int swap; // temporary swap variable
      swap = board->hold;
      board->hold = board->cur + 1;

      board->piece.x = wid / 2 - 1; // reset x
      board->piece.y = 0; // and y position
      board->dir = 0; // and rotation
      board->lD = 0; // reset lock delay because we aren't touching ground anymore.

      if (swap == 0) {
        board->cur = popNext(board);
      } else {
        board->cur = swap - 1;
      }
      board->holdCooldown = 1;
      updateHold(board);
    }
  }

  if (board->timeToDrop <= 0) { // soft drop
    board->timeToDrop = dropTime / board->level;
    sdrop(1, board);
  } else {
    board->timeToDrop--;
  }
}

void drawGame(struct tetBoard *board) {
  werase(board->gameWin); // clear screen

  wattron(board->gameWin, COLOR_PAIR(8));
  wborder(board->gameWin, '|', '|', '-', '-', '+', '+', '+', '+'); // ascii borders

  if (board->orderCount >= 7) {
    board->orderCount = 0;
  }

  for (int x = 0; x < wid; x++) {
    for (int y = 0; y < len; y++) {
      if (board->grid[x][y] != 0) {
        wmove(board->gameWin, y + 1, x + 1);
        wattron(board->gameWin, COLOR_PAIR(board->grid[x][y]));
        waddch(board->gameWin, tetrominoes[board->grid[x][y]-1]);
      }
    }
  }

  getTetromino(board->dir, board->cur, board->blocks);

  findGhost(board);

  wattron(board->gameWin, COLOR_PAIR(9)); // ghost piece is gray

  if (board->ghostOffset > 0) {
    for (int i = 0; i < 4; i++) { // draw ghost piece
      wmove(board->gameWin, board->blocks[i][1] + board->piece.y + 1 + board->ghostOffset, board->blocks[i][0] + board->piece.x + 1);
      waddch(board->gameWin, tetrominoes[board->cur]);
    }
  }

  wattron(board->gameWin, COLOR_PAIR(board->cur+1));

  for (int i = 0; i < 4; i++) { // draw current tetris piece
    if (board->blocks[i][1] + board->piece.y >= 0 || i == 3) {
      wmove(board->gameWin, board->blocks[i][1] + board->piece.y + 1, board->blocks[i][0] + board->piece.x + 1);
      waddch(board->gameWin, tetrominoes[board->cur]);
    }
  }

  wmove(board->gameWin, 0, 0);

  wrefresh(board->gameWin);
}

void printCenter(char *s, int offset) { // prints text at center of screen
  move((h / 2) + offset, (w - strlen(s)) / 2);
  printw(s);
}

int main() {
  struct tetBoard tetris = newBoard();

  initscr(); // initialize ncurses

  getmaxyx(stdscr, h, w); // get width and height of screen

  if (w < wid + 13 || (h < len + 2 || h < nextLen * 4 + 2)) {
    printCenter("Screen too small!", -1);
    printCenter("Press any key to quit", 1);
    getch(); // wait for input
  } else {
    srand(time(0)); // init random

    nodelay(stdscr, TRUE); // don't want delay with getch input
    noecho(); // also, don't echo output
    keypad(stdscr, TRUE); // I'm also using arrow keys.
    start_color(); // initialize colors
    curs_set(0); // hides cursor

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

    tetris.gameWin = newwin(len + 2, wid + 2, (h-len) / 2, (w-wid) / 2);
    tetris.nextWin = newwin(nextLen * 4 + 4, 6, (h-len) / 2 - 1, (w+wid) / 2 + 3);
    tetris.holdWin = newwin(10, 6, (h-len) / 2, (w-wid) / 2 - 8);
    tetris.statsWin = newwin(8, 7, (h+len) / 2 - 6, (w-wid) / 2 - 8);
    tetris.lineClearWin = newwin(1, w, ((h-len) / 2) + len + 3, 1);

    tetris.timeToDrop = dropTime / tetris.level;
    shuffle(tetris.order, 7); // shuffle order

    for (int i = 0; i < nextLen; i++) { // fill next array
      tetris.next[i] = popBag(&tetris);
    }

    tetris.piece.x = wid / 2 - 1; // init x position

    getch(); // first run of getch, to make sure nothing is cleared for no reason.

    updateHold(&tetris);
    updateStats(&tetris);
    updateNext(&tetris);

    wattron(tetris.gameWin, COLOR_PAIR(8)); // default colors

    for (int i = secondsToStart; i > 0; i--) { // countdown
      char num[wid]; // for whatever reason, cannot have length of 1.
      sprintf(num, "%d", i); // format number
      werase(tetris.gameWin); // clear screen
      wborder(tetris.gameWin, '|', '|', '-', '-', '+', '+', '+', '+'); // ascii borders
      wprintCenter(tetris.gameWin, num, 0);
      wmove(tetris.gameWin, 0, 0);
      wrefresh(tetris.gameWin);
      sleep(1);
    }

    werase(tetris.gameWin); // clear screen
    wborder(tetris.gameWin, '|', '|', '-', '-', '+', '+', '+', '+'); // ascii borders
    wprintCenter(tetris.gameWin, "START!", 0);
    wmove(tetris.gameWin, 0, 0);
    wrefresh(tetris.gameWin);
    sleep(1);

    while (getch() != -1) {} // clear input

    tetris.cur = popNext(&tetris); // get current tetromino

    while (1) { // game loop
      processKeys(&tetris);

      drawGame(&tetris);

      usleep(1);
    }
  }

  endwin();
}
