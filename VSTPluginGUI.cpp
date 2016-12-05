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
			rect.left, rect.top, rect.right, rect.bottom, NULL/*parent*/, CreateMenu(), GetModuleHandle(NULL), (LPVOID)this);
		return wnd != NULL; // i'm setting parent hwnd to null, because child window are displayed in front of parend window
	}						// and it doesn't look right
	else return false;
}

void VSTPluginGUI::Show() {
	if (wnd) {
		is_active = true;
		Dispatcher(effEditOpen, 0, 0, wnd); // call this every time?
		Window::Show();
	}
}

void VSTPluginGUI::Hide() {
	if (wnd) {
		is_active = false;
		Dispatcher(effEditClose, 0, 0, wnd); // this too?
		Window::Hide();
	}
}

LRESULT CALLBACK VSTPluginGUI::WindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
	switch (Msg) {
		case WM_CREATE:
			OnCreate(hWnd);
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case MenuItem::Bypass:
					MessageBox(wnd, TEXT("dupa"), TEXT("dupa"), 1);
					break;
				case MenuItem::Load:
					plugin.LoadState();
					InvalidateRect(hWnd, NULL, false);
					break;
				case MenuItem::Save:
					plugin.SaveState();
					break;
				case MenuItem::LoadFromFile:
					plugin.LoadStateFromFile();
					InvalidateRect(hWnd, NULL, false);
					break;
				case MenuItem::SaveToFile:
					plugin.SaveStateToFile();
					break;
				default:
					break;
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
	AppendMenu(plugin, MF_STRING, MenuItem::Close, "Close");
	AppendMenu(menu, MF_POPUP, (UINT_PTR)plugin, "Plugin");

	HMENU presets = ::CreateMenu();
	HMENU load = ::CreateMenu();
	auto v = VSTPluginGUI::plugin.GetPresets();
	int i = 0;
	for (auto& s : v) AppendMenu(load, MF_STRING | MF_GRAYED, i++, s.c_str());
	AppendMenu(presets, MF_POPUP, (UINT_PTR)load, "Load Preset");
	AppendMenu(presets, MF_STRING, MenuItem::Save, "Save");
	AppendMenu(presets, MF_STRING, MenuItem::Load, "Load");
	AppendMenu(presets, MF_STRING, MenuItem::SaveToFile, "Save To File");
	AppendMenu(presets, MF_STRING, MenuItem::LoadFromFile, "Load From File");
	AppendMenu(menu, MF_POPUP, (UINT_PTR)presets, "Presets");
	return menu;
}