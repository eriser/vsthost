#ifndef HOSTGUI_H
#define HOSTGUI_H
#include <Windows.h>
#include "resource.h"
#include <iostream>

class Host;

class HostGUI {
public:
	HINSTANCE instance;
	HWND wnd;
	RECT rect;
	WNDCLASS wc;
	Host &host;
	HostGUI(Host &h) : host(h) {}
	
	void dupa() {
		std::cout << "dupa" << std::endl;
	}
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
		wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1) ;
		wc.lpszMenuName = NULL;
		wc.lpszClassName = TEXT("HostGUI");
		if (!RegisterClass(&wc)) MessageBoxA(NULL, (LPCSTR)"DUPA", (LPCSTR)appname, MB_ICONERROR);
		wnd = CreateDialogParamA(instance, MAKEINTRESOURCEA(IDD_HOST), 0, AboutDlgProc, (LPARAM)this);
		if (wnd) {
			SendMessageA(wnd, WM_USER + 1, NULL, (LPARAM)this);
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
	static BOOL CALLBACK AboutDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		std::cout << " proccccccc" << std::endl;
		HostGUI *editor = NULL;
		if (uMsg == WM_USER + 1) {//WM_INITDIALOG
			editor = (HostGUI*)((LPCREATESTRUCT)lParam)->lpCreateParams;
			std::cout << (int)editor << std::endl;
			SetLastError(0);
			auto info = SetWindowLongPtr(hWnd, GWL_USERDATA, (LONG_PTR)editor);
			if (info == 0) std::cout << GetLastError() << std::endl;
			//else std::cout << info << std::endl;
		}
		else editor = (HostGUI*)GetWindowLongPtr(hWnd, GWL_USERDATA);
		
		//std::cout << (int)editor << std::endl;
		if (!editor) return DefWindowProc(hWnd, uMsg, wParam, lParam);
		else return editor->WindowProc(hWnd, uMsg, wParam, lParam);
	}
	static LRESULT CALLBACK Wrapper(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		
		HostGUI *editor = NULL;
		if (uMsg == WM_USER + 1) {
			editor = (HostGUI*)((LPCREATESTRUCT)lParam)->lpCreateParams;
			std::cout << (int)editor << std::endl;
			SetLastError(0);
			auto info = SetWindowLongPtr(hWnd, GWL_USERDATA, (LONG_PTR)editor);
			if (info == 0) std::cout << GetLastError() << std::endl;
			//else std::cout << info << std::endl;
		}
		else editor = (HostGUI*)GetWindowLongPtr(hWnd, GWL_USERDATA);
		editor->dupa();
		std::cout << (void *)editor << std::endl;
		if (!editor) return DefWindowProc(hWnd, uMsg, wParam, lParam);
		else return editor->WindowProc(hWnd, uMsg, wParam, lParam);
	}
	LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

};

#endif