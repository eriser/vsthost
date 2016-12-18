#include "VST3PluginGUI.h"
#include "VST3Plugin.h"

#include "base/source/fstring.h"
VST3PluginGUI::VST3PluginGUI(VST3Plugin& p) : PluginGUI(100, 100), plugin(p), plugin_view(plugin.CreateView()) {}

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
	if (RegisterWC(kClassName)) {
		SetRect();
		wnd = CreateWindow(kClassName, plugin.GetPluginName().c_str(), WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
			rect.left, rect.top, rect.right, rect.bottom, NULL/*parent*/, CreateMenu(), GetModuleHandle(NULL), (LPVOID)this);
		plugin_view = plugin.CreateView();	// i'm setting parent hwnd to null, because child window are displayed in front of parend window
		if (wnd && plugin_view)				// and it doesn't look right
			plugin_view->attached((void*)wnd, Steinberg::kPlatformTypeHWND);
		return wnd != NULL;
	}
	else return false;
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
				case MenuItem::Bypass:
					HMENU menu;
					if (menu = GetMenu(hWnd)) {
 						CheckMenuItem(menu, MenuItem::Bypass, bypass ? MF_UNCHECKED : MF_CHECKED);
						bypass = !bypass;
						plugin.SetBypass(bypass);
					}
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
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, Msg, wParam, lParam);
	}
	return 0;
}

HMENU VST3PluginGUI::CreateMenu() {
	HMENU hmenu = ::CreateMenu();
	HMENU hplugin = ::CreateMenu();
	AppendMenu(hplugin, MF_STRING, MenuItem::Bypass, "Bypass");
	AppendMenu(hplugin, MF_STRING, MenuItem::Close, "Close");
	AppendMenu(hmenu, MF_POPUP, (UINT_PTR)hplugin, "Plugin");

	HMENU hpresets = ::CreateMenu();
	HMENU hload = ::CreateMenu();
	Steinberg::Vst::ProgramListInfo list_info{};
	for (Steinberg::uint32 i = 0; i < plugin.GetProgramCount(); ++i) {
		if (plugin.unit_info->getProgramListInfo(0, list_info) == Steinberg::kResultTrue) {
			Steinberg::Vst::String128 tmp = { 0 };
			if (plugin.unit_info->getProgramName(list_info.id, i, tmp) == Steinberg::kResultTrue) {
				Steinberg::String str(tmp);
				AppendMenu(hload, MF_STRING, MenuItem::Preset + i, str.text8());
			}
		}
	}
	AppendMenu(hpresets, plugin.GetProgramCount() > 0 ? MF_POPUP : MF_POPUP | MF_GRAYED, (UINT_PTR)hload, "Load Preset");
	AppendMenu(hpresets, MF_STRING, MenuItem::Save, "Save");
	AppendMenu(hpresets, MF_STRING, MenuItem::Load, "Load");
	AppendMenu(hpresets, MF_STRING, MenuItem::SaveToFile, "Save To File");
	AppendMenu(hpresets, MF_STRING, MenuItem::LoadFromFile, "Load From File");
	AppendMenu(hmenu, MF_POPUP, (UINT_PTR)hpresets, "Presets");
	return hmenu;
}