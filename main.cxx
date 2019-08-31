#include <stdio.h>
#include <windows.h>

bool setup_raw_input(HWND hWnd)
{
	RAWINPUTDEVICE rid[1];

	rid[0].usUsagePage = 0x01;
	rid[0].usUsage = 0x02;
	rid[0].dwFlags = RIDEV_NOLEGACY | RIDEV_INPUTSINK;
	rid[0].hwndTarget = hWnd;

	return !!RegisterRawInputDevices(rid, sizeof rid / sizeof rid[0], sizeof rid[0]);
}

bool _handle_raw_input(RAWINPUT* raw)
{
	const char* type_str;
	switch (raw->header.dwType)
	{
		case RIM_TYPEMOUSE:
			type_str = "mouse";
			break;
		case RIM_TYPEKEYBOARD:
			type_str = "keyboard";
			break;
		case RIM_TYPEHID:
			type_str = "hid";
			break;
		default:
			return false;
	}
	printf("header.dwType: %s\n", type_str);
	printf("header.dwSize: %u\n", raw->header.dwSize);
	printf("sizeof(RAWINPUT): %llu\n", sizeof(RAWINPUT));
	if (raw->header.dwType == RIM_TYPEMOUSE)
	{
		printf("usFlags: %ui ", (unsigned)raw->data.mouse.usFlags);
		printf("X: %li Y: %li E: %uli\n", raw->data.mouse.lLastX, raw->data.mouse.lLastY, raw->data.mouse.ulExtraInformation);
	}
	return true;
}

bool handle_raw_input(HRAWINPUT hRawInput)
{
	UINT dwSize;
	if (GetRawInputData(hRawInput, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER)) != 0)
		return false;
	LPBYTE lpb = new BYTE[dwSize];
	if (!lpb)
		return false;
	if (GetRawInputData(hRawInput, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
	{
		delete[] lpb;
		return false;
	}

	bool ret = _handle_raw_input((RAWINPUT*)lpb);
	delete[] lpb;
	return ret;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_SHOWWINDOW:
			SetActiveWindow(hWnd);
			break;
		case WM_INPUT:
			if (!handle_raw_input((HRAWINPUT)lParam))
				PostQuitMessage(4);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	WNDCLASSEX wx = {};
	wx.cbSize = sizeof(WNDCLASSEX);
	wx.lpfnWndProc = WndProc;
	wx.hInstance = hInstance;
	wx.lpszClassName = "dummy";
	if (!RegisterClassEx(&wx))
		return 1;

#ifdef MESSAGE_ONLY
	HWND hWnd = CreateWindowEx(0, wx.lpszClassName, NULL, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, NULL);
#else
	HWND hWnd = CreateWindowEx(0, wx.lpszClassName, "rawinput-test", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, 300, 300, NULL, NULL, hInstance, NULL);
#endif
	if (hWnd == NULL)
		return 2;

	if (!setup_raw_input(hWnd))
		return 3;

	MSG msg;
	BOOL bRet;
	while ((bRet = GetMessage(&msg, NULL, 0, 0)) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}
