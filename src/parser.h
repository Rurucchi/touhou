#ifndef _PARSERH_
#define _PARSERH_

#include <windows.h>
#include "./types.h"
#include "./platform.h"

#define internal static
#define local_persist static
#define global_variable static

// LOCAL DEPENDENCIES : TYPES.H, PLATFORM.H

// reference : https://en.wikipedia.org/wiki/BMP_file_format

internal file_bitmap BMPToTexture(completeFile *file, texture *sprite) {
	
	file_bitmap bitmap = {0};
	
	// ------- HEADER : 40 bytes
	
	// memory 
	uint8 *memory = (uint8*)file->memory;
	
	bitmap.bitmapHeader = (file_bitmap_header*)memory;
	bitmap.bitmapInfoHeader = (file_bitmap_info_header*)memory;
	
	// base int is OK bc 4 bytes long
	sprite->width = *(int*)(memory + 18);
	sprite->height = *(int*)(memory + 22);
	sprite->memorySize = *(int*)(memory + 14);
	
	// int8 bc otherwise it reads too far
	sprite->bytesPerPixel = *(int8*)(memory + 28);
	uint32 bitmapSize = *(int32*)(memory + 34);
	
	// setting the memory to the texture struct
	sprite->memory = memory + bitmap.bitmapHeader->offset;
	
	return bitmap;
	
}	

#endif /* _PARSERH_ */