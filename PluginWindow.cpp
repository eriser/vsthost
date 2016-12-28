#include "PluginWindow.h"

namespace VSTHost {
const TCHAR* PluginWindow::kClassName = TEXT("PluginWindow");
WNDCLASSEX* PluginWindow::wc_static = nullptr;
int PluginWindow::offset = 50;

PluginWindow::PluginWindow(int width, int height) : Window(width, height) {}

void PluginWindow::OnCreate(HWND hWnd) {} // menu item dodawac

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
} // namespace