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

internal file_bitmap parseBMP(completeFile *file) {
	file_bitmap bitmap = {0};
	file_bitmap_header *bitmapHeader;
	file_bitmap_info_header *bitmapInfoHeader;
	
	// parsing
	uint8 *memory = (uint8*)file->memory;
	bitmapHeader = (file_bitmap_header*)memory;
	uint16 size = sizeof(bitmapHeader);
	memory += 14;
	bitmapInfoHeader = (file_bitmap_info_header*)memory;
	
	bitmap.bitmapHeader = bitmapHeader;
	bitmap.bitmapInfoHeader = bitmapInfoHeader;
	
	return bitmap;	
}

internal void BMPToTexture(completeFile *file, texture *sprite) {
	
	// ------- HEADER : 40 bytes
	file_bitmap bitmap = parseBMP(file);
	
	// memory 
	uint8 *memory = (uint8*)file->memory;
	
	// base int is OK bc 4 bytes long
	sprite->width = bitmap.bitmapInfoHeader->width;
	sprite->height = bitmap.bitmapInfoHeader->height;
	sprite->memorySize = bitmap.bitmapHeader->size - (sizeof(bitmap.bitmapHeader) + sizeof(bitmap.bitmapInfoHeader));
	sprite->bytesPerPixel = bitmap.bitmapInfoHeader->bpp;
	
	// setting the memory to the texture struct
	sprite->memory = memory + bitmap.bitmapHeader->offset;
}	

#endif /* _PARSERH_ */