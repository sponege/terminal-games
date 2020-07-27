// You can edit the configuration below. Be sure to compile again after making your changes.
// The default configuration is from the Tetris Guidelines.
// https://tetris.fandom.com/wiki/Tetris_Guideline

// Keyboard configuration
// Make sure that letter keys are in lowercase

#define rotate1 KEY_UP
#define rotate2 'x'
#define rrotate 'z' // Rotate counterclockwise. Cannot detect control, sorry!
#define rotate180 'a'
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
