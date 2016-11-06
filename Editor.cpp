#include "Editor.h"

const char* Editor::className = "EditorClass";
WNDCLASSEX* Editor::wc = NULL;

void Editor::RegisterWC() {
	if (!wc) {
		wc = new WNDCLASSEX;
		wc->cbSize = sizeof(WNDCLASSEX);
		wc->style = 0;
		wc->lpfnWndProc = Wrapper;
		wc->cbClsExtra = 0;
		wc->cbWndExtra = 0;
		wc->hInstance = instance;
		wc->hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wc->hCursor = LoadCursor(NULL, IDC_ARROW);
		wc->hbrBackground = (HBRUSH)(COLOR_WINDOWFRAME);
		wc->lpszMenuName = NULL;
		wc->lpszClassName = className;
		wc->hIconSm = LoadIcon(NULL, IDI_APPLICATION);
		RegisterClassEx(wc);
	}
}

Editor::Editor(const char *title) : wnd(NULL), list(NULL), button(NULL) {
	instance = GetModuleHandle(NULL);
	if (!wc)
		RegisterWC();
	//GetPresets();
	//SetRect();

	//wnd = CreateWindow(className, title, WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, 
	//	rect->left, rect->top, rect->right, rect->bottom, NULL, NULL, instance, (LPVOID)this);
}

Editor::~Editor() {
	if (rect) 
		delete rect;
	if (wc)
		delete wc;
}

void Editor::AdjustRect() {
	if (rect->right - rect->left < 150) rect->right = rect->left + 150;	// minimalna szerokosc
	rect->bottom += 22;	// wysokosc paska z presetami
	AdjustWindowRect(rect, WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, false);
	if (rect->left < 0) {
		rect->right -= rect->left;
		rect->left -= rect->left;
	}
	if (rect->top < 0) {
		rect->bottom -= rect->top;
		rect->top -= rect->top;
	}
}

void Editor::Show() {
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

LRESULT CALLBACK Editor::Wrapper(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	Editor *editor = NULL;
	if (uMsg == WM_NCCREATE) {
		editor = (Editor*)((LPCREATESTRUCT)lParam)->lpCreateParams;
		SetWindowLongPtr(hWnd, GWL_USERDATA, (LONG_PTR)editor);
	}
	else editor = (Editor*)GetWindowLongPtr(hWnd, GWL_USERDATA);

	if (!editor) return DefWindowProc(hWnd, uMsg, wParam, lParam);
	else return editor->WindowProc(hWnd, uMsg, wParam, lParam);
}