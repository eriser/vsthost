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
		AdjustWindowRect(&rect, WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, false); // WS_SYSMENU
		rect.bottom += ::GetSystemMetrics(SM_CYMENU);
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
			rect.left, rect.top, rect.right, rect.bottom, parent, CreateMenu(), GetModuleHandle(NULL), (LPVOID)this);
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
	std::cout << Msg << std::endl;
	switch (Msg) {
		case WM_CREATE:
			OnCreate(hWnd);
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
			case 100:
				MessageBox(wnd, TEXT("dupa"), TEXT("dupa"), 1);
			}
			break;
		case WM_CLOSE:
			//DestroyWindow(hWnd);
			Window::Hide();
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

HMENU VSTPluginGUI::CreateMenu() {
	HMENU menu = ::CreateMenu();
	HMENU plugin = ::CreateMenu();
	AppendMenu(plugin, MF_STRING, MenuItem::Bypass, "Bypass");
	AppendMenu(plugin, MF_STRING, MenuItem::Hide, "Hide");
	AppendMenu(menu, MF_POPUP, (UINT_PTR)plugin, "Plugin");

	HMENU presets = ::CreateMenu();
	HMENU load = ::CreateMenu();
	auto v = VSTPluginGUI::plugin.GetPresets();
	int i = 0;
	for (auto& s : v) AppendMenu(load, MF_STRING | MF_GRAYED, i++, s.c_str());
	AppendMenu(presets, MF_POPUP, (UINT_PTR)load, "Load");
	AppendMenu(presets, MF_STRING, MenuItem::Save, "Save");
	AppendMenu(menu, MF_POPUP, (UINT_PTR)presets, "Presets");
	return menu;
}