#ifndef _RENDERH_
#define _RENDERH_

#include <windows.h>
#include "./types.h"

#define internal static
#define local_persist static
#define global_variable static

#define Clamp(value, low, high) ((value) < (high)) ? (((value) > (low)) ? (value) : (low)) : (high)

// LOCAL DEPENDENCIES : TYPES.H

//  ----------------------------------- GENERAL RENDERING


// for texture 
internal void AbsoluteToRelative(POINT *absoluteCoords, POINT *relativeCoords, int maxX, int maxY){
	relativeCoords->x = absoluteCoords->x / maxX;
	relativeCoords->y = absoluteCoords->y / maxY;
}

internal void RelativeToAbsolute(POINT *relativeCoords, POINT *absoluteCoords, int maxX, int maxY){
	absoluteCoords->x = relativeCoords->x * maxX;
	absoluteCoords->y = relativeCoords->y * maxY;
}

internal void RenderGradient(win32_offscreen_buffer *buffer, int xOffset, int yOffset){
	int width = buffer->width;
	int height = buffer->height;
	
	int offset = width*buffer->bytesPerPixel;
	uint8 *row = (uint8 *)buffer->memory;
	
	// coloring each pixel of the rectangle
	for(int y = 0; y < buffer->height; ++y){
		
			uint8 *pixel = (uint8*)row;
			for(int x = 0; x < buffer->width; ++x){
				// /!\ LITTLE ENDIAN ARCH.! MEMORY INDEXES ARE INVERTED!
				
				// blue
				*pixel = (uint8)(x + xOffset);
				++pixel;
				
				// green
				*pixel = 0;
				++pixel;
				
				// red
				*pixel = (uint8)(y + yOffset);
				++pixel;
				
				// offset (for memory allignment)
				*pixel = 0;
				++pixel;
			};
		
		row += offset;
	};
}

internal void RenderEntity(win32_offscreen_buffer *buffer, entity *Entity, virtual_game_size *VirtualGameSize){
	// in here, we do use virtualgamesize btw
	
	int width = buffer->width;
	int height = buffer->height;
	
	int processedWidth = (width * Entity->width) / VirtualGameSize->horizontal;
	int processedHeight = (height * Entity->height) / VirtualGameSize->vertical;
	int processedLeft = ((width * Entity->left) / VirtualGameSize->horizontal) - processedWidth;
	int processedTop = ((height * Entity->top) / VirtualGameSize->vertical) - processedHeight;

	
	int verticalOffset = width*buffer->bytesPerPixel;
	
	POINT currentIndex = {0, 0};
	
	// texture coords (parsing)
	POINT relativeCoords = {0};
	POINT absoluteCoords = {0};
	
	int row = width*buffer->bytesPerPixel;
	uint8 *area = (uint8 *)buffer->memory;
	area += (int)row * processedTop;
	area += processedLeft*buffer->bytesPerPixel;
	
	// drawing 
	
	for(int y = 0; y < processedHeight; ++y){
		uint8 *pixelCursor = (uint8*)area;
		
		for(int x = 0; x < processedWidth; ++x){
			
			
			// TODO:(ru): this looks janky af but idc no ones gonna read it anyway
			
			// --- texture sampling
			
			AbsoluteToRelative(&currentIndex, &relativeCoords, Entity->sprite.width, Entity->sprite.height);
			RelativeToAbsolute(&relativeCoords, &absoluteCoords, Entity->sprite.width, Entity->sprite.height);
			
			// accessing texture memory
			
			uint32 *textureCursor = Entity->sprite.memory ;
			
			uint8 spritePixel = *(uint32*)textureCursor + (32 * Entity->sprite.width * absoluteCoords.y) + absoluteCoords.x;
			
			
			// writing pixel
			
			// /!\ LITTLE ENDIAN ARCH.! MEMORY INDEXES ARE INVERTED!
				
			// blue
			*pixelCursor = spritePixel;
			++pixelCursor;
			spritePixel += 8;
				
			// green
			*pixelCursor = spritePixel;
			++pixelCursor;
			spritePixel += 8;
				
			// red
			*pixelCursor = spritePixel;
			++pixelCursor;
				
			// offset (for memory allignment)
			*pixelCursor = 0;
			++pixelCursor;
			
			textureCursor++;
			currentIndex.x ++;
			};
			
		currentIndex.y ++;
		area += row;
	};
}

#endif /* _RENDERH_ */