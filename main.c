#include <windows.h>
#include <WinUser.h>
#include <wingdi.h>
#include <stdint.h>
#include <synchapi.h>
#include <stdio.h>

#define UNICODE
#define _UNICODE

#define internal static
#define local_persist static
#define global_variable static

typedef unsigned int uint;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

// playable area : 255 * 5 = 1275px, to scale.


// --------------------------------------- CUSTOM STRUCTS AND TYPEDEFS


// -------------- GAME LOGIC

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


// -------------- ENTIIES
typedef struct entity {
	uint zIndex;
	float top;
	float left;
	float width;
	float height;
	int visibility;
} entity;

typedef struct player {
	entity entity;
	int health;
} player;


// --- RENDERING STRUCTS

// bitmap stuff
typedef struct win32_offscreen_buffer {
	BITMAPINFO info;
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
	BITMAPINFO info;
	void *memory;
	int width;
	int height;
	int bytesPerPixel;
} texture;


//	-------	GLOBAL VARIABLES
global_variable int running;
global_variable game_state GameState;
global_variable win32_offscreen_buffer GlobalBackBuffer;
global_variable virtual_game_size VirtualGameSize;

//	-------	RENDERING

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

internal void RenderPlayer(win32_offscreen_buffer *buffer, POINT *MousePos, player *Player) {
	int width = buffer->width;
	int height = buffer->height;
	
	// since the buffer's Y coordinates are inverted compared the mousepos, it's better to invert it.
	int invertedY = height-MousePos->y;
	
	int offset = width*buffer->bytesPerPixel;
	uint8 *row = (uint8 *)buffer->memory;
	
	row += offset*invertedY;
	row += buffer->bytesPerPixel*MousePos->x;
	
	uint8 *pixel = (uint8 *)row;
	
	// blue
	*pixel = 255;
	++pixel;
				
	// green
	*pixel = 255;
	++pixel;
				
	// red
	*pixel = 255;
	++pixel;
				
	// offset (for memory alignment)
	*pixel = 0;
	++pixel;
}

internal void scaleTexture(win32_offscreen_buffer *buffer) {
	
}


// this function updates send the pixels to windows to draw the buffer
internal void Win32UpdateWindow(HDC DeviceContext, 
								RECT *WindowRect, 
								win32_offscreen_buffer *buffer, 
								int width, 
								int height) 
{
									
	int windowWidth = WindowRect->right - WindowRect->left;
	int windowHeight = WindowRect->bottom - WindowRect->top;

	StretchDIBits(DeviceContext, 
	0, 0, buffer->width, buffer->height, 
	0, 0, windowWidth, windowHeight, 
	buffer->memory,
	&buffer->info,
  DIB_RGB_COLORS, SRCCOPY);
}

internal void ListenToDevices(RAWINPUTDEVICE RID[2]) {
        
	RID[0].usUsagePage = 0x01;          // HID_USAGE_PAGE_GENERIC
	RID[0].usUsage = 0x02;              // HID_USAGE_GENERIC_MOUSE
	RID[0].dwFlags = RIDEV_NOLEGACY;    // adds mouse and also ignores legacy mouse messages
	RID[0].hwndTarget = 0;

	RID[1].usUsagePage = 0x01;          // HID_USAGE_PAGE_GENERIC
	RID[1].usUsage = 0x06;              // HID_USAGE_GENERIC_KEYBOARD
	RID[1].dwFlags = RIDEV_NOLEGACY;    // adds keyboard and also ignores legacy keyboard messages
	RID[1].hwndTarget = 0;
}



//	------- WIN32 STUFF

internal win32_rect Win32GetDrawableRect(HWND windowHandler) {
	win32_rect drawableRect;
	GetClientRect(windowHandler, &drawableRect.rectangle);
	drawableRect.width 	= drawableRect.rectangle.right - drawableRect.rectangle.left;
	drawableRect.height	= drawableRect.rectangle.bottom - drawableRect.rectangle.top;
	
	return drawableRect;
}

internal void Win32ResizeDIBDSection(win32_offscreen_buffer *buffer, int width, int height) {
	
	// free memory if already exists
	if(buffer->memory){
		VirtualFree(buffer->memory, 0, MEM_RELEASE);
	}
	
	// set bitmap sizes to drawable area size
	buffer->width = width;
	buffer->height = height;
	
	// create (or edit cuz declared already) the rectangle we will draw in
	buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
	buffer->info.bmiHeader.biWidth = buffer->width;
	buffer->info.bmiHeader.biHeight = buffer->height;
	buffer->info.bmiHeader.biPlanes = 1;
	buffer->info.bmiHeader.biBitCount = 32;
	buffer->info.bmiHeader.biCompression = BI_RGB;
	
	int bitMapMemorySize = (buffer->width * buffer->height) * buffer->bytesPerPixel;
	buffer->memory = VirtualAlloc(0, bitMapMemorySize, MEM_COMMIT, PAGE_READWRITE);
};

//	------- INPUT PROCESSING 

internal void HandleFunctionKeystrokes(WPARAM wParam) {

	switch (wParam) 
	{ 
		case VK_ESCAPE: {
			OutputDebugStringA("Game paused?\n");
			// flips between 0 and 1
			GameState.pause = 1 - GameState.pause;
			break;
		}

		default: {
			// Process other non-character keystrokes. 
			break; 
		}
	}
}


// ---	MESSAGES PROCESSING 			/!\ THIS RUNS ON ANOTHER THREAD THAN THE GAME LOOP. 

LRESULT CALLBACK Win32MainWindowCallback(
	HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam
) {
	LRESULT Result = 0;


	switch (uMsg) {
		
		//	WINDOW RESIZE 
		case WM_SIZE: {
					
			win32_rect clientRect = Win32GetDrawableRect(hWnd);
			Win32ResizeDIBDSection(&GlobalBackBuffer, clientRect.width, clientRect.height);
			break;
		};
		
		//	WINDOW IS DESTROYED (process killed)
		case WM_DESTROY: {
			running = 0;
			OutputDebugStringA("DESTROY\n");
			break;
		};
		
		//	WINDOW IS CLOSED BY THE USER
		case WM_CLOSE: {
			running = 0;
			OutputDebugStringA("CLOSE\n");
			break;
		};
		
		//	WINDOW IS CREATED
		case WM_ACTIVATEAPP: {
			GlobalBackBuffer.bytesPerPixel = 4;
			OutputDebugStringA("WM_ACTIVATEAPP\n");
			break;
		};
		
		//	ON RESIZE : RENDER AGAIN
		case WM_PAINT : {
			
		PAINTSTRUCT Paint;
		HDC DC = BeginPaint(hWnd, &Paint);
		
		int X = Paint.rcPaint.left;
		int Y = Paint.rcPaint.top;
		int width = Paint.rcPaint.right - Paint.rcPaint.left;
		int height = Paint.rcPaint.bottom - Paint.rcPaint.top;
		
		RECT ClientRect;
		GetClientRect(hWnd, &ClientRect);
		Win32UpdateWindow(DC, &ClientRect, &GlobalBackBuffer, width, height);
		break;
		};
		
		//	INPUT PROCESSING
		case WM_KEYDOWN: {
			HandleFunctionKeystrokes(wParam);
			break;
		}
		
		default: {
			Result = DefWindowProcA(hWnd, uMsg, wParam, lParam);
			break;
		};
	};
	return (Result);
};



//  ------- ENTRY POINT

int CALLBACK WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{

	WNDCLASSEXA WindowClass = { 0 };

	WindowClass.cbSize = sizeof(WNDCLASSEXA);
	WindowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	WindowClass.lpfnWndProc = Win32MainWindowCallback;
	WindowClass.hInstance = hInstance;
	WindowClass.lpszClassName = "TouhouWindowClass";

	if (RegisterClassExA(&WindowClass)) {
		HWND WindowHandle = CreateWindowExA(
			0,
			WindowClass.lpszClassName,
			"touhou",
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			600,
			450,
			0,
			0,
			hInstance,
			0
		);
		if (WindowHandle) {
			// Game loop status
			running = 1;
			
			// Messages
			MSG Msg;
			
			// Rendering offsets
			int xOffset = 0;
			int yOffset = 0;
			
			// FIRST RUN (GAME STATE)
			{
				GameState.level = 0;
				GameState. pause = 0;
				GameState.difficulty = 0;
				
				VirtualGameSize.horizontal = 1600;
				VirtualGameSize.vertical = 900;
			}
			
			
			// FIRST RUN (RENDERING)
			{
				GlobalBackBuffer.bytesPerPixel = 4;
				win32_rect clientRect = Win32GetDrawableRect(WindowHandle);
				Win32ResizeDIBDSection(&GlobalBackBuffer, clientRect.width, clientRect.height);
			}
			
			
			// todo(ru): cleanup this later
			// FIRST RUN (ENTITIES)
			player Player;
			Player.health = 100;
			
			
			
			// GAME LOOP :
			while (running) {
				
				// leaving that here for now, it's basically used everywhere anyways
				HDC DC = GetDC(WindowHandle);
				win32_rect clientRect = Win32GetDrawableRect(WindowHandle);
				int isInWindow = 0;
				POINT MousePos;
				
				// MESSAGE PROCESSING
				{
					while(PeekMessageA(&Msg, 0, 0, 0, PM_REMOVE)) {
						
						if(Msg.message == WM_QUIT){
							running = 0;
						}
						
						if(Msg.message == WM_INPUT) {
							// FilterInputs(Msg.lParam);
						}
						
						TranslateMessage(&Msg);
						DispatchMessage(&Msg);
					}
				}
				
				
				// INPUTS
				{
					GetCursorPos(&MousePos);
					// TODO(ru): fix this
					ScreenToClient(WindowHandle, &MousePos);
					char buffer[256];
					_snprintf_s(buffer, sizeof(buffer), sizeof(buffer), "Pos: %d,%d\n", MousePos.x, MousePos.y);
					 OutputDebugStringA(buffer);
					if((MousePos.x > 0 && MousePos.y > 0) && (MousePos.x < clientRect.width && MousePos.y < clientRect.height) ) {
						isInWindow = 1;
						// OutputDebugStringA("inside");
					} else {
						isInWindow = 0;
						// OutputDebugStringA("outside");
					}
				}
				
				// RENDERING 
				{
					if(!GameState.pause) {
						RenderGradient(&GlobalBackBuffer, xOffset, yOffset);
						++xOffset;
					}
					if(isInWindow){
						RenderPlayer(&GlobalBackBuffer, &MousePos, &Player);
					}
					Win32UpdateWindow(DC, &clientRect.rectangle, &GlobalBackBuffer, clientRect.width, clientRect.height);
					ReleaseDC(WindowHandle, DC);
				}

				// GAME SETTINGS AND LIMITATIONS
				{
					// NOTE(wuwi) : limit FPS and game ticks
					// (144 for now, might want to increase to 165 or 240, depends of performance.)
					Sleep(7);
				}
			}
		}

		return 0;
	}
	else {
		return 0;
	};
};