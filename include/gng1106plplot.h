/*----------------------------------------------------
File: GNG1106plplot.h

Description:  Provides definitions to use the color
map 0 in PLplot and styles

To use these definitions, must change the colors in the
map as follows:
1) Set background color to white.
   Before calling plinit(), call
           plscolbg(255, 255, 255);    // White background color
2) Set default forground color to black.
   Before calling plinit(), call
           plscol0(1,0,0,0);
3) Change Wheat color to red (was default foreground color).
   Before calling plinit(), call
           plscol0(15,255,0,0);

Color scheme:
The arguments r, g, b define the amount of red, green and
blue added to create the color.  The value can range between
0 and 255.  Thus when r, g, b are set to 0, 0, 0, no color
is added and black is the result.  When r, g, b, are
set to 255, 255, 255, then an equal and maximum amount
of red, green and blue are added and the resulting color
is white.
-----------------------------------------------------*/
#include "plplot.h"

// The color map for creating maps on white background
#define WHITE       0 // Changed from PLplot defined color
#define BLACK       1 // Changed from PLplot defined color
#define YELLOW      2
#define GREEN       3
#define AQUAMARINE  4
#define PINK        5
#define WHEAT       6
#define GREY        7
#define BROWN       8
#define BLUE        9
#define BLUEVIOLET  10
#define CYAN        11
#define TURQUOISE   12
#define MAGENTA     13
#define SALMON      14
#define RED         15   // Changed from PLplot defined color

// Definitions for syles

#define SOLID               1
#define SHRTDASH_SHRTGAP    2
#define LNGDASH_LNGGAP      3
#define LNGDASH_SHRTGAP     4
#define LNGDASH_SHRTDASH    5
#define LSTYLE6             6
#define LSTYLE7             7
#define LSTYLE8             8
