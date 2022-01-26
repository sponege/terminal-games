#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>   /* for random */
#include <unistd.h> /* for sleep */
#include "config.h" /* user configuration */
#include <errno.h>
#include <math.h>
#include <png.h>
#include <stdarg.h>
#include <stdio.h>

static void fatal_error(const char *message, ...) {
  va_list args;
  va_start(args, message);
  vfprintf(stderr, message, args);
  va_end(args);
  exit(EXIT_FAILURE);
}

int console_width, console_height; // width and height of terminal screen
char png_file[100];                // name of file
int *avg_colors = NULL; // gets the average of all colors to be used in terminal
int *avg_bw = NULL;     // gets the average of all pixels to be used in terminal

int main() {
  initscr(); // initialize ncurses

  getmaxyx(stdscr, console_height,
           console_width); // get width and height of screen

  nodelay(stdscr, TRUE); // don't want delay with getch input
  noecho();              // also, don't echo output
  curs_set(0);           // hides cursor

  endwin(); // call before exiting to restore terminal settings

  for (int i = 1; i < NUM_FRAMES; i++) {
    memset(png_file, 0, 100);
    sprintf(png_file, "./frames/frame%d.png", i);

    png_structp png_ptr;
    png_infop info_ptr;
    FILE *fp;
    png_uint_32 width;
    png_uint_32 height;
    int bit_depth;
    int color_type;
    int interlace_method;
    int compression_method;
    int filter_method;
    png_bytepp rows;

    fp = fopen(png_file, "rb");
    if (!fp) {
      fatal_error("Cannot open '%s': %s\n", png_file, strerror(errno));
    }
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
      fatal_error("Cannot create PNG read structure");
    }
    info_ptr = png_create_info_struct(png_ptr);
    if (!png_ptr) {
      fatal_error("Cannot create PNG info structure");
    }
    png_init_io(png_ptr, fp);
    png_read_png(png_ptr, info_ptr, 0, 0);
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
                 &interlace_method, &compression_method, &filter_method);
    rows = png_get_rows(png_ptr, info_ptr);

    avg_colors = realloc(avg_colors, width * height * 3 * sizeof(*avg_colors));
    avg_bw = realloc(avg_colors, width * height * sizeof(*avg_colors));

    fclose(fp);

    int y_ppc = (int)(height / console_height); // Pixels per character
    int x_ppc = (int)(width / console_width);   // Pixels per character

    for (int char_x = 0; char_x < console_width; char_x++) {
      for (int char_y = 0; char_y < console_height; char_y++) {

        int avg_r = 0;
        int avg_g = 0;
        int avg_b = 0;

        for (int y_off = 0; y_off < y_ppc; y_off++) {

          png_bytep row;
          row = rows[y_off + (char_y * y_ppc)];

          for (int x_off = 0; x_off < x_ppc; x_off++) {

            png_byte pixel;
            pixel = row[(x_off + char_x * x_ppc) * 3];
            avg_r += pixel;
            pixel = row[(x_off + char_x * x_ppc) * 3 + 1];
            avg_g += pixel;
            pixel = row[(x_off + char_x * x_ppc) * 3 + 2];
            avg_b += pixel;
          }
        }

        int total_ppc = y_ppc * x_ppc;

        avg_r /= total_ppc;
        avg_g /= total_ppc;
        avg_b /= total_ppc;

        avg_colors[(char_x + (char_y * console_width) * 3)] = avg_r;
        avg_colors[(char_x + (char_y * console_width) * 3) + 1] = avg_g;
        avg_colors[(char_x + (char_y * console_width) * 3) + 2] = avg_b;
        int avg_col = (avg_r + avg_g + avg_b) / 3;
        avg_bw[char_x + (char_y * console_width)] = avg_col;
      }
    }

    printf("\e[1;1H\e[2J"); // clear screen

    for (int y = 0; y < console_height; y++) {
      for (int x = 0; x < console_width; x++) {
        // printf("%c", 'a'+((avg_bw[x+(y*console_width)])/3));
        if (avg_bw[x + (y * console_width)] < 64) {
          printf(" ");
        } else if (avg_bw[x + (y * console_width)] < 128) {
          printf(".");
        } else if (avg_bw[x + (y * console_width)] < 196) {
          printf("*");
        } else {
          printf("#");
        }
      }
      printf("\n");
    }
    usleep(32000);
  }
  return 0;
}
