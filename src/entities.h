#ifndef _ENTITIESH_
#define _ENTITIESH_

#include <windows.h>
#include "./types.h"

#define internal static
#define local_persist static
#define global_variable static

#define Clamp(value, low, high) ((value) < (high)) ? (((value) > (low)) ? (value) : (low)) : (high)

// LOCAL DEPENDENCIES : TYPES.H

//  ----------------------------------- GENERAL ENTITIES

typedef struct entity {
	uint zIndex;
	POINT center;
	float top;
	float left;
	float width;
	float height;
	texture sprite;
	int visibility;
} entity;

typedef struct player {
	entity entity;
	uint size;
	int health;
} player;

#endif /* _ENTITIESH_ */