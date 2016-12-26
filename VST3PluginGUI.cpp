#include "VST3PluginGUI.h"
#include "VST3Plugin.h"

#include "base/source/fstring.h"
VST3PluginGUI::VST3PluginGUI(VST3Plugin& p, Steinberg::IPlugView* pv) : PluginGUI(100, 100), plugin(p), plugin_view(pv) {}

VST3PluginGUI::~VST3PluginGUI() {
	if (plugin_view)
		plugin_view->release();
}

void VST3PluginGUI::SetRect() {
	Steinberg::ViewRect vr;
	plugin_view->getSize(&vr);
	rect.left = vr.left;
	rect.right = vr.right;
	rect.bottom = vr.bottom;
	rect.top = vr.top;
	AdjustWindowRect(&rect, WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, false);
	rect.bottom += ::GetSystemMetrics(SM_CYMENU);
	if (rect.left < 0) {
		rect.right -= rect.left;
		rect.left -= rect.left;
	}
	if (rect.top < 0) {
		rect.bottom -= rect.top;
		rect.top -= rect.top;
	}
	ApplyOffset();
}

bool VST3PluginGUI::Initialize(HWND parent) {
	if (plugin_view && RegisterWC(kClassName)) {
		SetRect();
		wnd = CreateWindow(kClassName, plugin.GetPluginName().c_str(), WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
			rect.left, rect.top, rect.right, rect.bottom, 
			NULL/*parent*/, CreateMenu(), GetModuleHandle(NULL), static_cast<LPVOID>(this));
		if (wnd)	// i'm setting parent hwnd to null, because child window are displayed in front of parend window
			plugin_view->attached(static_cast<void*>(wnd), Steinberg::kPlatformTypeHWND);	// and it doesn't look right
		return wnd != NULL;
	}
	else
		return false;
}

void VST3PluginGUI::Show() {
	if (wnd) {
		is_active = true;
		Window::Show();
	}
}

void VST3PluginGUI::Hide() {
	if (wnd) {
		is_active = false;
		Window::Hide();
	}
}

LRESULT CALLBACK VST3PluginGUI::WindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
	switch (Msg) {
		case WM_CLOSE:
			Window::Hide();
			break;
		case WM_COMMAND:
			if (LOWORD(wParam) >= MenuItem::Preset) {
				plugin.SetProgram(LOWORD(wParam) - MenuItem::Preset);
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
		case WM_DESTROY:
			::PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, Msg, wParam, lParam);
	}
	return 0;
}

HMENU VST3PluginGUI::CreateMenu() {
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
	Steinberg::Vst::ProgramListInfo list_info{};
	for (Steinberg::int32 i = 0; i < plugin.GetProgramCount(); ++i) {
		if (plugin.unit_info->getProgramListInfo(0, list_info) == Steinberg::kResultTrue) {
			Steinberg::Vst::String128 tmp = { 0 };
			if (plugin.unit_info->getProgramName(list_info.id, i, tmp) == Steinberg::kResultTrue) {
				Steinberg::String str(tmp);
				AppendMenu(hpresets, MF_STRING, MenuItem::Preset + i, str.text8());
			}
		}
	}
	AppendMenu(hmenu, plugin.GetProgramCount() > 0 ? MF_POPUP : MF_POPUP | MF_GRAYED, (UINT_PTR)hpresets, "Plugin");
	return hmenu;
}