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
			if (LOWORD(wParam) >= MenuItem::Preset) {
				plugin.SetProgram(LOWORD(wParam) - MenuItem::Preset);
				InvalidateRect(hWnd, NULL, false);
				break;
			}
			switch (LOWORD(wParam)) {
				case MenuItem::Bypass: {
					HMENU menu; 
					if (menu = GetMenu(hWnd)) {
						CheckMenuItem(menu, MenuItem::Bypass, plugin.IsBypassed() ? MF_UNCHECKED : MF_CHECKED);
						plugin.SetBypass(!plugin.IsBypassed());
					}
					break;
				}
				case MenuItem::Active: {
					HMENU menu;
					if (menu = GetMenu(hWnd)) {
						CheckMenuItem(menu, MenuItem::Active, plugin.IsActive() ? MF_UNCHECKED : MF_CHECKED);
						plugin.SetActive(!plugin.IsActive());
					}
					break;
				}
				case MenuItem::Close:
					Window::Hide();
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
	HMENU hmenu = ::CreateMenu();
	// plugin submenu
	HMENU hplugin = ::CreateMenu();
	AppendMenu(hplugin, MF_STRING, MenuItem::Bypass, "Bypass");
	auto flag = MF_STRING;
	if (plugin.IsActive())
		flag |= MF_CHECKED;
	AppendMenu(hplugin, flag, MenuItem::Active, "Active");
	AppendMenu(hplugin, MF_STRING, MenuItem::Close, "Close");
	AppendMenu(hmenu, MF_POPUP, (UINT_PTR)hplugin, "Plugin");
	// state submenu
	HMENU hstate = ::CreateMenu();
	AppendMenu(hstate, MF_STRING, MenuItem::Save, "Save");
	AppendMenu(hstate, MF_STRING, MenuItem::Load, "Load");
	AppendMenu(hstate, MF_STRING, MenuItem::SaveToFile, "Save To File");
	AppendMenu(hstate, MF_STRING, MenuItem::LoadFromFile, "Load From File");
	AppendMenu(hmenu, MF_POPUP, (UINT_PTR)hstate, "State");
	// preset submenu
	HMENU hpresets = ::CreateMenu();
	auto currentProgram = Dispatcher(AEffectOpcodes::effGetProgram);
	bool programChanged = false;
	for (decltype(plugin.GetProgramCount()) i = 0; i < plugin.GetProgramCount(); ++i) {
		char tmp[kVstMaxProgNameLen + 1] = { 0 };
		plugin.Dispatcher(AEffectXOpcodes::effBeginSetProgram);
		if (!Dispatcher(AEffectXOpcodes::effGetProgramNameIndexed, i, 0, tmp)) {
			Dispatcher(AEffectOpcodes::effSetProgram, 0, i);
			Dispatcher(AEffectOpcodes::effGetProgramName, 0, 0, tmp);
			if (!programChanged) programChanged = true;
		}
		plugin.Dispatcher(AEffectXOpcodes::effEndSetProgram);
		AppendMenu(hpresets, MF_STRING, MenuItem::Preset + i, tmp);
	}
	if (programChanged) Dispatcher(AEffectOpcodes::effSetProgram, 0, currentProgram);
	AppendMenu(hmenu, plugin.GetProgramCount() > 0 ? MF_POPUP : MF_POPUP | MF_GRAYED, (UINT_PTR)hpresets, "Plugin");
	return hmenu;
}