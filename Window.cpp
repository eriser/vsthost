#include "Window.h"

namespace VSTHost {
Window::Window(int width, int height) : wnd(NULL) {
	int offset_x = 200;
	int offset_y = offset_x;
	rect.left = offset_x;
	rect.right = rect.left + width;
	rect.top = offset_y;
	rect.bottom = rect.top + height;
}

Window::~Window() {
	SetWindowLongPtr(wnd, GWLP_USERDATA, (LONG_PTR)NULL);
	if (wnd)
		::DestroyWindow(wnd);
}

LRESULT CALLBACK Window::Wrapper(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	Window* window = NULL;
	if (uMsg == WM_NCCREATE) {
		window = (Window*)((LPCREATESTRUCT)lParam)->lpCreateParams;
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)window);
	}
	else window = (Window*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	return window ? window->WindowProc(hWnd, uMsg, wParam, lParam) : DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void Window::Go() {
	if (wnd) {
		MSG Msg;
		ShowWindow(wnd, SW_SHOW);
		UpdateWindow(wnd);
		while (GetMessage(&Msg, NULL, 0, 0) > 0)
		{
			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
		}
	}
}

void Window::Show() {
	if (wnd)
		ShowWindow(wnd, SW_SHOW);
}

void Window::Hide() {
	if (wnd)
		ShowWindow(wnd, SW_HIDE);
}

void Window::Refresh() {
	InvalidateRect(wnd, NULL, false);
}

bool Window::RegisterWC(const TCHAR* class_name) {
	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = Wrapper;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = class_name;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	return 0 != RegisterClassEx(&wc);
}

void Window::OnCreate(HWND hWnd) {}
} // namespace