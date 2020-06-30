#include <stdio.h>
#include <curses.h>
#include <unistd.h> /* for sleep */
#include <stdlib.h>
#include <string.h>
#include <time.h>   /* for random */

// Made by Jordan Perry
// 15/06/2020

// -- How To Run --
// Install ncurses:
// Debian: sudo apt-get install libncurses5-dev libncursesw5-dev
// Red Hat: sudo yum install ncurses-devel ncurses
// Compile binary:
// gcc snake.c -o snake -lncurses

// You can edit the configuration below. Be sure to compile again after making your changes.

#define sleepTime 50000 // adjust this to a higher value to make game slower, adjust this to a smaller value to make game faster
#define numCells 10 // this is the number of cells you start out with
#define cellAdd 10 // this is the number of cells you get when you eat food

int *ptr = NULL; // array that can grow and shrink
int len = 0; // length of array
int dir = 0; // starts at right, going clockwise
int score = 0; // your score
int w, h; // width and height of terminal screen
int fX, fY; // food position
int alive = -1;
int cellCount = 0;

int randrange(int lower, int upper) {
    // copied and modified from https://www.geeksforgeeks.org/generating-random-number-range-c/
		return (rand() %
		(upper - lower + 1)) + lower;
}

void addCell() {
  len += 2;
  ptr = realloc( ptr, len * sizeof *ptr );
  ptr[len-1] = ptr[len-3]; // copy last cell
  ptr[len-2] = ptr[len-4]; // to new cell
}

void moveFood() {
  fX = randrange(0, w - 5) + 2;
  fY = randrange(0, h - 5) + 2;
}

void changeDir() {
  int oldDir = dir;
  int input = getch();
  if (alive == 1) {
    if (input == 'w' || input == KEY_UP) {
      dir = 3;
    } else if (input == 'a' || input == KEY_LEFT) {
      dir = 2;
    } else if (input == 's' || input == KEY_DOWN) {
      dir = 1;
    } else if (input == 'd' || input == KEY_RIGHT) {
      dir = 0;
    }
    if (dir + 2 == oldDir || dir - 2 == oldDir) { // you aren't allowed to go backwards
      dir = (dir + 2) % 4;
    }
  } else if (input != -1 && alive == 0) {
    if (input == KEY_HOME) {
      endwin(); // call before exiting to restore terminal settings
      exit(0);
    } else {
      alive = -1;
    }
  }
}

void moveSnake() {
  for (int i = 0; i < (len / 2) - 1; i++) { // shift all cells
    ptr[len-(i*2)-1] = ptr[len-(i*2)-2-1];
    ptr[len-(i*2)-1-1] = ptr[len-(i*2)-3-1];
  }
  switch (dir) { // move head
    case 0: // right
      ptr[0]++; // x increment
      break;
    case 1: // down
      ptr[1]++; // y increment
      break;
    case 2: // left
      ptr[0]--; // x decrement
      break;
    case 3: // up
      ptr[1]--; // y decrement
  }
}

void checkForDeath() {
  if (ptr[0] < 2 || ptr[0] > w - 3 || ptr[1] < 2 || ptr[1] > h - 3) { // if head touches wall
    alive = 0; // you dead bro
  } else {
    for (int i = 0; i < (len / 2) - 1; i++) {
      if (ptr[0] == ptr[len-(i*2)-2] && ptr[1] == ptr[len-(i*2)-1]) { // if head is touching body
        alive = 0; // you dead bro
      }
    }
  }
  if (!alive) {
    puts("\a"); // play bell on death
  }
}

void printCenter(char *s, int offset) { // prints text at center of screen
  move((h / 2) + offset, (w - strlen(s)) / 2);
  printw(s);
}

void drawGame() {
  // draw the game

  erase(); // clear screen
  getmaxyx(stdscr, h, w); // get width and height

  move(1, 1); // top left
  addch('+');

  move(1, w - 2); // top right
  addch('+');

  move(h - 2, 1); // bottom left
  addch('+');

  move(h - 2, w - 2); // bottom right
  addch('+');


  for (int count = 0; count < w - 4; count++) { // vertical rows
    move(1, count + 2);
    addch('-');
    move(h - 2, count + 2);
    addch('-');
  }

  for (int count = 0; count < h - 4; count++) { // horizontal columns
    move(count + 2, 1);
    addch('|');
    move(count + 2, w - 2);
    addch('|');
  }

  for (int i = 0; i < (len / 2); i++) { // draw the snake
    move(ptr[(i*2)+1], ptr[(i*2)]);
    addch('@'); // default
    // addch('A'+(i%26));  // alphabet snake, I like it :)
    // addch('!'+i);  // ASCII snake
  }

  if (!alive) { // if dead
    // print the death screen
    move(ptr[1], ptr[0]);
    addch('X');
    printCenter("--------GAME OVER-------", -2);
    printCenter("Press Any Key To Restart", 0);
    printCenter("Press [Home] To Quit", 2);
  }

  move(h - 1, 1);
  printw("Score: %d", score);

  move(fY, fX); // cursor will be food :)
  refresh();
}

int main() {
  initscr(); // initialize ncurses

  nodelay(stdscr, TRUE); // don't want delay with getch input
  noecho(); // also, don't echo output
  keypad(stdscr, TRUE); // I also want to use arrow keys for controlling the snake

  srand(time(0)); // init random

  while (1) {

    //puts("\033[34mhuh cool \033[35mnice\033[37m");

    if (alive == 1) { // loop until exit

      moveSnake();

      checkForDeath();

      if (len < numCells * 2) {
        addCell();
      }

      if (ptr[0] == fX && ptr[1] == fY) { // if touching food
        score++; // increment score
        moveFood(); // move food
        cellCount = cellAdd;
      }

      if (cellCount > 0) {
        addCell();
        cellCount--;
      }
    } else if (alive == -1) {
      score = 0;
      dir = 0;

      len = 2;
      ptr = realloc( ptr, len * sizeof *ptr ); // add head
      // alright, realloc allows us to have essentially infinite memory,
      // which is perfect for my snake game. remember to increment len
      // as well when resizing your array. we are starting off with an
      // array size of 2, because we start with a head. we are working
      // with x y coordinates in the array.

      getmaxyx(stdscr, h, w); // get width and height
      ptr[0] = w / 2; // x
      ptr[1] = h / 2; // y

      moveFood(); // initialize food position

      alive = 1;
    }

    changeDir();
    drawGame();
    usleep(sleepTime);
  }
}
