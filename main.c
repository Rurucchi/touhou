#include <windows.h>
#include <WinUser.h>
#include <wingdi.h>
#include <stdint.h>

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

// states
global_variable int running;

// bitmap stuff
global_variable BITMAPINFO BitMapInfo;
global_variable	void *BitMapMemory;
global_variable int BitMapWidth;
global_variable int BitMapHeight;
global_variable	int bytesPerPixel = 4;

internal void RenderGradient(int xOffset, int yOffset){
	int width = BitMapWidth;
	int height = BitMapHeight;
	
	int offset = width*bytesPerPixel;
	uint8 *row = (uint8 *)BitMapMemory;
	
	// coloring each pixel of the rectangle
	for(int y = 0; y < BitMapHeight; ++y){
		
			uint8 *pixel = (uint8*)row;
			for(int x = 0; x < BitMapWidth; ++x){
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

internal void Win32ResizeDIBDSection(int width, int height) {
	
	// free memory if already exists
	if(BitMapMemory){
		VirtualFree(BitMapMemory, 0, MEM_RELEASE);
	}
	
	// set bitmap sizes to drawable area size
	BitMapWidth = width;
	BitMapHeight = height;
	
	// create (or edit cuz declared already) the rectangle we will draw in
	BitMapInfo.bmiHeader.biSize = sizeof(BitMapInfo.bmiHeader);
	BitMapInfo.bmiHeader.biWidth = BitMapWidth;
	BitMapInfo.bmiHeader.biHeight = -BitMapHeight;
	BitMapInfo.bmiHeader.biPlanes = 1;
	BitMapInfo.bmiHeader.biBitCount = 32;
	BitMapInfo.bmiHeader.biCompression = BI_RGB;
	
	int bitMapMemorySize = (BitMapWidth * BitMapHeight) * bytesPerPixel;
	BitMapMemory = VirtualAlloc(0, bitMapMemorySize, MEM_COMMIT, PAGE_READWRITE);
};

internal void Win32UpdateWindow(HDC DeviceContext, RECT *WindowRect, int x, int y, int width, int height) {
	int windowWidth = WindowRect->right - WindowRect->left;
	int windowHeight = WindowRect->bottom - WindowRect->top;
	
	StretchDIBits(DeviceContext, 
	0, 0, BitMapWidth, BitMapHeight, 
	0, 0, windowWidth, windowHeight, 
	BitMapMemory,
	&BitMapInfo,
  DIB_RGB_COLORS, SRCCOPY);
}

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
		
		Win32ResizeDIBDSection(width, height);
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
		Win32UpdateWindow(DC, &ClientRect, X, Y, width, height);
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
				
				RenderGradient(xOffset, yOffset);
				Win32UpdateWindow(DC, &ClientRect, 0, 0, windowWidth, windowHeight);
				ReleaseDC(WindowHandle, DC);
				
				++xOffset;
			}
		}

		return 0;
	}
	else {
		return 0;
	};
};