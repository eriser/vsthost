#include "PluginVST3Window.h"
#include "PluginVST3.h"

#include "base/source/fstring.h"

namespace VSTHost {
PluginVST3Window::PluginVST3Window(PluginVST3& p, Steinberg::IPlugView* pv) : PluginWindow(100, 100, p), plugin_view(pv) {}

PluginVST3Window::~PluginVST3Window() {
	if (plugin_view) {
		plugin_view->removed();
		plugin_view->release();
	}
}

void PluginVST3Window::SetRect() {
	Steinberg::ViewRect vr;
	plugin_view->getSize(&vr);
	size_x = vr.getWidth();
	size_y = vr.getHeight();
	rect.left = vr.left;
	rect.right = vr.right;
	rect.bottom = vr.bottom;
	rect.top = vr.top;
	::AdjustWindowRect(&rect, WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, false);
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

bool PluginVST3Window::Initialize(HWND parent) {
	if (plugin_view && RegisterWC(kClassName)) {
		SetRect();
		wnd = ::CreateWindow(kClassName, plugin.GetPluginName().c_str(), WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
			rect.left, rect.top, rect.right, rect.bottom, 
			NULL/*parent*/, menu = CreateMenu(), ::GetModuleHandle(NULL), static_cast<LPVOID>(this));
		if (wnd)	// i'm setting parent hwnd to null, because child window are displayed in front of parend window
			plugin_view->attached(static_cast<void*>(wnd), Steinberg::kPlatformTypeHWND);	// and it doesn't look right
		return wnd != NULL;
	}
	else
		return false;
}

void PluginVST3Window::Show() {
	if (wnd) {
		Steinberg::ViewRect vr;
		plugin_view->getSize(&vr);
		if (vr.right - vr.left != size_x || vr.bottom - vr.top != size_y) {
			SetRect();
			::SetWindowPos(wnd, NULL, rect.left, rect.top, rect.right, rect.bottom, NULL);
		}
		is_active = true;
		Window::Show();
	}
}

void PluginVST3Window::Hide() {
	if (wnd) {
		is_active = false;
		Window::Hide();
	}
}

HMENU PluginVST3Window::CreateMenu() const {
	HMENU hmenu = ::CreateMenu();
	// plugin submenu
	HMENU hplugin = ::CreateMenu();
	::AppendMenu(hplugin, MF_STRING, MenuItem::Bypass, TEXT("Bypass"));
	auto flag = MF_STRING;
	if (plugin.IsActive())
		flag |= MF_CHECKED;
	::AppendMenu(hplugin, flag, MenuItem::Active, TEXT("Active"));
	::AppendMenu(hplugin, MF_STRING, MenuItem::Close, TEXT("Close"));
	::AppendMenu(hmenu, MF_POPUP, (UINT_PTR)hplugin, TEXT("Plugin"));
	// state submenu
	HMENU hstate = ::CreateMenu();
	::AppendMenu(hstate, MF_STRING, MenuItem::Save, TEXT("Save"));
	::AppendMenu(hstate, MF_STRING, MenuItem::Load, TEXT("Load"));
	::AppendMenu(hstate, MF_STRING, MenuItem::SaveToFile, TEXT("Save To File"));
	::AppendMenu(hstate, MF_STRING, MenuItem::LoadFromFile, TEXT("Load From File"));
	::AppendMenu(hmenu, MF_POPUP, (UINT_PTR)hstate, TEXT("State"));
	// preset submenu
	HMENU hpresets = ::CreateMenu();
	PluginVST3& p = dynamic_cast<PluginVST3&>(plugin);
	Steinberg::Vst::ProgramListInfo list_info{};
	for (Steinberg::int32 i = 0; i < plugin.GetProgramCount(); ++i) {
		if (p.unit_info->getProgramListInfo(0, list_info) == Steinberg::kResultTrue) {
			Steinberg::Vst::String128 tmp = { 0 };
			if (p.unit_info->getProgramName(list_info.id, i, tmp) == Steinberg::kResultTrue) {
				Steinberg::String str(tmp);
				::AppendMenuW(hpresets, MF_STRING, MenuItem::Preset + i, str.text16());
			}
		}
	}
	::AppendMenu(hmenu, plugin.GetProgramCount() > 0 ? MF_POPUP : MF_POPUP | MF_GRAYED, (UINT_PTR)hpresets, TEXT("Plugin"));
	return hmenu;
}
} // namespace