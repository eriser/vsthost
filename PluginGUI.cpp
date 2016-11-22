#include "PluginGUI.h"

const TCHAR* PluginGUI::kClassName = TEXT("PluginGUI");
WNDCLASSEX* PluginGUI::wc_static = nullptr;
int PluginGUI::offset = 50;

PluginGUI::PluginGUI(int width, int height) : Window(width, height) {}

void PluginGUI::OnCreate(HWND hWnd) {} // menu item dodawac

void PluginGUI::ApplyOffset() {
	rect.left += offset;
	rect.top += offset;
	offset += 40;
}

bool PluginGUI::RegisterWC(const TCHAR* class_name) {
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