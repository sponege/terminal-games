#include <stdio.h>
#include <curses.h>
#include <unistd.h> /* for sleep */
#include <stdlib.h>
#include <string.h>
#include <time.h>   /* for random */
#include "config.h" /* user configuration */
#include <png.h>
#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include <math.h>

static void
fatal_error(const char * message, ...) {
  va_list args;
  va_start(args, message);
  vfprintf(stderr, message, args);
  va_end(args);
  exit(EXIT_FAILURE);
}

int w, h; // width and height of terminal screen
char png_file[100]; // name of file
int *avg_colors = NULL; // gets the average of all colors to be used in terminal
int *avg_bw = NULL; // gets the average of all pixels to be used in terminal

int main() {
  initscr(); // initialize ncurses

  getmaxyx(stdscr, h, w); // get width and height of screen

  nodelay(stdscr, TRUE); // don't want delay with getch input
  noecho(); // also, don't echo output
  curs_set(0); // hides cursor

  endwin(); // call before exiting to restore terminal settings

  for (int i = 1; i < 1000; i++) {
    sprintf(png_file, "./frames/frame%d.png", i);

    png_structp png_ptr;
    png_infop info_ptr;
    FILE * fp;
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
    png_get_IHDR(png_ptr, info_ptr, & width, & height, & bit_depth, &
      color_type, & interlace_method, & compression_method, &
      filter_method);
    rows = png_get_rows(png_ptr, info_ptr);
    // printf("Width is %d, height is %d\n", width, height);
    int rowbytes;
    rowbytes = png_get_rowbytes(png_ptr, info_ptr);
    // printf("Row bytes = %d\n", rowbytes); // dont need to deal with this crap right now
    avg_colors = realloc(avg_colors, ((2*w)*(3*h))*3 * sizeof *avg_colors); // braille digits are two across and and three above, dont forget about rgb
    avg_bw = realloc(avg_colors, ((2*w)*(3*h)) * sizeof *avg_colors); // braille digits are two across and and three above

    // for (int j = 0; j < w*2; j++) {
    //   for (int k = 0; k < h*3; k++) {
    //     int avg_r = 0;
    //     int avg_g = 0;
    //     int avg_b = 0;
    //     for (int l = 0; l < (int)(height/(h*3)); l++) {
    //       png_bytep row;
    //       row = rows[l+(k*(int)(height/(h*3)))];
    //       for (int m = 0; m < (int)(width/(w*2)); m++) {
    //         png_byte pixel;
    //         pixel = row[m+(j*(int)(width/(w*2))*3)];
    //         avg_r += pixel;
    //         pixel = row[m+(j*(int)(width/(w*2))*3)+1];
    //         avg_g += pixel;
    //         pixel = row[m+(j*(int)(width/(w*2))*3)+2];
    //         avg_b += pixel;
    //       }
    //     }
    //     avg_r /= (int)(height/(h*3)) * (int)(width/(w*2));
    //     avg_g /= (int)(height/(h*3)) * (int)(width/(w*2));
    //     avg_b /= (int)(height/(h*3)) * (int)(width/(w*2));
    //     avg_colors[(3*j+(k*w))] = avg_r;
    //     avg_colors[(3*j+(k*w))+1] = avg_g;
    //     avg_colors[(3*j+(k*w))+2] = avg_b;
    //     int avg_col = (avg_r + avg_g + avg_b) / 3;
    //     avg_bw[j+(k*w)] = avg_b;
    //   }
    // }

    for (int j = 0; j < w; j++) {
      for (int k = 0; k < h; k++) {
        int avg_r = 0;
        int avg_g = 0;
        int avg_b = 0;
        for (int l = 0; l < (int)(height/(h)); l++) {
          png_bytep row;
          row = rows[l+(k*(int)(height/(h)))];
          for (int m = 0; m < (int)(width/(w)); m++) {
            png_byte pixel;
            pixel = row[(m+(j*(int)(width/(w)))*3)];
            avg_r += pixel;
            pixel = row[(m+(j*(int)(width/(w)))*3)+1];
            avg_g += pixel;
            pixel = row[(m+(j*(int)(width/(w)))*3)+2];
            avg_b += pixel;
          }
        }
        avg_r /= (int)(height/(h)) * (int)(width/(w));
        avg_g /= (int)(height/(h)) * (int)(width/(w));
        avg_b /= (int)(height/(h)) * (int)(width/(w));
        avg_colors[(3*j+(k*w))] = avg_r;
        avg_colors[(3*j+(k*w))+1] = avg_g;
        avg_colors[(3*j+(k*w))+2] = avg_b;
        int avg_col = (avg_r + avg_g + avg_b) / 3;
        avg_bw[j+(k*w)] = avg_b;
      }
    }

    printf("\e[1;1H\e[2J"); // clear screen

    for (int k = 0; k < h; k++) {
      for (int j = 0; j < w; j++) {
        // printf("%c", 'a'+((avg_bw[j+(k*w)] & 0b1) * 1));
        if (avg_bw[j+(k*w)] < 64) {
		printf ("#");
	    }
	    else if (avg_bw[j+(k*w)] < 128) {
		printf ("*");
	    }
	    else if (avg_bw[j+(k*w)] < 196) {
		printf (".");
	    }
	    else {
		printf (" ");
	    }
      }
      printf("\n");
    }
    usleep(1000);
    // for (j = 0; j < height; j++) {
    //   int i;
    //   png_bytep row;
    //   row = rows[j];
    //   for (i = 0; i < rowbytes; i++) {
    //     png_byte pixel;
    //     pixel = row[i];
    //     // if (pixel < 64) {
    //     //   printf("##");
    //     // } else if (pixel < 128) {
    //     //   printf("**");
    //     // } else if (pixel < 196) {
    //     //   printf("..");
    //     // } else {
    //     //   printf("  ");
    //     // }
    //
    //   }
    //   // printf("\n");
    // }
  }
  return 0;
}
