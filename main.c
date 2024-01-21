#include <windows.h>
#include <WinUser.h>
#include <wingdi.h>
#include <stdint.h>
#include <synchapi.h>

#define UNICODE
#define _UNICODE

#define internal static
#define local_persist static
#define global_variable static

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

// bitmap stuff
typedef struct win32_offscreen_buffer {
	BITMAPINFO info;
	void *memory;
	int width;
	int height;
	int bytesPerPixel;
} win32_offscreen_buffer;

typedef struct win32_rect {
	RECT rectangle;
	int top;
	int bottom;
	int left;
	int right;
	int width;
	int height;
} win32_rect;

// states
global_variable int running;
global_variable win32_offscreen_buffer GlobalBackBuffer;

// rectangle

internal void Win32GetDrawableRect(HDC DeviceContext) {
	
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
				
				// offset
				*pixel = 0;
				++pixel;
		};
		
		row += offset;
	};
}

internal void 

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

internal void Win32UpdateWindow(HDC DeviceContext, 
								RECT *WindowRect, 
								win32_offscreen_buffer *buffer, 
								int x, int y, 
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

// messages from windows, interactions with the window by OS and User
LRESULT CALLBACK Win32MainWindowCallback(
	HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam
) {
	LRESULT Result = 0;


	switch (uMsg) {
	case WM_SIZE: {
		// resize window
		RECT ClientRect;
		GetClientRect(hWnd, &ClientRect);
		
		int width = ClientRect.right - ClientRect.left;
		int height = ClientRect.bottom - ClientRect.top;
		
		Win32ResizeDIBDSection(&GlobalBackBuffer, width, height);
		OutputDebugStringA("WM_SIZE\n");
		break;
	};
	case WM_DESTROY: {
		running = 0;
		OutputDebugStringA("WM_DESTROY\n");
		break;
	};
	case WM_CLOSE: {
		running = 0;
		OutputDebugStringA("WM_CLOSE\n");
		break;
	};
	case WM_ACTIVATEAPP: {
		GlobalBackBuffer.bytesPerPixel = 4;
		OutputDebugStringA("WM_ACTIVATEAPP\n");
		break;
	};
	
	case WM_PAINT : {
		PAINTSTRUCT Paint;
		HDC DC = BeginPaint(hWnd, &Paint);
		
		int X = Paint.rcPaint.left;
		int Y = Paint.rcPaint.top;
		int width = Paint.rcPaint.right - Paint.rcPaint.left;
		int height = Paint.rcPaint.bottom - Paint.rcPaint.top;
		
		RECT ClientRect;
		GetClientRect(hWnd, &ClientRect);
		Win32UpdateWindow(DC, &ClientRect, &GlobalBackBuffer, X, Y, width, height);
		break;
	};

	default: {
		OutputDebugStringA("defaut\n");
		Result = DefWindowProcA(hWnd, uMsg, wParam, lParam);
		break;
	};

	}
	return (Result);
};


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
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			0,
			0,
			hInstance,
			0
		);
		if (WindowHandle) {
			MSG Msg;
			running = 1;
			
			int xOffset = 0;
			int yOffset = 0;
			
				GlobalBackBuffer.bytesPerPixel = 4;
			
				RECT ClientRect;
				GetClientRect(WindowHandle, &ClientRect);
				int windowWidth = ClientRect.right - ClientRect.left;
				int windowHeight = ClientRect.bottom - ClientRect.top;
		
				Win32ResizeDIBDSection(&GlobalBackBuffer, windowWidth, windowHeight);
			
			while (running) {
				
				while(PeekMessageA(&Msg, 0, 0, 0, PM_REMOVE)) {
					
					if(Msg.message == WM_QUIT){
						running = 0;
					}
					
					TranslateMessage(&Msg);
					DispatchMessage(&Msg);
				}
				
				HDC DC = GetDC(WindowHandle);
				RECT ClientRect;
				GetClientRect(WindowHandle, &ClientRect);
				int windowWidth = ClientRect.right - ClientRect.left;
				int windowHeight = ClientRect.bottom - ClientRect.top;
				
				RenderGradient(&GlobalBackBuffer, xOffset, yOffset);
				Win32UpdateWindow(DC, &ClientRect, &GlobalBackBuffer, 0, 0, windowWidth, windowHeight);
				ReleaseDC(WindowHandle, DC);
				
				++xOffset;
				
				// NOTE(wuwi) : limit FPS and game ticks
				// (144 for now, might want to increase to 165 or 240, depends of performance.)
				Sleep(7);
			}
		}

		return 0;
	}
	else {
		return 0;
	};
};