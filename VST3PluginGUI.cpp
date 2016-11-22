#include "VST3PluginGUI.h"
#include "VST3Plugin.h"


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
			rect.left, rect.top, rect.right, rect.bottom, parent, CreateMenu(), GetModuleHandle(NULL), (LPVOID)this);
		Show();
		return wnd != NULL;
	}
	else return false;
}

void VST3PluginGUI::Show() {
	if (wnd && plugin_view)
		plugin_view->attached((void*)wnd, Steinberg::kPlatformTypeHWND);
	Window::Show();
}

LRESULT CALLBACK VST3PluginGUI::WindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
	switch (Msg) {
		case WM_CLOSE:
			Window::Hide();
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
	HMENU menu = ::CreateMenu();
	HMENU plugin = ::CreateMenu();
	AppendMenu(plugin, MF_STRING, MenuItem::Bypass, "Bypass");
	AppendMenu(plugin, MF_STRING, MenuItem::Hide, "Hide");
	AppendMenu(menu, MF_POPUP, (UINT_PTR)plugin, "Plugin");

	HMENU presets = ::CreateMenu();
	HMENU load = ::CreateMenu();
	auto v = VST3PluginGUI::plugin.GetPresets();
	int i = 0;
	for (auto& s : v) AppendMenu(load, MF_STRING, i++, s.c_str());
	AppendMenu(presets, MF_POPUP, (UINT_PTR)load, "Load");
	AppendMenu(presets, MF_STRING, MenuItem::Save, "Save");
	AppendMenu(menu, MF_POPUP, (UINT_PTR)presets, "Presets");
	return menu;
}