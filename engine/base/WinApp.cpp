#include "WinApp.h"

#pragma comment(lib,"winmm.lib")

LRESULT WinApp::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
#ifdef _DEBUG

	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
	{
		return true;
	}

#endif

	switch (msg)
	{
	case WM_DESTROY:

		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

void WinApp::Initialize()
{
	// システムタイマー分解能を上げる
	timeBeginPeriod(1);

	HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);

	wc.lpfnWndProc = WindowProc;

	wc.lpszClassName = L"CG2WindowClass";

	wc.hInstance = GetModuleHandle(nullptr);

	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

	RegisterClass(&wc);

	//ウィンドウサイズを表す構造体にクライアント領域を入れる
	RECT wrc = { 0,0,kClientWidth,kClientHeight };

	//クライアント領域を元にwrcを変更してもらう
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	hwnd = CreateWindow(wc.lpszClassName, L"3124_TiN", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, wrc.right - wrc.left, wrc.bottom - wrc.top, nullptr, nullptr, wc.hInstance, nullptr);

	ShowWindow(hwnd, SW_SHOW);
}

void WinApp::Finalize()
{
	CloseWindow(hwnd);
	CoUninitialize();
}

bool WinApp::ProcessMessage()
{
	MSG msg{};

	if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (msg.message == WM_QUIT)
	{
		return true;
	}

	return false;
}
