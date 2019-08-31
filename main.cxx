#include <stdio.h>
#include <windows.h>

HWND hWnd;

static inline bool setup_raw_input(HWND hWnd)
{
	RAWINPUTDEVICE rid;

	rid.usUsagePage = 0x01;
	rid.usUsage = 0x02;
	rid.dwFlags = RIDEV_NOLEGACY | RIDEV_INPUTSINK;
	rid.hwndTarget = hWnd;

	return !!RegisterRawInputDevices(&rid, 1, sizeof rid);
}

static inline char const* rim_type_to_string(DWORD type)
{
	switch (type)
	{
		case RIM_TYPEMOUSE:
			return "mouse";
		case RIM_TYPEKEYBOARD:
			return "keyboard";
		case RIM_TYPEHID:
			return "hid";
	}
	return 0;
}

static inline bool handle_raw_input_2(RAWINPUT const& raw)
{
	char const* type_str = rim_type_to_string(raw.header.dwType);
	if (!type_str)
		return false;

	printf("header.dwType: %s\n"
	       "header.dwSize: %u\n",
	       (char const*)type_str,
	       (unsigned int)raw.header.dwSize);

	if (raw.header.dwType == RIM_TYPEMOUSE)
		printf("usFlags: %ui X: %li Y: %li Extra: %uli\n",
		       (unsigned int)raw.data.mouse.usFlags,
		       (long int)raw.data.mouse.lLastX,
		       (long int)raw.data.mouse.lLastY,
		       (unsigned long int)raw.data.mouse.ulExtraInformation);

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

	bool ret = handle_raw_input_2(*(RAWINPUT*)lpb);
	delete[] lpb;
	return ret;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
#ifndef MESSAGE_ONLY
		case WM_SHOWWINDOW:
			SetActiveWindow(hWnd);
			break;
#endif
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

#if 0 // Does not work under Wine, did not test on Windows.
BOOL WINAPI ctrl_handler(DWORD fdwCtrlType)
{
	CloseWindow(hWnd);
	DestroyWindow(hWnd);
	return TRUE;
}
#endif

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	WNDCLASSEX wx = {};
	wx.cbSize = sizeof(WNDCLASSEX);
	wx.lpfnWndProc = WndProc;
	wx.hInstance = hInstance;
	wx.lpszClassName = "rawinput-test-class";
	if (!RegisterClassEx(&wx))
		return 1;

#ifdef MESSAGE_ONLY
	hWnd = CreateWindowEx(0, wx.lpszClassName, NULL, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, hInstance, NULL);
#else
	hWnd = CreateWindowEx(0, wx.lpszClassName, "rawinput-test", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, 300, 300, NULL, NULL, hInstance, NULL);
#endif
	if (hWnd == NULL)
		return 2;

#if 0
	SetConsoleCtrlHandler(ctrl_handler, TRUE);
#endif

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
