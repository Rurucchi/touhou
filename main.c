#include <windows.h>
#include <WinUser.h>
#include <wingdi.h>

#define UNICODE
#define _UNICODE

#define internal static
#define local_persist static
#define global_variable static

global_variable int running;
global_variable BITMAPINFO BitMapInfo;
global_variable	void *BitMapMemory;
global_variable HBITMAP BitMapHandle;
global_variable HDC BitMapDeviceContext;

internal void Win32ResizeDIBDSection(int width, int height) {

	if(BitMapHandle){
		DeleteObject(BitMapHandle);
	} 
	if(!BitMapDeviceContext){
		BitMapDeviceContext = CreateCompatibleDC(0);
	}

	// create (or edit cuz declared already) the rectangle we will draw in
	
	
	
	BitMapInfo.bmiHeader.biSize = sizeof(BitMapInfo.bmiHeader);
	BitMapInfo.bmiHeader.biWidth = width;
	BitMapInfo.bmiHeader.biHeight = height;
	BitMapInfo.bmiHeader.biPlanes = 1;
	BitMapInfo.bmiHeader.biBitCount = 32;
	BitMapInfo.bmiHeader.biCompression = BI_RGB;
	
	BitMapHandle = CreateDIBSection(
	BitMapDeviceContext,
	&BitMapInfo,
	DIB_RGB_COLORS,
	&BitMapMemory,
	0,
	0
);
}

internal void Win32UpdateWindow(HDC DeviceContext, int x, int y, int width, int height) {
	StretchDIBits(DeviceContext, x, y, width, height, x, y, width, height, 
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
		
		Win32UpdateWindow(DC, X, Y, width, height);
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
			while (running) {
				BOOL MsgResult = GetMessage(&Msg, 0, 0, 0);
				if (MsgResult > 0) {
					TranslateMessage(&Msg);
					DispatchMessage(&Msg);
				}
				else {
					break;
				}
			}
		}

		return 0;
	}
	else {
		return 0;
	};
};