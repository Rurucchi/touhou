#ifndef _PLATFORMH_
#define _PLATFORMH_

#include <windows.h>
#include "./types.h"

#define internal static
#define local_persist static
#define global_variable static

#define Clamp(value, low, high) ((value) < (high)) ? (((value) > (low)) ? (value) : (low)) : (high)

// LOCAL DEPENDENCIES : TYPES.H

//  ------------------------------------ GENERAL ENTITIES

void ReadFullFile(char *Location, completeFile *File){
	HANDLE rawFile = CreateFileA(Location, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	
	//check the handle
    if (rawFile == INVALID_HANDLE_VALUE)
    {
		OutputDebugStringA("FILE OPENING ERROR\n");
    } else {
		LARGE_INTEGER fileSize;
	
		GetFileSizeEx(rawFile,
		  &fileSize
		);
	
		uint32 fileSize32 = SafeTruncateUInt64(fileSize.QuadPart);
		File->memory = VirtualAlloc(0, fileSize32, MEM_COMMIT, PAGE_READWRITE);
		ReadFile(rawFile, File->memory, fileSize32, NULL, NULL);
		File->size = fileSize32;
	}

}

#endif /* _PLATFORMH_ */