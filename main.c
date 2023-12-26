#include <windows.h>
#include <WinUser.h>

LRESULT CALLBACK MainWindowCallback(
	HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam
) {
	LRESULT Result = 0;

	switch (uMsg) {
	case WM_SIZE: {
		OutputDevugStringA("WM_SIZE\n")
		break;
	};
	case WM_DESTROY: {
		OutputDevugStringA("WM_DESTROY\n")
		break;
	};
	case WM_CLOSE: {
		OutputDevugStringA("WM_CLOSE\n")
		break;
	};
	case WM_ACTIVATEAPP: {
		OutputDevugStringA("WM_ACTIVATEAPP\n")
		break;
	};
	default: {
		OutputDevugStringA("defaut\n")
		break;
	};
	}
}
;


int CALLBACK WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	struct WNDCLASSEXA WindowClass = { 0 };

	WindowClass.cbSize = sizeof(WNDCLASSEX);
	WindowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	WindowClass.lpfnWndProc = MainWindowCallback;
	WindowClass.hInstance = hInstance;

	return 0;
}