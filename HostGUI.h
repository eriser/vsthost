#ifndef HOSTGUI_H
#define HOSTGUI_H
#include <Windows.h>
#include "vsthost_resource.h"
#include <iostream>

class Host;

class HostGUI {
public:
	void go() {
		instance = GetModuleHandle(NULL);
		TCHAR appname[] = TEXT("HostGUI");
		wc.style = 0;
		wc.lpfnWndProc = Wrapper;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = DLGWINDOWEXTRA;
		wc.hInstance = instance;
		wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)(COLOR_WINDOWFRAME);
		wc.lpszMenuName = NULL;
		wc.lpszClassName = TEXT("HostGUI");
		if (!RegisterClass(&wc)) MessageBoxA(NULL, (LPCSTR)"DUPA", (LPCSTR)appname, MB_ICONERROR);
		wnd = CreateDialogA(instance, (LPCSTR)IDD_DIALOG1, 0, NULL);
		if (wnd) {
			MSG Msg;
			ShowWindow(wnd, SW_SHOW);
			UpdateWindow(wnd);
			while (GetMessage(&Msg, NULL, 0, 0) > 0)
			{
				TranslateMessage(&Msg);
				DispatchMessage(&Msg);
			}
		}
		else
			std::cout << GetLastError() << std::endl;
	}
	static LRESULT CALLBACK Wrapper(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		HostGUI *editor = NULL;
		if (uMsg == WM_NCCREATE) {
			editor = (HostGUI*)((LPCREATESTRUCT)lParam)->lpCreateParams;
			SetWindowLongPtr(hWnd, GWL_USERDATA, (LONG_PTR)editor);
		}
		else editor = (HostGUI*)GetWindowLongPtr(hWnd, GWL_USERDATA);

		if (!editor) return DefWindowProc(hWnd, uMsg, wParam, lParam);
		else return editor->WindowProc(hWnd, uMsg, wParam, lParam);
	}
	virtual LRESULT CALLBACK WindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
		switch (Msg) {
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, Msg, wParam, lParam);
		}
		return 0;
	}
private:
	HINSTANCE instance;
	HWND wnd;
	RECT rect;
	WNDCLASS wc;
};

#endif