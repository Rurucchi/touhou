/*  ----------------------------------- INFOS
	This header file is the base to most of the other files and headers in this project.
	
*/

#ifndef _TYPESH_
#define _TYPESH_

#include <windows.h>

#define internal static
#define local_persist static
#define global_variable static

//  ----------------------------------- GENERAL

typedef unsigned int uint;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

//  ----------------------------------- NUMBERS AND MEMORY

#define Assert(expression)                                                     \
  if(!(expression)) {                                                          \
    *(int *)0 = 0;                                                             \
}

uint32 SafeTruncateUInt64(uint64 value){
  Assert(value <= 0xFFFFFFFF);
  uint32 result = (uint32)value;
  return result;
}

typedef struct floatPOINT{
	float x;
	float y;
} floatPOINT;

//  ----------------------------------- PLATFORM


typedef struct completeFile {
	int32 size;
	void *memory;
} completeFile;

typedef struct file_bitmap_header
{
    // header: 14 bytes
    char formatID[2];
    uint32 size;
    uint32 pad;
    uint32 offset; // start of the data array
} file_bitmap_header;

typedef struct file_bitmap_info_header
{
    uint32 headerSize;
    int32 width;
    int32 height;
    uint16 numPlanes;
    uint16 bpp; // bits per pixel
    char compression[4];
    uint32 imageSize;
    int32 hres;
    int32 vres;
    uint32 numColors; // number of colors in the palette
    uint32 numImpColors; // number of important colors used
} file_bitmap_info_header;

typedef struct file_bitmap+- {
	file_bitmap_header *bitmapHeader;
	file_bitmap_info_header *bitmapInfoHeader;
} file_bitmap;

// ----------------------------------- GAME LOGIC

// game state 
typedef struct game_state {
	int pause;	// if the game is paused (menu has the game paused by default?) 0 or 1.
	int level;	// level of the game, 0 is the main menu
	int difficulty; // difficulty : easy(0), medium(1), hard(2)
} game_state;

// game size
typedef struct virtual_game_size {
	int horizontal;
	int vertical;
} virtual_game_size;

//  ----------------------------------- RENDERING

// bitmap stuff
typedef struct win32_offscreen_buffer {
	BITMAPINFO info;
	POINT MousePos;
	void *memory;
	int width;
	int height;
	int bytesPerPixel;
} win32_offscreen_buffer;

// rectangle
typedef struct win32_rect {
	RECT rectangle;
	int width;
	int height;
} win32_rect;

// texture
typedef struct texture {
	void *memory;
	int memorySize;
	int width;
	int height;
	int bytesPerPixel;
} texture;

#endif /* _TYPESH_ */