/*
  Raycasting Fenster port based on SDL example
  Original author: Miloslav Ciz
  Adapted for Fenster
*/

#include "fenster.h"
#include <stdio.h>
#include <time.h>
#include <string.h>

#define RCL_COMPUTE_FLOOR_TEXCOORDS 1
#define RCL_VERTICAL_FOV 380
#define RCL_TEXTURE_VERTICAL_STRETCH 1
#define RCL_PIXEL_FUNCTION pixelFunc 

#include "raycastlib.h"

#define LEVEL_X_RES 29
#define LEVEL_Y_RES 21

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define MIDDLE_ROW (SCREEN_HEIGHT / 2)

#define max(a,b) ((a) > (b) ? (a) : (b))

#define KEYS 8
#define KEY_UP 17     // Fenster uses 17-20 for arrow keys
#define KEY_RIGHT 19
#define KEY_DOWN 18
#define KEY_LEFT 20
#define KEY_Q 81
#define KEY_W 87
#define KEY_A 65
#define KEY_S 83

unsigned long frame = 0;

RCL_Camera camera;
uint32_t pixels[SCREEN_WIDTH * SCREEN_HEIGHT];
uint32_t pixelCounter[SCREEN_WIDTH * SCREEN_HEIGHT];
uint32_t palette[256];

typedef struct
{
  unsigned char *mImage;
  RCL_Vector2D mPosition;
  RCL_Unit mHeight;
  RCL_Unit mPixelSize;
} Sprite;

#define SPRITES 7
#define SPRITE_MAX_DISTANCE 5 * RCL_UNITS_PER_SQUARE

// For each level square says the texture index.
const unsigned char levelTexture[] =
{
// 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28
   1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 3, 0, 3, 3, 2, 2, 2, 2, // 0   20
   1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 1, 1, 1, 2, // 1   19
   1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 2, // 2   18
   1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 1, 1, 1, 2, // 3   17
   1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 2, // 4   16
   1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 1, 1, 1, 2, // 5   15
   1, 0, 1, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, // 6   14
   1, 0, 1, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 2, // 7   13
   1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, // 8   12
   1, 1, 1, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, // 9   11
   1, 1, 1, 1, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, // 10  10
   1, 1, 1, 1, 0, 0, 3, 3, 3, 3, 0, 0, 0, 0, 0, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, // 11  9
   1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, // 12  8
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 3, 3, 3, // 13  7
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, // 14  6
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 3, 3, 3, // 15  5
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 3, 3, 3, // 16  4
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 3, 3, 3, // 17  3
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 3, 3, 3, // 18  2
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 3, 3, 3, // 19  1
   1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3  // 20  0
};

// For each level square says the floor height.
const signed char levelFloor[] =
{
// 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28
  40,40,40,40,40,40,40,40,48,40,48,40,40,40,40,40,40,48,40,48,40,48,40,48,48,24,24,26,28, // 0  20
  40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,40, 2, 2, 2,40,32,32,32,32,32,32,32,48, 2, 2, 2,26, // 1  19
  40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,40, 2, 2, 2,40, 0, 0, 0, 0, 0,32,32,40, 2, 2, 2,26, // 2  18
  40,16,12, 8, 4, 0,48, 0, 0, 0, 0, 0,24, 2,24, 8,24, 0, 0, 9, 9, 0,28,32,48, 2, 2, 2,24, // 3  17
  40,20,48,48,48,48,48, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 9, 0,24,32,40, 0, 0, 0,24, // 4  16
  40,24,48,40,40,40,40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,20,32,48, 0, 0, 0,24, // 5  15
  40,28,32,32,32,32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 8,12,16,32,40, 0, 0, 0,24, // 6  14
  40,32,32,32,32,32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,32,32,32,32,32,32,48, 0, 0, 0,24, // 7  13
  40, 0,48,40,40,40,40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,32,-3,-8,-8,-5,-2, 0, 0, 0, 0,24, // 8  12
  40, 0,-3,-8,-8,-8,32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,32,-3,-8,-8, 0, 0, 0, 0, 0, 0,24, // 9  11
  40, 0,-6,-8,-8,-8,32, 0, 0, 0,48,48, 0,48,48, 0, 0,36,32,36,-8,-8, 0, 0, 0, 0, 0, 0,24, // 10 10
  40, 0,48,-8,-8,-8,32,32,32,32,40, 1, 0, 1,40,32,32,32,32,32,-8,-8, 0, 0, 0, 0, 0, 0,24, // 11 9
  40, 0,48,-8,-8,-8,-8,-8,-8,-8,-8, 0, 0, 0,-8,-8,-8,36,32,36,-8,-8, 0, 0, 0, 0, 0, 0,24, // 12 8
  40, 0,48,-8,-8,-8,-8,-8,-8,-8,-8, 0, 0, 0,-8,-8,-8,-8,-8,-8,-8,-8, 0, 0, 0, 0, 0, 0,24, // 13 7
  40, 0,48, 0,-2,-2, 0,-8,-8,-8,-8, 0, 0, 0,-8,-8, 0, 0, 0, 0, 0, 0, 0, 0,24,24,10,24,24, // 14 6
  40, 0, 0, 0,-2,-2, 0, 0, 0, 0, 8, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0,24, 0, 0, 0,24, // 15 5
  40, 0, 0, 0,-2,-2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,24, 0, 0, 0,24, // 16 4
  40,24,48,-2,-2,-2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,24, // 17 3
   0,24,48,48,-2,-2,-2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,24, // 18 2
   0,24,48,48,48,-2,-2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,24, 0, 0, 0,24, // 19 1
   0,24,24,24,32,32,32,27,24,27,30,34,36,38,36,34,36,34,34,32,32,33,37,39,24,24,24,24,24  // 20 0
};

#define XX 127 // helper to keep a big number two-characters, for formatting

// For each level square says the ceiling height.
const signed char levelCeiling[] =
{
// 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28
  40,40,40,40,40,40,40,XX,XX,XX,XX,36,40,40,40,40,40,36,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX, // 0  20
  40,50,50,50,45,40,20,XX,XX,XX,XX,36,40,30,30,30,XX,36,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX, // 1  19
  40,50,50,50,45,40,20,XX,XX,XX,XX,36,40,30,30,30,XX,36,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX, // 2  18
  40,50,50,50,45,40,48,XX,XX,XX,XX,36,24,24,24,24,24,36,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX, // 3  17
  40,50,48,48,48,48,47,XX,XX,XX,XX,36,36,36,36,36,36,36,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX, // 4  16
  40,50,48,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX, // 5  15
  40,50,48,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX, // 6  14
  40,50,48,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX, // 7  13
  40,50,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX, // 8  12
  40,40,16,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX, // 9  11
  40,30,16,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX, // 10 10
  40,25,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX, // 11 9
  40,20,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX, // 12 8
  40,18,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,24,24,24,24,24,24, // 13 7
  40,18,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,24,24,24,24,24,24, // 14 6
  40,18,16,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,24,24,25,27,29,24, // 15 5
  40,18,16,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,24,24,25,27,29,24, // 16 4
  40,24,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,24,24,25,27,29,24, // 17 3
  XX,24,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,24,24,25,27,29,24, // 18 2
  XX,24,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,24,24,25,27,29,24, // 19 1
  XX,24,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,XX,24,24,24,24,24,24  // 20 0
};

#undef XX

const unsigned char texture1[] =
{  32, 32 // width, height
  ,0x12,0x65,0x65,0x6d,0x24,0x64,0x5b,0x12,0x12,0x64,0x5b,0x5b,0x12,0x51
  ,0x12,0x09,0x00,0x6d,0x6d,0x6d,0x24,0x5b,0x5b,0x52,0x12,0x5b,0x52,0x5b
  ,0x12,0x5b,0x52,0x09,0x12,0x5b,0x5b,0x5b,0x52,0x5b,0x5b,0x52,0x00,0x64
  ,0x64,0x52,0x52,0x52,0x5b,0x00,0x09,0x6d,0x5b,0x5a,0x5b,0x5b,0x5a,0x51
  ,0x12,0x64,0x5b,0x52,0x5b,0x52,0x5b,0x08,0x12,0x24,0x5b,0x5b,0x5b,0x5b
  ,0x09,0x11,0x00,0x64,0x5b,0x5b,0x5a,0x09,0x09,0x08,0x00,0x5b,0x5b,0x5b
  ,0x5a,0x64,0x12,0x51,0x00,0x64,0x5b,0x52,0x52,0x11,0x09,0x09,0x12,0x24
  ,0x5b,0x5b,0x52,0x5a,0x12,0x09,0x00,0x5b,0x52,0x09,0x12,0x09,0x09,0x09
  ,0x00,0x24,0x5a,0x5a,0x5b,0x51,0x12,0x08,0x09,0x25,0x5b,0x52,0x5b,0x52
  ,0x11,0x09,0x12,0x64,0x5b,0x52,0x09,0x11,0x12,0x09,0x00,0x12,0x00,0x12
  ,0x00,0x00,0x00,0x08,0x09,0x64,0x5b,0x51,0x12,0x11,0x12,0x08,0x09,0x64
  ,0x5a,0x5b,0x12,0x12,0x09,0x09,0x12,0x65,0x5c,0x52,0x5a,0x5b,0x51,0x09
  ,0x00,0x64,0x6d,0x6d,0x5c,0x65,0x64,0x09,0x00,0x24,0x64,0x5b,0x09,0x12
  ,0x11,0x00,0x09,0x64,0x5b,0x52,0x52,0x12,0x12,0x08,0x00,0x64,0x52,0x52
  ,0x52,0x52,0x5b,0x09,0x00,0x6d,0x5b,0x5b,0x52,0x5b,0x51,0x09,0x00,0x1b
  ,0x5b,0x12,0x12,0x12,0x12,0x09,0x12,0x5b,0x11,0x09,0x11,0x09,0x09,0x09
  ,0x09,0x5b,0x09,0x09,0x09,0x09,0x09,0x09,0x12,0x65,0x5b,0x5b,0x5b,0x52
  ,0x5b,0x09,0x00,0x64,0x12,0x63,0x5b,0x12,0x5b,0x09,0x00,0x00,0x00,0x00
  ,0x00,0x09,0x00,0x00,0x00,0x09,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x5b
  ,0x64,0x5b,0x09,0x5b,0x52,0x09,0x09,0x5b,0x11,0x52,0x09,0x12,0x09,0x08
  ,0x00,0x1b,0x1b,0x23,0x1b,0x5a,0x5b,0x11,0x12,0x64,0x64,0x6d,0x24,0x5b
  ,0x5b,0x09,0x00,0x5b,0x5a,0x52,0x52,0x09,0x08,0x09,0x12,0x5b,0x52,0x11
  ,0x5a,0x11,0x12,0x08,0x08,0x1b,0x52,0x52,0x5b,0x5a,0x09,0x09,0x12,0x64
  ,0x5b,0x64,0x12,0x12,0x12,0x08,0x00,0x00,0x00,0x12,0x09,0x00,0x09,0x00
  ,0x12,0x24,0x5a,0x52,0x12,0x12,0x12,0x08,0x12,0x1b,0x12,0x5a,0x49,0x5a
  ,0x52,0x09,0x12,0x6d,0x5b,0x52,0x5a,0x12,0x09,0x09,0x12,0x65,0x64,0x6d
  ,0x6d,0x64,0x64,0x5b,0x00,0x5a,0x08,0x08,0x09,0x09,0x08,0x09,0x00,0x5b
  ,0x5a,0x11,0x51,0x51,0x5a,0x09,0x12,0x6d,0x5b,0x5b,0x12,0x11,0x12,0x09
  ,0x00,0x65,0x5b,0x64,0x5b,0x5b,0x5b,0x52,0x00,0x00,0x12,0x12,0x12,0x09
  ,0x00,0x00,0x00,0x5a,0x52,0x52,0x09,0x52,0x09,0x09,0x12,0x63,0x5b,0x5a
  ,0x09,0x5b,0x11,0x08,0x00,0x65,0x5b,0x5b,0x52,0x5b,0x52,0x52,0x00,0x65
  ,0x6d,0x64,0x65,0x64,0x64,0x5b,0x00,0x5b,0x12,0x09,0x11,0x09,0x09,0x09
  ,0x09,0x5b,0x11,0x08,0x09,0x09,0x09,0x08,0x00,0x64,0x64,0x5b,0x5b,0x5b
  ,0x5b,0x09,0x00,0x5b,0x5b,0x5b,0x5b,0x5b,0x52,0x52,0x00,0x64,0x51,0x52
  ,0x52,0x52,0x09,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x12,0x00,0x00,0x5b
  ,0x5b,0x5b,0x52,0x52,0x5b,0x09,0x12,0x65,0x5b,0x5b,0x5b,0x52,0x5b,0x09
  ,0x00,0x63,0x5a,0x51,0x51,0x51,0x5a,0x08,0x12,0x24,0x64,0x65,0x24,0x64
  ,0x64,0x5b,0x00,0x5b,0x5b,0x52,0x5b,0x5b,0x52,0x09,0x00,0x64,0x5b,0x5b
  ,0x12,0x52,0x5a,0x09,0x12,0x24,0x52,0x52,0x09,0x52,0x52,0x08,0x12,0x64
  ,0x64,0x5b,0x5b,0x5b,0x64,0x52,0x00,0x6d,0x5b,0x52,0x5b,0x52,0x52,0x08
  ,0x00,0x5b,0x5b,0x52,0x52,0x5b,0x52,0x09,0x00,0x1b,0x52,0x51,0x5a,0x52
  ,0x09,0x09,0x12,0x64,0x52,0x5c,0x5b,0x64,0x5b,0x51,0x09,0x6d,0x5b,0x52
  ,0x5b,0x52,0x11,0x09,0x00,0x64,0x5a,0x64,0x5b,0x52,0x5b,0x08,0x00,0x11
  ,0x09,0x49,0x08,0x09,0x00,0x09,0x00,0x64,0x64,0x5b,0x52,0x5b,0x5b,0x09
  ,0x12,0x65,0x64,0x5b,0x52,0x5b,0x52,0x09,0x12,0x64,0x64,0x64,0x52,0x09
  ,0x5c,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x12,0x64,0x5b,0x64
  ,0x52,0x5b,0x52,0x49,0x00,0x64,0x5b,0x52,0x52,0x5b,0x51,0x09,0x09,0x5b
  ,0x09,0x09,0x09,0x08,0x09,0x12,0x00,0x6d,0x24,0x6d,0x24,0x64,0x64,0x5b
  ,0x00,0x64,0x63,0x5b,0x5b,0x52,0x52,0x09,0x12,0x64,0x5b,0x5b,0x52,0x11
  ,0x5b,0x09,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x09,0x12,0x6d,0x64,0x5b
  ,0x52,0x64,0x5b,0x52,0x12,0x6d,0x5b,0x5b,0x5b,0x5b,0x5b,0x09,0x00,0x64
  ,0x11,0x5a,0x52,0x5b,0x52,0x09,0x00,0x5b,0x5a,0x63,0x5a,0x23,0x12,0x00
  ,0x12,0x6d,0x5b,0x11,0x5b,0x64,0x64,0x52,0x12,0x63,0x5b,0x5b,0x5b,0x5b
  ,0x52,0x08,0x00,0x5c,0x52,0x5b,0x5b,0x52,0x52,0x08,0x08,0x5b,0x52,0x51
  ,0x09,0x52,0x5a,0x09,0x00,0x6d,0x52,0x11,0x52,0x5b,0x52,0x09,0x12,0x64
  ,0x5a,0x5b,0x52,0x5b,0x5b,0x08,0x00,0x5b,0x52,0x09,0x09,0x09,0x09,0x09
  ,0x09,0x24,0x52,0x09,0x12,0x09,0x52,0x08,0x00,0x5b,0x5a,0x52,0x12,0x5b
  ,0x5b,0x09,0x00,0x65,0x52,0x5b,0x5b,0x52,0x11,0x09,0x09,0x00,0x00,0x00
  ,0x00,0x12,0x00,0x08,0x00,0x5b,0x52,0x51,0x09,0x09,0x09,0x09,0x00,0x64
  ,0x5b,0x52,0x52,0x09,0x5b,0x09,0x12,0x6d,0x5b,0x5b,0x5b,0x5b,0x5a,0x11
  ,0x00,0x6d,0x6d,0x24,0x64,0x64,0x5b,0x5b,0x00,0x5a,0x12,0x52,0x12,0x09
  ,0x12,0x08,0x00,0x5b,0x12,0x09,0x09,0x09,0x08,0x09,0x00,0x64,0x5b,0x5b
  ,0x52,0x5a,0x5b,0x09,0x12,0x6d,0x64,0x5a,0x5b,0x5b,0x5b,0x5b,0x00,0x5a
  ,0x11,0x52,0x09,0x12,0x09,0x09,0x09,0x00,0x00,0x00,0x00,0x12,0x12,0x12
  ,0x12,0x64,0x52,0x5b,0x5b,0x5b,0x5b,0x09,0x12,0x6d,0x12,0x5b,0x12,0x52
  ,0x5b,0x52,0x09,0x5a,0x09,0x51,0x09,0x09,0x09,0x09,0x09,0x5c,0x64,0x6d
  ,0x1b,0x24,0x64,0x08,0x12,0x64,0x5b,0x52,0x5b,0x5b,0x52,0x09,0x00,0x5b
  ,0x5b,0x52,0x11,0x5b,0x12,0x09,0x08,0x5b,0x5a,0x09,0x51,0x52,0x12,0x08
  ,0x00,0x5b,0x5b,0x11,0x5b,0x5b,0x5b,0x08,0x09,0x5b,0x12,0x52,0x52,0x09
  ,0x09,0x09,0x00,0x6d,0x5b,0x52,0x52,0x12,0x12,0x09,0x08,0x11,0x00,0x49
  ,0x09,0x00,0x09,0x00,0x00,0x6d,0x12,0x5b,0x52,0x5b,0x52,0x09,0x00,0x00
  ,0x09,0x09,0x12,0x00,0x12,0x00,0x00,0x64,0x52,0x5b,0x52,0x5b,0x52,0x09
  ,0x09,0x00,0x00,0x00,0x00,0x09,0x00,0x00,0x00,0x64,0x52,0x5b,0x5b,0x52
  ,0x52,0x09
};

const unsigned char texture2[] =
{  32, 32 // width, height
  ,0x65,0x1d,0x01,0x1c,0x12,0x0a,0x01,0x13,0x14,0x6e,0x2e,0x0b,0x1c,0x1d
  ,0x2e,0x1d,0x13,0x0b,0x13,0x65,0x1d,0x1d,0x13,0x66,0x6f,0x6e,0x65,0x6f
  ,0x77,0x7f,0x6f,0x6f,0x1d,0x14,0x13,0x6e,0x6e,0x66,0x6f,0x77,0x77,0x6f
  ,0x66,0x66,0x13,0x25,0x0a,0x01,0x6e,0x66,0x01,0x12,0x02,0x01,0x1b,0x6e
  ,0x77,0x77,0x77,0x77,0x77,0x6f,0x6f,0x6e,0x1c,0x1d,0x1c,0x77,0x67,0x6f
  ,0x6f,0x77,0x67,0x77,0x77,0x66,0x13,0x0a,0x14,0x77,0x66,0x6f,0x77,0x66
  ,0x1b,0x13,0x66,0x77,0x6f,0x77,0x6f,0x6f,0x6f,0x26,0x6e,0x1d,0x25,0x14
  ,0x13,0x6e,0x6f,0x6f,0x77,0x6f,0x77,0x77,0x77,0x1d,0x1d,0x01,0x25,0x67
  ,0x7f,0x6f,0x6f,0x66,0x66,0x14,0x66,0x6f,0x7f,0x6f,0x77,0x6f,0x1d,0x25
  ,0x1d,0x66,0x1d,0x65,0x14,0x2e,0x26,0x77,0x6d,0x67,0x6f,0x25,0x6f,0x5e
  ,0x5d,0x0b,0x5d,0x6f,0x6f,0x26,0x1d,0x77,0x66,0x1c,0x13,0x6f,0x6f,0x6f
  ,0x66,0x67,0x66,0x65,0x6e,0x1c,0x1b,0x0a,0x01,0x1d,0x65,0x1e,0x6f,0x6e
  ,0x67,0x66,0x66,0x65,0x14,0x1c,0x25,0x65,0x6e,0x66,0x66,0x6e,0x66,0x1c
  ,0x02,0x1d,0x77,0x6e,0x77,0x66,0x25,0x13,0x5c,0x1d,0x0a,0x14,0x0b,0x14
  ,0x6e,0x65,0x1d,0x66,0x1d,0x66,0x1d,0x66,0x14,0x25,0x14,0x6e,0x66,0x25
  ,0x66,0x5d,0x66,0x25,0x0a,0x6e,0x6f,0x6f,0x6e,0x1d,0x13,0x65,0x5c,0x01
  ,0x0a,0x15,0x1d,0x0a,0x01,0x5d,0x6e,0x5c,0x25,0x1d,0x1e,0x26,0x1d,0x1b
  ,0x14,0x25,0x25,0x1d,0x14,0x1d,0x14,0x13,0x01,0x25,0x6e,0x25,0x25,0x25
  ,0x1d,0x1c,0x01,0x0a,0x14,0x14,0x5d,0x14,0x14,0x0a,0x1d,0x5d,0x1c,0x6e
  ,0x1d,0x65,0x5d,0x1d,0x0a,0x01,0x14,0x1c,0x25,0x0a,0x01,0x12,0x1c,0x0b
  ,0x01,0x1c,0x14,0x25,0x0b,0x1b,0x25,0x14,0x25,0x02,0x1d,0x25,0x1d,0x09
  ,0x13,0x1c,0x1d,0x66,0x1d,0x1c,0x0b,0x14,0x0b,0x65,0x2d,0x09,0x0a,0x09
  ,0x14,0x66,0x6f,0x66,0x66,0x1c,0x0a,0x01,0x0b,0x14,0x66,0x1c,0x1d,0x1c
  ,0x66,0x66,0x65,0x6e,0x1c,0x09,0x1c,0x1d,0x14,0x1c,0x0b,0x01,0x1d,0x6e
  ,0x77,0x66,0x1c,0x1c,0x6e,0x6f,0x6f,0x77,0x7f,0x77,0x5e,0x6e,0x6f,0x67
  ,0x6f,0x6f,0x0a,0x0a,0x66,0x67,0x66,0x25,0x13,0x1c,0x24,0x14,0x25,0x1c
  ,0x0b,0x1d,0x6e,0x77,0x7f,0x77,0x5e,0x1c,0x65,0x6f,0x7f,0x6e,0x6f,0x77
  ,0x6f,0x67,0x6f,0x6f,0x66,0x25,0x14,0x6f,0x6f,0x77,0x6f,0x67,0x6e,0x13
  ,0x0a,0x09,0x13,0x12,0x1d,0x66,0x6f,0x66,0x5e,0x6e,0x66,0x25,0x14,0x25
  ,0x26,0x25,0x6f,0x1d,0x77,0x6f,0x66,0x6e,0x66,0x1c,0x12,0x6f,0x6f,0x67
  ,0x77,0x1d,0x77,0x13,0x66,0x77,0x6f,0x6f,0x6f,0x77,0x6f,0x66,0x77,0x77
  ,0x6f,0x6e,0x13,0x25,0x26,0x25,0x66,0x65,0x66,0x66,0x6e,0x66,0x6e,0x14
  ,0x09,0x66,0x6f,0x6e,0x2f,0x6e,0x26,0x1c,0x6f,0x66,0x6f,0x7f,0x6f,0x7f
  ,0x6f,0x6f,0x6e,0x6e,0x66,0x6f,0x13,0x13,0x1d,0x66,0x1d,0x25,0x66,0x65
  ,0x6e,0x1d,0x1d,0x0a,0x14,0x6f,0x77,0x77,0x2e,0x25,0x1c,0x0a,0x65,0x6f
  ,0x6f,0x6f,0x7f,0x6f,0x77,0x77,0x66,0x6f,0x6e,0x6e,0x25,0x14,0x01,0x0a
  ,0x09,0x0a,0x0b,0x14,0x1d,0x0a,0x14,0x13,0x66,0x6f,0x6f,0x25,0x1c,0x5d
  ,0x65,0x01,0x66,0x1e,0x6f,0x6f,0x6f,0x77,0x6f,0x6f,0x6e,0x6f,0x66,0x1d
  ,0x1d,0x0a,0x66,0x6f,0x66,0x2e,0x6e,0x6e,0x14,0x13,0x12,0x14,0x6f,0x6f
  ,0x6e,0x6e,0x25,0x0a,0x13,0x12,0x6e,0x6f,0x66,0x6f,0x66,0x65,0x66,0x6f
  ,0x66,0x25,0x66,0x25,0x0a,0x01,0x6e,0x77,0x77,0x6f,0x6f,0x6e,0x6e,0x14
  ,0x1d,0x6e,0x1b,0x25,0x1d,0x1c,0x1d,0x14,0x01,0x25,0x25,0x66,0x6e,0x6f
  ,0x1d,0x6f,0x66,0x1d,0x1d,0x1d,0x6e,0x1d,0x0b,0x6e,0x6f,0x77,0x7f,0x6f
  ,0x77,0x77,0x66,0x66,0x13,0x5d,0x13,0x0a,0x01,0x1c,0x13,0x0a,0x6e,0x09
  ,0x1d,0x65,0x25,0x66,0x66,0x6e,0x65,0x25,0x65,0x6e,0x1d,0x65,0x13,0x65
  ,0x6f,0x77,0x67,0x7f,0x6f,0x77,0x6f,0x6e,0x14,0x1c,0x1c,0x6e,0x77,0x6e
  ,0x77,0x6f,0x25,0x0a,0x1c,0x1d,0x1d,0x66,0x1d,0x6e,0x15,0x26,0x25,0x5e
  ,0x25,0x13,0x1d,0x77,0x77,0x66,0x66,0x6e,0x6f,0x77,0x7f,0x6e,0x12,0x13
  ,0x66,0x77,0x6f,0x77,0x6f,0x77,0x26,0x6f,0x0b,0x25,0x1c,0x25,0x25,0x25
  ,0x66,0x1d,0x1b,0x25,0x0a,0x1d,0x6e,0x67,0x67,0x6f,0x66,0x1d,0x65,0x6e
  ,0x67,0x6f,0x6e,0x13,0x77,0x7f,0x67,0x77,0x77,0x77,0x6f,0x6e,0x5d,0x13
  ,0x14,0x1d,0x1c,0x1d,0x1d,0x14,0x1d,0x0a,0x09,0x1c,0x66,0x77,0x66,0x67
  ,0x6e,0x1c,0x65,0x66,0x66,0x6d,0x1c,0x09,0x6e,0x67,0x6f,0x7f,0x6f,0x77
  ,0x6f,0x6e,0x66,0x0a,0x01,0x13,0x1d,0x14,0x65,0x14,0x02,0x01,0x1d,0x1c
  ,0x13,0x66,0x2e,0x66,0x77,0x6e,0x1e,0x25,0x25,0x6e,0x0a,0x13,0x6e,0x67
  ,0x26,0x67,0x6f,0x77,0x67,0x6f,0x1d,0x6e,0x14,0x0a,0x01,0x13,0x01,0x25
  ,0x66,0x77,0x66,0x65,0x1c,0x0b,0x66,0x25,0x66,0x66,0x25,0x6e,0x66,0x1d
  ,0x14,0x0a,0x1d,0x66,0x66,0x66,0x6f,0x6e,0x6f,0x6f,0x5d,0x1d,0x0a,0x25
  ,0x26,0x6f,0x6f,0x6f,0x7f,0x6f,0x6f,0x6f,0x77,0x0a,0x09,0x1d,0x6f,0x6e
  ,0x15,0x25,0x1c,0x1d,0x1d,0x13,0x25,0x66,0x1c,0x6e,0x66,0x26,0x66,0x66
  ,0x25,0x1c,0x0a,0x66,0x6f,0x7f,0x6f,0x6f,0x6f,0x66,0x6f,0x77,0x6e,0x66
  ,0x0a,0x1b,0x26,0x1d,0x5e,0x1d,0x65,0x13,0x0a,0x1d,0x66,0x6e,0x25,0x6e
  ,0x1d,0x1d,0x6e,0x25,0x13,0x1d,0x14,0x5e,0x77,0x27,0x7f,0x77,0x7f,0x67
  ,0x66,0x67,0x1d,0x1d,0x13,0x09,0x76,0x66,0x24,0x1c,0x1b,0x13,0x13,0x1d
  ,0x25,0x25,0x5e,0x1d,0x6e,0x25,0x1c,0x25,0x66,0x1d,0x09,0x1d,0x77,0x6f
  ,0x66,0x6f,0x66,0x66,0x66,0x6e,0x65,0x66,0x65,0x1c,0x1d,0x1d,0x65,0x14
  ,0x1c,0x25,0x0a,0x0a,0x0a,0x25,0x1d,0x66,0x14,0x25,0x66,0x6e,0x1d,0x25
  ,0x09,0x1c,0x66,0x77,0x6f,0x66,0x7f,0x77,0x1d,0x6e,0x1c,0x1d,0x1d,0x0a
  ,0x25,0x66,0x1d,0x25,0x13,0x0a,0x1d,0x14,0x1e,0x13,0x1d,0x14,0x65,0x66
  ,0x66,0x25,0x1d,0x14,0x0a,0x14,0x15,0x65,0x7f,0x77,0x66,0x25,0x1d,0x1d
  ,0x1d,0x6e,0x1c,0x14,0x13,0x65,0x1d,0x1c,0x0a,0x0a,0x65,0x1d,0x25,0x0a
  ,0x1c,0x1d,0x1c,0x25,0x1d,0x1c,0x14,0x01,0x1b,0x0a,0x1d,0x65,0x6f,0x1d
  ,0x66,0x65,0x6e,0x66,0x1d,0x14,0x0a,0x0a,0x1b,0x12,0x1c,0x0a,0x14,0x6e
  ,0x66,0x25
};

const unsigned char texture3[] =
{  32, 32 // width, height
  ,0xb6,0x3d,0x33,0x33,0x3e,0x33,0x2a,0x29,0x2a,0x29,0x2a,0x20,0x29,0x29
  ,0x2a,0x29,0x10,0x10,0x10,0x49,0x58,0x99,0x50,0x50,0x10,0x4b,0x54,0x54
  ,0x55,0x55,0x4a,0x49,0x29,0x3d,0x3d,0x7c,0x3d,0x33,0x34,0x3d,0x33,0x33
  ,0x33,0x20,0x20,0x29,0x10,0x20,0x10,0x50,0x10,0x49,0x10,0x99,0x51,0x51
  ,0x33,0x10,0x53,0x66,0x67,0x15,0x54,0x4b,0x7f,0x33,0x7f,0x3d,0x3e,0x3d
  ,0x33,0x3d,0x29,0x29,0x2a,0x20,0x20,0x2a,0x10,0x20,0x10,0x51,0x50,0x50
  ,0x58,0x51,0x51,0x51,0x33,0x33,0x10,0x55,0x5e,0x67,0x26,0x66,0x29,0x7f
  ,0x3e,0x3d,0x3d,0x33,0x3d,0x33,0x29,0x10,0x10,0x20,0x33,0x10,0x10,0x2a
  ,0x58,0x59,0x50,0x10,0x51,0x51,0x10,0x51,0x52,0x37,0x37,0x10,0x10,0x5d
  ,0x53,0x55,0x6c,0x3e,0x3d,0x33,0x33,0x3d,0x33,0x29,0x10,0x2a,0x20,0x20
  ,0x29,0x20,0x10,0x10,0x58,0x10,0x10,0x10,0x58,0x49,0x51,0x99,0x49,0x37
  ,0x37,0x33,0x33,0x10,0x10,0x49,0x7f,0x7f,0x33,0x3d,0x3d,0x7f,0x33,0x3e
  ,0x29,0x2a,0x29,0x2a,0x29,0x2a,0x20,0x20,0x20,0x50,0x51,0x10,0x58,0x51
  ,0x51,0x99,0x49,0x00,0x33,0x10,0x10,0x10,0x10,0x10,0x3d,0x7f,0x3d,0x33
  ,0x3d,0x3d,0x33,0x2a,0x2a,0x20,0x29,0x29,0x29,0x10,0x20,0x19,0x10,0x19
  ,0x51,0x50,0x58,0x51,0x49,0x99,0x00,0x00,0x10,0x10,0x33,0x33,0x33,0x10
  ,0x3d,0x7f,0x7f,0x33,0x3e,0x33,0x33,0x33,0x29,0x20,0x33,0x2a,0x20,0x58
  ,0x58,0x19,0x19,0x51,0x10,0x51,0x59,0x51,0x51,0x51,0x00,0x10,0x10,0x00
  ,0x00,0x10,0x10,0x10,0x3d,0xb6,0x2a,0x3e,0x33,0x3d,0x33,0x3d,0x2a,0x10
  ,0x20,0x2a,0x10,0x58,0x58,0x19,0x50,0x19,0x58,0x10,0x58,0x99,0x51,0x49
  ,0x00,0x00,0x00,0x00,0x00,0x00,0x10,0x10,0x3d,0x2a,0x6c,0x3d,0x3e,0x33
  ,0x33,0x29,0x33,0x29,0x10,0x29,0x20,0x69,0x58,0x10,0x19,0x10,0x99,0x10
  ,0x58,0x99,0x49,0x99,0x51,0x00,0x00,0x00,0x10,0x10,0x10,0x53,0xb6,0x3d
  ,0x6c,0x33,0x33,0x33,0x3d,0x29,0x20,0x33,0x20,0x20,0x29,0x20,0x51,0x51
  ,0x51,0x50,0x99,0x58,0x99,0x49,0x99,0x49,0x00,0x00,0x00,0x10,0x10,0x00
  ,0x00,0x53,0x29,0x3e,0x7f,0x29,0x33,0x2a,0x33,0x29,0x29,0x33,0x20,0x29
  ,0x10,0x58,0x10,0x49,0x51,0x58,0x99,0x49,0x49,0x51,0x51,0x00,0x00,0x00
  ,0x00,0x00,0x00,0x00,0x53,0x33,0x29,0x33,0x7e,0x33,0xb6,0x2a,0x33,0x29
  ,0x20,0x10,0x29,0x20,0x10,0x58,0x59,0x10,0x50,0x99,0x49,0x49,0x49,0x49
  ,0x51,0x9a,0x4b,0x53,0x53,0x4a,0x4a,0x15,0x37,0x37,0x2d,0x7c,0x3e,0x3d
  ,0x7f,0x29,0x2a,0x10,0x29,0x10,0x10,0x20,0x29,0x58,0x51,0x51,0x99,0x51
  ,0x49,0x49,0x99,0x10,0x49,0x49,0x4a,0x4b,0x53,0x56,0x56,0x33,0x37,0x37
  ,0x6c,0x33,0x3d,0x3d,0x3d,0x7f,0x20,0x10,0x29,0x10,0x10,0x99,0x10,0x58
  ,0x51,0x49,0x49,0x49,0x49,0x58,0x49,0x51,0x4a,0x4b,0x4b,0x54,0x66,0x67
  ,0x67,0x33,0x10,0x10,0xb6,0x3d,0x33,0x7f,0x29,0x3d,0x33,0x33,0x29,0x29
  ,0xac,0x58,0x10,0x49,0x49,0x49,0x49,0x49,0x49,0x50,0x59,0x51,0x51,0x51
  ,0x54,0x15,0x66,0x5e,0x33,0x10,0x56,0x54,0xb6,0x3d,0x33,0x33,0x3e,0x33
  ,0x29,0x33,0x33,0x33,0x10,0x10,0x10,0x51,0x49,0x49,0x49,0x49,0x49,0x49
  ,0x58,0x50,0x50,0x49,0x4b,0x54,0x54,0x55,0x55,0x55,0x4a,0x49,0x29,0x3d
  ,0x3d,0x7c,0x7f,0x6c,0x2a,0x29,0x10,0x10,0x10,0x19,0x20,0x10,0x99,0x51
  ,0x51,0x49,0x49,0x49,0x10,0x51,0x51,0x51,0x52,0x53,0x66,0x67,0x67,0x15
  ,0x54,0x4b,0x29,0x33,0x7f,0x3d,0x6c,0x29,0x33,0x20,0x29,0x10,0x20,0x20
  ,0x99,0x10,0x99,0x51,0x58,0x99,0x49,0x49,0x49,0x51,0x51,0x4b,0x4b,0x53
  ,0x55,0x5e,0x5e,0x67,0x26,0x66,0x29,0x7f,0x3e,0x3d,0x29,0x29,0x33,0x2a
  ,0x2a,0x20,0x33,0x20,0x61,0x20,0x58,0x10,0x10,0x99,0x99,0x49,0x49,0x10
  ,0x51,0x52,0x4a,0x4b,0x53,0x54,0x54,0x5d,0x53,0x55,0x6c,0x7f,0x6c,0x29
  ,0x33,0x3d,0x2a,0x20,0x29,0x2a,0x10,0x2a,0x20,0x20,0x58,0x10,0x10,0x58
  ,0x99,0x51,0x51,0x49,0x99,0x00,0x00,0x00,0x00,0x00,0x10,0x00,0x54,0x54
  ,0xb6,0xb6,0x33,0x3d,0x3e,0x3e,0x2a,0x33,0x20,0x2a,0x20,0x33,0x2a,0x58
  ,0x58,0x50,0x10,0x58,0x10,0x10,0x58,0x99,0x00,0x99,0x00,0x00,0x00,0x00
  ,0x33,0x10,0x00,0x49,0xb6,0x29,0x3f,0x33,0x33,0x33,0x3d,0x33,0x20,0x2a
  ,0x33,0x2a,0x29,0x58,0x99,0x10,0x50,0x10,0x10,0x10,0x10,0x99,0x51,0x00
  ,0x51,0x00,0x10,0x10,0x33,0x33,0x10,0x00,0x3d,0x6c,0x3d,0x33,0x3e,0x33
  ,0x33,0x2a,0x2a,0x2a,0x33,0x29,0x10,0x58,0x58,0x10,0x10,0x10,0x51,0x10
  ,0x50,0x99,0x51,0x51,0x00,0x00,0x00,0x10,0x10,0x37,0x37,0x10,0x7f,0xb6
  ,0x33,0x3d,0x33,0x3f,0x2a,0x33,0x2a,0x33,0x29,0x10,0x10,0x29,0x10,0x33
  ,0x10,0x50,0x49,0x10,0x51,0x51,0x51,0x51,0x51,0x00,0x00,0x00,0x00,0x33
  ,0x37,0x33,0x3d,0x2a,0x3d,0x3d,0x3e,0x33,0x33,0x2a,0x2a,0x33,0x33,0x10
  ,0x2a,0x20,0x10,0x10,0x10,0x50,0x49,0x58,0x51,0x51,0x58,0x99,0x00,0x00
  ,0x00,0x00,0x00,0x10,0x33,0x33,0x7f,0x3d,0x33,0x7f,0x3d,0x3d,0x33,0x3d
  ,0x20,0x29,0x20,0x33,0x2a,0x10,0x10,0x2a,0x10,0x10,0x10,0x58,0x99,0x51
  ,0x59,0x99,0x00,0x00,0x00,0x00,0x10,0x10,0x10,0x33,0x29,0x3e,0x3d,0x3d
  ,0x7c,0x3e,0x2a,0x3d,0x2c,0x2a,0x20,0x33,0x29,0x20,0x10,0x2a,0x10,0x10
  ,0x49,0x10,0x99,0x49,0x51,0x51,0x49,0x00,0x10,0x10,0x10,0x10,0x53,0x14
  ,0x29,0x33,0x7e,0x33,0x3d,0x3d,0x33,0x33,0x29,0x20,0x20,0x33,0x20,0x20
  ,0x20,0x20,0x10,0x51,0x50,0x10,0x99,0x49,0x49,0x51,0x9a,0x4b,0x53,0x53
  ,0x4a,0x15,0x56,0x54,0x2d,0x7c,0x3e,0x3d,0x3d,0x7e,0x3d,0x3d,0x2a,0x33
  ,0x10,0x29,0x2a,0x29,0x10,0x20,0x10,0x58,0x58,0x58,0x99,0x51,0x10,0x49
  ,0x49,0x4a,0x4b,0x53,0x56,0x54,0x4b,0x54,0x6c,0x33,0x3d,0x3d,0x3d,0x33
  ,0x3d,0x33,0x29,0x33,0x20,0x20,0x20,0x10,0x2a,0x10,0x50,0x99,0x58,0x58
  ,0x51,0x51,0x51,0x4a,0x4b,0x4b,0x54,0x66,0x67,0x56,0x2e,0x5e,0xb6,0x3d
  ,0x33,0x7f,0x29,0x3d,0x2a,0x29,0x33,0x29,0x20,0x2a,0x10,0x10,0x2a,0x10
  ,0x10,0x50,0x10,0x50,0x59,0x51,0x51,0x51,0x51,0x54,0x15,0x66,0x5e,0x15
  ,0x56,0x54
};

const unsigned char texture4[] =
{  32, 32 // width, height
  ,0x12,0x1c,0x65,0x65,0x65,0x65,0x1c,0x1c,0x1c,0x1c,0x1c,0x1b,0x1b,0x1b
  ,0x13,0x00,0x5b,0x5b,0xad,0xad,0xad,0xad,0x5b,0x00,0x64,0xad,0xad,0x64
  ,0x64,0x64,0x5b,0x00,0x1c,0x1b,0x1b,0x1b,0x13,0x13,0x12,0x13,0x12,0x12
  ,0x12,0x12,0x0a,0x0a,0x09,0x00,0x5b,0x5b,0xad,0xad,0xad,0x64,0x5b,0x00
  ,0xad,0x64,0x64,0x64,0x64,0x64,0x5b,0x00,0x65,0x1b,0x1b,0x13,0x13,0x12
  ,0x0a,0x0a,0x12,0x0a,0x0a,0x0a,0x0a,0x09,0x09,0x00,0x5b,0x5b,0xad,0xad
  ,0xad,0x64,0x5b,0x00,0xad,0x64,0x64,0x64,0x64,0x64,0x5b,0x00,0x65,0x1b
  ,0x13,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x09,0x0a,0x09,0x00
  ,0x5b,0x5b,0xad,0xad,0x64,0x64,0x5b,0x00,0x64,0x64,0x64,0x64,0x64,0x64
  ,0x12,0x00,0x65,0x13,0x12,0x00,0x09,0x09,0x0a,0x09,0x09,0x09,0x09,0x09
  ,0x13,0x0a,0x09,0x00,0x5b,0x5b,0xad,0x64,0x64,0x5b,0x5b,0x00,0x64,0x64
  ,0x64,0x64,0x64,0x64,0x5b,0x00,0x65,0x13,0x12,0x00,0x09,0x0a,0x12,0x12
  ,0x0a,0x12,0x12,0x0a,0x1b,0x0a,0x09,0x00,0x5b,0x5b,0x64,0x64,0x5b,0x64
  ,0x5b,0x00,0x64,0x64,0x64,0x64,0x64,0x64,0x5b,0x00,0x65,0x12,0x13,0x00
  ,0x09,0x12,0x0a,0x0a,0x12,0x0a,0x0a,0x0a,0x1c,0x09,0x0a,0x00,0x5b,0x5b
  ,0x64,0x64,0x64,0x64,0x5b,0x00,0x5b,0x5b,0x5b,0x12,0x5b,0x12,0x5b,0x00
  ,0x65,0x13,0x12,0x00,0x09,0x12,0x0a,0x0a,0x0a,0x09,0x0a,0x09,0x1c,0x0a
  ,0x0a,0x00,0x5b,0x5b,0xad,0x64,0x5b,0x64,0x5b,0x00,0x00,0x00,0x00,0x00
  ,0x00,0x00,0x00,0x00,0x1c,0x12,0x0a,0x00,0x09,0x12,0x0a,0x0a,0x09,0x0a
  ,0x09,0x09,0x1c,0x0a,0x0a,0x00,0x52,0x52,0x64,0x64,0x64,0x5b,0x5b,0x00
  ,0xad,0xad,0xad,0xad,0xad,0xad,0x5b,0x00,0x1c,0x12,0x0a,0x00,0x09,0x0a
  ,0x0a,0x0a,0x09,0x09,0x09,0x09,0x65,0x0a,0x09,0x00,0x5b,0x52,0x64,0x64
  ,0x5b,0x64,0x5b,0x00,0xad,0xad,0xad,0xad,0xad,0x64,0x5b,0x00,0x1b,0x0a
  ,0x12,0x00,0x09,0x0a,0x09,0x09,0x0a,0x0a,0x09,0x09,0x65,0x09,0x09,0x00
  ,0x5b,0x5b,0x5b,0x64,0x5b,0x5b,0x5b,0x00,0xad,0xad,0xad,0xad,0xad,0x64
  ,0x5b,0x00,0x1b,0x12,0x0a,0x00,0x09,0x0a,0x0a,0x09,0x0a,0x09,0x09,0x0a
  ,0x1c,0x09,0x09,0x00,0x52,0x12,0x64,0x64,0x64,0x5b,0x5b,0x00,0xad,0xad
  ,0xad,0xad,0x64,0x64,0x5b,0x00,0x1b,0x0a,0x09,0x09,0x13,0x1b,0x1c,0x1c
  ,0x65,0x65,0x1c,0x1c,0x1b,0x09,0x09,0x00,0x5b,0x52,0x64,0x64,0x64,0x5b
  ,0x12,0x00,0xad,0xad,0xad,0x64,0x64,0x5b,0x5b,0x00,0x1b,0x0a,0x09,0x0a
  ,0x09,0x09,0x09,0x0a,0x09,0x0a,0x09,0x09,0x09,0x09,0x09,0x00,0x12,0x12
  ,0x5b,0x5b,0x5b,0x5b,0x5b,0x00,0xad,0xad,0x64,0x64,0x5b,0x5b,0x12,0x00
  ,0x13,0x09,0x09,0x09,0x09,0x09,0x0a,0x0a,0x09,0x09,0x09,0x09,0x09,0x09
  ,0x09,0x00,0x09,0x09,0x5b,0x12,0x12,0x12,0x12,0x00,0xad,0xad,0x64,0x64
  ,0x64,0x64,0x5b,0x00,0x0a,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
  ,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
  ,0xad,0x64,0x64,0x5b,0x5b,0x5b,0x12,0x00,0x12,0x1c,0x65,0x65,0x65,0x65
  ,0x1c,0x1c,0x1c,0x1c,0x1c,0x1b,0x1b,0x1b,0x13,0x00,0x5b,0x5b,0xad,0x64
  ,0xad,0xad,0x5b,0x00,0x64,0x64,0x64,0x5b,0x5b,0x64,0x5b,0x00,0x1c,0x1b
  ,0x1b,0x1b,0x1b,0x13,0x12,0x13,0x12,0x12,0x12,0x12,0x0a,0x0a,0x09,0x00
  ,0x5b,0x5b,0xad,0xad,0xad,0x64,0x5b,0x00,0xad,0x64,0x5b,0x5b,0x5b,0x5b
  ,0x5b,0x00,0x65,0x1b,0x1b,0x13,0x13,0x12,0x0a,0x0a,0x12,0x0a,0x0a,0x0a
  ,0x0a,0x09,0x09,0x00,0x5b,0x5b,0xad,0x64,0x64,0x64,0x5b,0x00,0x64,0x64
  ,0x64,0x64,0x5b,0x5b,0x5b,0x00,0x65,0x13,0x1b,0x00,0x00,0x00,0x00,0x00
  ,0x00,0x00,0x00,0x00,0x09,0x0a,0x09,0x00,0x5b,0x5b,0x64,0x64,0x64,0x64
  ,0x5b,0x00,0x64,0x64,0x64,0x64,0x5b,0x64,0x5b,0x00,0x65,0x1b,0x13,0x00
  ,0x09,0x09,0x0a,0x09,0x09,0x09,0x09,0x09,0x1b,0x0a,0x09,0x00,0x5b,0x5b
  ,0xad,0x64,0x5b,0x64,0x5b,0x00,0x64,0x64,0x64,0x64,0x5b,0x5b,0x5b,0x00
  ,0x65,0x13,0x12,0x00,0x09,0x0a,0x12,0x12,0x0a,0x12,0x12,0x12,0x1b,0x0a
  ,0x09,0x00,0x5b,0x52,0x64,0x64,0x5b,0x5b,0x5b,0x00,0x64,0x64,0x64,0x64
  ,0x5b,0x5b,0x12,0x00,0x65,0x12,0x13,0x00,0x09,0x12,0x0a,0x0a,0x0a,0x0a
  ,0x0a,0x0a,0x1c,0x09,0x0a,0x00,0x5b,0x5b,0x64,0x64,0x5b,0x64,0x5b,0x00
  ,0x5b,0x5b,0x5b,0x5b,0x5b,0x12,0x5b,0x00,0x65,0x13,0x12,0x00,0x09,0x12
  ,0x0a,0x0a,0x0a,0x09,0x0a,0x09,0x1c,0x0a,0x0a,0x00,0x5b,0x52,0x64,0x5b
  ,0x64,0x5b,0x5b,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1c,0x12
  ,0x0a,0x00,0x09,0x12,0x0a,0x0a,0x09,0x0a,0x09,0x09,0x1c,0x0a,0x0a,0x00
  ,0x52,0x5b,0x64,0x64,0x5b,0x5b,0x5b,0x00,0xad,0xad,0xad,0xad,0xad,0xad
  ,0x5b,0x00,0x1c,0x12,0x0a,0x00,0x09,0x0a,0x0a,0x0a,0x09,0x09,0x09,0x09
  ,0x65,0x0a,0x09,0x00,0x5b,0x52,0x64,0x64,0x5b,0x5b,0x5b,0x00,0xad,0xad
  ,0xad,0xad,0xad,0x64,0x5b,0x00,0x1b,0x0a,0x12,0x00,0x09,0x0a,0x09,0x09
  ,0x0a,0x0a,0x09,0x09,0x65,0x09,0x09,0x00,0x52,0x12,0x64,0x5b,0x5b,0x5b
  ,0x5b,0x00,0xad,0xad,0xad,0xad,0xad,0x64,0x5b,0x00,0x1b,0x12,0x0a,0x00
  ,0x09,0x0a,0x0a,0x09,0x0a,0x09,0x09,0x0a,0x1c,0x09,0x09,0x00,0x52,0x52
  ,0x5b,0x5b,0x5b,0x64,0x5b,0x00,0xad,0xad,0xad,0xad,0x64,0x64,0x5b,0x00
  ,0x1b,0x0a,0x09,0x09,0x13,0x1b,0x1c,0x1c,0x65,0x65,0x1c,0x1c,0x1b,0x09
  ,0x09,0x00,0x12,0x12,0x5b,0x5b,0x5b,0x5b,0x5b,0x00,0xad,0xad,0xad,0xad
  ,0xad,0x64,0x5b,0x00,0x1b,0x0a,0x09,0x0a,0x0a,0x09,0x09,0x09,0x09,0x09
  ,0x09,0x09,0x09,0x09,0x09,0x00,0x12,0x12,0x5b,0x5b,0x5b,0x5b,0x12,0x00
  ,0xad,0xad,0xad,0x64,0xad,0x64,0x5b,0x00,0x13,0x09,0x0a,0x09,0x09,0x09
  ,0x09,0x09,0x09,0x09,0x09,0x09,0x09,0x09,0x09,0x00,0x09,0x12,0x5b,0x5b
  ,0x5b,0x12,0x5b,0x00,0xad,0xad,0x64,0x64,0xad,0x64,0x5b,0x00,0x0a,0x00
  ,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
  ,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xad,0x64,0xad,0x64,0xad,0x64
  ,0x12,0x00
};

const unsigned char *textures[] = {texture1, texture2, texture3, texture4};

void clearScreen()
{
  memset(pixels, 0, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t));
}

void clearPixelCounter()
{
  memset(pixelCounter, 0, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t));
}

static inline uint8_t rgbToIndex(uint8_t r, uint8_t g, uint8_t b)
{
  return (r & 7) | ((g & 7) << 3) | ((b & 7) << 6);
}

uint8_t sampleImage(const unsigned char *image, RCL_Unit x, RCL_Unit y)
{
  x = RCL_wrap(x,RCL_UNITS_PER_SQUARE);
  y = RCL_wrap(y,RCL_UNITS_PER_SQUARE);

  int32_t index =
    image[1] * ((image[0] * x) / RCL_UNITS_PER_SQUARE) +
    (image[0] * y) / RCL_UNITS_PER_SQUARE;

  return image[2 + index];
}

uint8_t addIntensity(uint8_t color, int16_t intensity)
{
  uint8_t r = color & 7;
  uint8_t g = (color & 56) >> 3;
  uint8_t b = (color & 172) >> 6;

  if (intensity >= 0)
  {
    r += intensity;
    r = r > 7 ? 7 : r;

    g += intensity;
    g = g > 7 ? 7 : g;

    b += intensity / 2;
    b = b > 3 ? 3 : b;
  }
  else
  {
    intensity *= -1;
    r = (intensity > r) ? 0 : r - intensity;
    g = (intensity > g) ? 0 : g - intensity;
    intensity /= 2;
    b = intensity > b ? 0 : b - intensity;
  }

  return rgbToIndex(r,g,b);
}

RCL_Unit textureAt(int16_t x, int16_t y)
{
  if (x >= 0 && x < LEVEL_X_RES && y >= 0 && y < LEVEL_Y_RES)
    return levelTexture[(LEVEL_Y_RES - y -1) * LEVEL_X_RES + x]; 

  return 0;
}

RCL_Unit floorHeightAt(int16_t x, int16_t y)
{
  if (x == 6 && (y == 13 || y == 14)) // moving lift
    return ((RCL_abs(-1 * (frame % 64) + 32)) * RCL_UNITS_PER_SQUARE) / 8; 

  if (x >= 0 && x < LEVEL_X_RES && y >= 0 && y < LEVEL_Y_RES)
    return (levelFloor[(LEVEL_Y_RES - y -1) * LEVEL_X_RES + x] * RCL_UNITS_PER_SQUARE) / 8; 

  int a = RCL_abs(x - LEVEL_X_RES / 2) - LEVEL_X_RES / 2;
  int b = RCL_abs(y - LEVEL_Y_RES / 2) - LEVEL_Y_RES / 2;

  return (a > b ? a : b) * RCL_UNITS_PER_SQUARE;
}

RCL_Unit ceilingHeightAt(int16_t x, int16_t y)
{
  int v = 1024;

  if (x >= 0 && x < LEVEL_X_RES && y >= 0 && y < LEVEL_Y_RES)
    v = levelCeiling[(LEVEL_Y_RES - y -1) * LEVEL_X_RES + x]; 

  return (v * RCL_UNITS_PER_SQUARE) / 8;
}

/**
  Function for drawing a single pixel (like fragment shader). Bottleneck =>
  should be as fast as possible.
*/
void pixelFunc(RCL_PixelInfo *pixel)
{
  uint8_t c;

  if (pixel->isWall)
    c = sampleImage(textures[pixel->hit.type], pixel->texCoords.x, pixel->texCoords.y);
  else
    c = pixel->height == RCL_FLOOR_TEXCOORDS_HEIGHT ?
      sampleImage(textures[0], pixel->texCoords.x, pixel->texCoords.y) : 0b00010001;

  int intensity = pixel->depth - 8 * RCL_UNITS_PER_SQUARE;
  intensity = intensity < 0 ? 0 : intensity;
  intensity = (intensity * 32) / RCL_UNITS_PER_SQUARE;

  int32_t color = palette[c];

  int32_t r = ((color & 0xFF000000) >> 24) - intensity;
  r = r > 0 ? r : 0;

  int32_t g = ((color & 0x00FF0000) >> 16) - intensity;
  g = g > 0 ? g : 0;

  int32_t b = ((color & 0x0000FF00) >> 8) - intensity;
  b = b > 0 ? b : 0;

  int32_t index = pixel->position.y * SCREEN_WIDTH + pixel->position.x;
  
  // Convert to Fenster's RGB format (24-bit)
  pixels[index] = (r << 16) | (g << 8) | b;
  pixelCounter[index]++;
}

void draw()
{
  RCL_RayConstraints c;
  c.maxHits = 32;
  c.maxSteps = 32;
  RCL_renderComplex(camera, floorHeightAt, ceilingHeightAt, textureAt, c);
}

int main()
{
  // Initialize palette
  for (uint8_t r = 0; r < 8; ++r)
    for (uint8_t g = 0; g < 8; ++g)
      for (uint8_t b = 0; b < 4; ++b)
        palette[rgbToIndex(r,g,b)] = ((36 * r) << 24) | ((36 * g) << 16) | ((85 * b) << 8);

  // Initialize camera
  camera.position.x = 11237;
  camera.position.y = 12392;
  camera.shear = -50;
  camera.direction = -415;
  camera.height = 4648;
  camera.resolution.x = SCREEN_WIDTH;
  camera.resolution.y = SCREEN_HEIGHT;

  // Initialize Fenster
  struct fenster f = {
    .title = "Raycasting Demo",
    .width = SCREEN_WIDTH,
    .height = SCREEN_HEIGHT,
    .buf = pixels
  };

  if (fenster_open(&f) != 0) {
    fprintf(stderr, "Failed to open window\n");
    return 1;
  }

  int64_t lastTime = fenster_time();
  int fps = 0;

  // Main loop
  while (fenster_loop(&f) == 0) {
    draw();
    fps++;


    int step = 1;
    int step2 = 5;

    RCL_Vector2D direction = RCL_angleToDirection(camera.direction);
    direction.x /= 10;
    direction.y /= 10;

    // Handle input
    if (f.keys[KEY_UP]) {
      camera.position.x += step * direction.x;
      camera.position.y += step * direction.y;
    }
    else if (f.keys[KEY_DOWN]) {
      camera.position.x -= step * direction.x;
      camera.position.y -= step * direction.y;
    }

    if (f.keys[KEY_Q])
      camera.height += step * 100;
    else if (f.keys[KEY_W])
      camera.height -= step * 100;

    if (f.keys[KEY_RIGHT])
      camera.direction += step2;
    else if (f.keys[KEY_LEFT])
      camera.direction -= step2;

    const int shearAdd = 10;

    if (f.keys[KEY_A])
      camera.shear = camera.shear + shearAdd <= SCREEN_HEIGHT ? camera.shear + shearAdd : SCREEN_HEIGHT;
    else if (f.keys[KEY_S])
      camera.shear = camera.shear - shearAdd >= -1 * SCREEN_HEIGHT ? camera.shear - shearAdd : -1 * SCREEN_HEIGHT;

    // Calculate and display FPS
    int64_t currentTime = fenster_time();
    if (currentTime - lastTime >= 1000) {
      printf("FPS: %d\n", fps);
      fps = 0;
      lastTime = currentTime;
    }

    frame++;
  }

  fenster_close(&f);
  return 0;
}
