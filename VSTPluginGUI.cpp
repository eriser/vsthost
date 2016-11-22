#include "VSTPluginGUI.h"
#include "VSTPlugin.h"

#include <iostream>

VSTPluginGUI::VSTPluginGUI(VSTPlugin& p) : PluginGUI(100, 100), VSTBase(p.GetAEffect()), plugin(p) {  }

void VSTPluginGUI::SetRect() {
	ERect *erect = new ERect;
	if (Dispatcher(effEditGetRect, 0, 0, &erect)) {
		rect.left = erect->left;
		rect.right = erect->right;
		rect.top = erect->top;
		rect.bottom = erect->bottom;
		AdjustWindowRect(&rect, WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, false);
		if (rect.left < 0) {
			rect.right -= rect.left;
			rect.left -= rect.left;
		}
		if (rect.top < 0) {
			rect.bottom -= rect.top;
			rect.top -= rect.top;
		}
	}
	else {
		rect.left = 0;
		rect.top = 0;
		rect.right = 600;
		rect.bottom = 300;
	}
	ApplyOffset();
}

bool VSTPluginGUI::Initialize(HWND parent) {
	if (RegisterWC(kClassName)) {
		SetRect();
		wnd = CreateWindow(kClassName, plugin.GetPluginName().c_str(), WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
			rect.left, rect.top, rect.right, rect.bottom, parent, NULL, GetModuleHandle(NULL), (LPVOID)this);
		Show();
		return wnd != NULL;
	}
	else return false;
}

void VSTPluginGUI::Show() {
	if (wnd)
		Dispatcher(effEditOpen, 0, 0, wnd);
	Window::Show();
}

LRESULT CALLBACK VSTPluginGUI::WindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
	switch (Msg) {
		case WM_CREATE:
			OnCreate(hWnd);
			break;
		case WM_COMMAND:
			break;
		case WM_CLOSE:
			DestroyWindow(hWnd);
			Dispatcher(effEditClose);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, Msg, wParam, lParam);
	}
	return 0;
}