#include "PluginWindow.h"

#include "Plugin.h"

namespace VSTHost {
const TCHAR* PluginWindow::kClassName = TEXT("PluginWindow");
WNDCLASSEX* PluginWindow::wc_static = nullptr;
int PluginWindow::offset = 50;

PluginWindow::PluginWindow(int width, int height, Plugin& p) : Window(width, height), plugin(p) {}

void PluginWindow::ApplyOffset() {
	rect.left += offset;
	rect.top += offset;
	offset += 40;
}

bool PluginWindow::RegisterWC(const TCHAR* class_name) {
	if (!wc_static) {
		wc_static = new WNDCLASSEX;
		wc_static->cbSize = sizeof(WNDCLASSEX);
		wc_static->style = 0;
		wc_static->lpfnWndProc = Wrapper;
		wc_static->cbClsExtra = 0;
		wc_static->cbWndExtra = 0;
		wc_static->hInstance = GetModuleHandle(NULL);
		wc_static->hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wc_static->hCursor = LoadCursor(NULL, IDC_ARROW);
		wc_static->hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
		wc_static->lpszMenuName = NULL;
		wc_static->lpszClassName = class_name;
		wc_static->hIconSm = LoadIcon(NULL, IDI_APPLICATION);
		return 0 != RegisterClassEx(wc_static);
	}
	else return true;
}

bool PluginWindow::IsActive() {
	return is_active;
}


LRESULT CALLBACK PluginWindow::WindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
	switch (Msg) {
		case WM_CLOSE:
			//DestroyWindow(hWnd);
			Hide();
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