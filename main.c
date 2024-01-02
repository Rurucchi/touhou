#include <windows.h>
#include <WinUser.h>
#include <wingdi.h>

#define UNICODE
#define _UNICODE

#define internal static
#define local_persist static
#define global_variable static

global_variable int running;

internal void ResizeDIBDSection(int width, int height){
	
}

LRESULT CALLBACK MainWindowCallback(
	HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam
) {
	LRESULT Result = 0;


	switch (uMsg) {
	case WM_SIZE: {
		// resize window
		RECT ClientRect;
		int width = ClientRect.rcPaint.right - ClientRect.rcPaint.left;
		int height = ClientRect.rcPaint.bottom - ClientRect.rcPaint.top;
		
		int GetClientRect(hWnd, &ClientRect);
		ResizeDIBDSection();
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
		
		//white window
		local_persist DWORD ROP = WHITENESS;
		
		PatBlt(DC, X, Y, width, height, ROP);
		
		EndPaint(hWnd, &Paint);
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
	WindowClass.lpfnWndProc = MainWindowCallback;
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