#include "PluginWindow.h"

#include "Plugin.h"

namespace VSTHost {
const TCHAR* PluginWindow::kClassName = TEXT("PluginWindow");
int PluginWindow::offset = 50;
bool PluginWindow::registered = false;

PluginWindow::PluginWindow(int width, int height, Plugin& p) : Window(width, height), menu(NULL), plugin(p) {

}

PluginWindow::~PluginWindow() {
	if (menu)
		DestroyMenu(menu);
}

void PluginWindow::ApplyOffset() {
	rect.left += offset;
	rect.top += offset;
	offset += 40;
}

bool PluginWindow::RegisterWC(const TCHAR* class_name) {
	if (!registered)
		registered = Window::RegisterWC(class_name);
	return registered;
}

bool PluginWindow::IsActive() const {
	return is_active;
}


LRESULT CALLBACK PluginWindow::WindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
	switch (Msg) {
		case WM_CLOSE:
			Hide();
			break;
		case WM_COMMAND:
			if (LOWORD(wParam) >= MenuItem::Preset) {
				plugin.SetProgram(LOWORD(wParam) - MenuItem::Preset);
				break;
			}
			switch (LOWORD(wParam)) {
				case MenuItem::Bypass: {
					if (menu) {
						CheckMenuItem(menu, MenuItem::Bypass, plugin.IsBypassed() ? MF_UNCHECKED : MF_CHECKED);
						plugin.SetBypass(!plugin.IsBypassed());
					}
					break;
				}
				case MenuItem::Active: {
					if (menu) {
						CheckMenuItem(menu, MenuItem::Active, plugin.IsActive() ? MF_UNCHECKED : MF_CHECKED);
						plugin.SetActive(!plugin.IsActive());
					}
					break;
				}
				case MenuItem::Close:
					Hide();
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
} // namespace