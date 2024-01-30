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

typedef unsigned int uint;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;



// ------- CUSTOM STRUCTS AND TYPEDEFS

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



//	-------	GLOBAL VARIABLES
global_variable int running;
global_variable win32_offscreen_buffer GlobalBackBuffer;
global_variable POINT MousePos;



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
				
				// offset
				*pixel = 0;
				++pixel;
		};
		
		row += offset;
	};
}



//	-------	INPUT HANDLING

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

internal void FilterInputs(LPARAM lParam) {
	HRESULT hResult;
	uint dwSize;

	GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
    uint8 lpb[256];
	
    if (lpb == NULL) 
    {
		OutputDebugStringA("lpb is null\n");
		return;
    } 

    if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
         OutputDebugString (TEXT("GetRawInputData does not return correct size !\n")); 

    RAWINPUT* raw = (RAWINPUT*)lpb;

    if (raw->header.dwType == RIM_TYPEKEYBOARD) 
    {
		/*
        hResult = StringCchPrintf(szTempOutput, STRSAFE_MAX_CCH,
            TEXT(" Kbd: make=%04x Flags:%04x Reserved:%04x ExtraInformation:%08x, msg=%04x VK=%04x \n"), 
            raw->data.keyboard.MakeCode, 
            raw->data.keyboard.Flags, 
            raw->data.keyboard.Reserved, 
            raw->data.keyboard.ExtraInformation, 
            raw->data.keyboard.Message, 
            raw->data.keyboard.VKey);
        if (FAILED(hResult))
        {
        // TODO: write error handler
        }
		*/
        OutputDebugStringA("keyboard\n");
    }
	
	
	// ↓ NOTE(ru): this code is deprecated for now, using get GetCursorPos, keeping the code in case extra infos would be needed
	
	/*
    else if (raw->header.dwType == RIM_TYPEMOUSE) 
    {
		
        hResult = StringCchPrintf(szTempOutput, STRSAFE_MAX_CCH,
            TEXT("Mouse: usFlags=%04x ulButtons=%04x usButtonFlags=%04x usButtonData=%04x ulRawButtons=%04x lLastX=%04x lLastY=%04x ulExtraInformation=%04x\r\n"), 
            raw->data.mouse.usFlags, 
            raw->data.mouse.ulButtons, 
            raw->data.mouse.usButtonFlags, 
            raw->data.mouse.usButtonData, 
            raw->data.mouse.ulRawButtons, 
            raw->data.mouse.lLastX, 
            raw->data.mouse.lLastY, 
            raw->data.mouse.ulExtraInformation);

        if (FAILED(hResult))
        {
        // TODO: write error handler
        }
		
        OutputDebugStringA("mouse\n");
    } 	
	*/
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


// ---	MESSAGES PROCESSING
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
		case WM_INPUT: {
			// FilterInputs(lParam);
			break;
		};
		
		/*
		
		// TODO(ru): very later on, pause the game when cursor is out of window
		// (mouse should be kept inside window anyway but alt tab and windows key will not be enforced.)
		case WM_MOUSELEAVE:
		{
			// TODO(ru): use this -> https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-trackmouseevent
			// this too -> https://stackoverflow.com/questions/68021291/how-to-detect-mouse-cursor-is-outside-a-windows
			// NOTE(ru): ^ this may or may not be useful? It's possible to know if the mouse is inside the app just by 
			// doing some calculation with both the rect and mouses absolute coordinates to make relative ones
			// ^ (might possibly be faster than calling an event and stuff)
			
		}
		
		*/
		
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
			1600,
			900,
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
			
			
			// FIRST RUN (RENDERING)
			{
				GlobalBackBuffer.bytesPerPixel = 4;
				win32_rect clientRect = Win32GetDrawableRect(WindowHandle);
				Win32ResizeDIBDSection(&GlobalBackBuffer, clientRect.width, clientRect.height);
			}

			
			// FIRST RUN (REGISTER DEVICES
			{
				RAWINPUTDEVICE RID[2];
				// ListenToDevices(RID);
				
				// if (RegisterRawInputDevices(RID, 2, sizeof(RID[0])) == FALSE){
				// 	OutputDebugStringA("registration failed. Call GetLastError for the cause of the error\n");
				// }
			}
			
			
			
			// GAME LOOP :
			while (running) {
				
				// leaving that here for now, it's basically used everywhere anyways
				HDC DC = GetDC(WindowHandle);
				win32_rect clientRect = Win32GetDrawableRect(WindowHandle);
				
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
				

				
				// RENDERING 
				{
					RenderGradient(&GlobalBackBuffer, xOffset, yOffset);
					Win32UpdateWindow(DC, &clientRect.rectangle, &GlobalBackBuffer, clientRect.width, clientRect.height);
					ReleaseDC(WindowHandle, DC);
					
					++xOffset;
				}
				
				// INPUTS
				{
					GetCursorPos(&MousePos);
					// TODO(ru): fix this
					if((MousePos.x > clientRect.rectangle.left && MousePos.x < clientRect.rectangle.right) && (MousePos.y < clientRect.rectangle.top && MousePos.y > clientRect.rectangle.bottom)){
						OutputDebugStringA("inside window\n");
					} else {
						OutputDebugStringA("not inside window\n");
					}
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