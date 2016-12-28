#include "PluginVST2Window.h"
#include "PluginVST2.h"

#include <iostream>

namespace VSTHost {
PluginVST2Window::PluginVST2Window(PluginVST2& p) : PluginWindow(100, 100, p) {}

void PluginVST2Window::SetRect() {
	ERect *erect = new ERect;
	if (dynamic_cast<PluginVST2&>(plugin).Dispatcher(AEffectOpcodes::effEditGetRect, 0, 0, &erect)) {
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

bool PluginVST2Window::Initialize(HWND parent) {
	if (RegisterWC(kClassName)) {
		SetRect();
		wnd = CreateWindow(kClassName, plugin.GetPluginName().c_str(), WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
			rect.left, rect.top, rect.right, rect.bottom, NULL/*parent*/, CreateMenu(), GetModuleHandle(NULL), (LPVOID)this);
		return wnd != NULL; // i'm setting parent hwnd to null, because child window are displayed in front of parend window
	}						// and it doesn't look right
	else return false;
}

void PluginVST2Window::Show() {
	if (wnd) {
		is_active = true;
		dynamic_cast<PluginVST2&>(plugin).Dispatcher(AEffectOpcodes::effEditOpen, 0, 0, wnd); // call this every time?
		Window::Show();
	}
}

void PluginVST2Window::Hide() {
	if (wnd) {
		is_active = false;
		dynamic_cast<PluginVST2&>(plugin).Dispatcher(AEffectOpcodes::effEditClose, 0, 0, wnd); // this too?
		Window::Hide();
	}
}

HMENU PluginVST2Window::CreateMenu() {
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
	PluginVST2& p = dynamic_cast<PluginVST2&>(plugin);
	auto currentProgram = p.Dispatcher(AEffectOpcodes::effGetProgram);
	bool programChanged = false;
	for (decltype(plugin.GetProgramCount()) i = 0; i < plugin.GetProgramCount(); ++i) {
		char tmp[kVstMaxProgNameLen + 1] = { 0 };
		p.Dispatcher(AEffectXOpcodes::effBeginSetProgram);
		if (!p.Dispatcher(AEffectXOpcodes::effGetProgramNameIndexed, i, 0, tmp)) {
			p.Dispatcher(AEffectOpcodes::effSetProgram, 0, i);
			p.Dispatcher(AEffectOpcodes::effGetProgramName, 0, 0, tmp);
			if (!programChanged) programChanged = true;
		}
		p.Dispatcher(AEffectXOpcodes::effEndSetProgram);
		AppendMenu(hpresets, MF_STRING, MenuItem::Preset + i, tmp);
	}
	if (programChanged) p.Dispatcher(AEffectOpcodes::effSetProgram, 0, currentProgram);
	AppendMenu(hmenu, plugin.GetProgramCount() > 0 ? MF_POPUP : MF_POPUP | MF_GRAYED, (UINT_PTR)hpresets, "Plugin");
	return hmenu;
}
} // namespace