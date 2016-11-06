#include "EditorVST3.h"
#include <iostream>

EditorVST3::EditorVST3(const char *title, Steinberg::Vst::IEditController *ec) : Editor(title), editController(ec) {
	pluginView = editController->createView("editor");
	SetRect();
	AdjustRect();
	rect->bottom -= 22;
	wnd = CreateWindow(className, title, WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		rect->left, rect->top, rect->right, rect->bottom, NULL, NULL, instance, (LPVOID)this);
	if (pluginView) {
		pluginView->attached((void*)wnd, Steinberg::kPlatformTypeHWND);
	}

}

EditorVST3::~EditorVST3() {
	pluginView->release();
}

void EditorVST3::Show() {
	Editor::Show();
}

LRESULT CALLBACK EditorVST3::WindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
	switch (Msg) {
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, Msg, wParam, lParam);
	}
	return 0;
}

void EditorVST3::SetRect() {
	if (!rect)
		rect = new RECT;
	Steinberg::ViewRect vr;
	pluginView->getSize(&vr);
	rect->left = vr.left;
	rect->right = vr.right;
	rect->bottom = vr.bottom;
	rect->top = vr.top;
}

void EditorVST3::GetPresets() {

}

void EditorVST3::OnCreate(HWND wnd) {

}

void EditorVST3::ParameterChanged() {

}