#include "HostGUI.h"
#include "Host.h"

#include <iostream>

const TCHAR* HostGUI::kClassName = TEXT("HostGUI");
const TCHAR* HostGUI::kCaption = TEXT("HostGUI");
const int HostGUI::kWindowWidth = 300;
const int HostGUI::kWindowHeight = 500;
const int HostGUI::kListWidth = 120;
const int HostGUI::kListHeight = 200;

HostGUI::HostGUI(Host& h) : host(h) {
	int offset = 200;
	rect.left = offset;
	rect.right = rect.left + kWindowWidth;
	rect.top = offset;
	rect.bottom = rect.top + kWindowHeight;
	if (RegisterWC()) {
		wnd = CreateWindow(kClassName, kCaption, WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
			offset, offset, offset + kWindowWidth, offset + kWindowHeight, NULL, NULL, GetModuleHandle(NULL), (LPVOID)this);
		Show();
	}
}

bool HostGUI::RegisterWC() {
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = Wrapper;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = kClassName;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	return RegisterClassEx(&wc);
}

void HostGUI::OnCreate(HWND hWnd) {
	plugin_list = CreateWindow(TEXT("listbox"), NULL, WS_CHILD | WS_VISIBLE | LBS_STANDARD,
		20, 20, kListWidth, kListHeight, hWnd, (HMENU)5, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);
	if (!plugin_list) std::cout << GetLastError() << std::endl;
	for (auto& p : host.plugins) {
		SendMessage(plugin_list, LB_ADDSTRING, 0, (LPARAM)p->GetPluginName().c_str());
	}
	//SendMessage(plugin_list, LB_ADDSTRING, 0, (LPARAM)host.plugins[i]->GetPluginName().c_str());
}

void HostGUI::Show() {
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
}

LRESULT CALLBACK HostGUI::Wrapper(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	HostGUI* gui = NULL;
	if (uMsg == WM_NCCREATE) {
		gui = (HostGUI*)((LPCREATESTRUCT)lParam)->lpCreateParams;
		SetWindowLongPtr(hWnd, GWL_USERDATA, (LONG_PTR)gui);
	}
	else gui = (HostGUI*)GetWindowLongPtr(hWnd, GWL_USERDATA);

	if (!gui) return DefWindowProc(hWnd, uMsg, wParam, lParam);
	else return gui->WindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK HostGUI::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case WM_CREATE:
			OnCreate(hWnd);
			std::cout << "on create" << std::endl;
			break;
		case WM_COMMAND:
			break;
		case WM_CLOSE:
			DestroyWindow(hWnd);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return 0;
}