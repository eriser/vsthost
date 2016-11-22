#include "Host.h"
#include "HostGUI.h"
#include "VSTPlugin.h"
#include "VST3Plugin.h"
#include "VST3PluginGUI.h"
#include "VSTPluginGUI.h"

#include <iostream>

const TCHAR* HostGUI::kClassName = TEXT("HostGUI");
const TCHAR* HostGUI::kCaption = TEXT("HostGUI");
const int HostGUI::kWindowWidth = 300;
const int HostGUI::kWindowHeight = 500;
const int HostGUI::kListWidth = 150;
const int HostGUI::kListHeight = 250;

HostGUI::HostGUI(Host& h) : Window(kWindowWidth, kWindowHeight), host(h) { }

HostGUI::~HostGUI() {
	//::DeleteObject(font)
}

bool HostGUI::Initialize(HWND parent) {
	if (RegisterWC(kClassName)) {
		wnd = CreateWindow(kClassName, kCaption, WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
			rect.left, rect.top, rect.right, rect.bottom, parent, NULL, GetModuleHandle(NULL), (LPVOID)this);
		SetFont();
		return 0 != wnd;
	}
	else return false;
}

void HostGUI::OnCreate(HWND hWnd) {
	plugin_list = CreateWindow(TEXT("listbox"), NULL, WS_CHILD | WS_VISIBLE | LBS_STANDARD,
		20, 20, kListWidth, kListHeight, hWnd, NULL, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);
	if (!plugin_list) std::cout << GetLastError() << std::endl;

}


LRESULT CALLBACK HostGUI::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case WM_CREATE:
			OnCreate(hWnd);
			break;
		case WM_COMMAND:
			break;
		case WM_CLOSE:
			DestroyWindow(hWnd);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return 0;
}

void HostGUI::AddEditor(Plugin* p) { // todo: check whether plugin supports an editor at all
	PluginGUI* editor = nullptr;
	if (p->isVST()) editor = new VSTPluginGUI(*dynamic_cast<VSTPlugin*>(p));
	else if (p->IsVST3()) editor = new VST3PluginGUI(*dynamic_cast<VST3Plugin*>(p));
	if (editor && editor->Initialize(wnd)) {
		editor->Show();
		editors.push_back(editor);
	}
	else delete editor;
}

void HostGUI::InsertPluginList(std::vector<std::string>& v) {
	if (plugin_list) {
		for (auto& s : v) SendMessage(plugin_list, LB_ADDSTRING, 0, (LPARAM)s.c_str());
	}
}

void HostGUI::SetFont() {
	NONCLIENTMETRICS metrics;
	metrics.cbSize = sizeof(NONCLIENTMETRICS);
	::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS),
		&metrics, 0);
	HFONT font = ::CreateFontIndirect(&metrics.lfMessageFont);
	::SendMessage(wnd, WM_SETFONT, (WPARAM)font, MAKELPARAM(TRUE, 0));
	::SendMessage(plugin_list, WM_SETFONT, (WPARAM)font, MAKELPARAM(TRUE, 0));
	::SendMessage(button_add, WM_SETFONT, (WPARAM)font, MAKELPARAM(TRUE, 0));
	::SendMessage(button_up, WM_SETFONT, (WPARAM)font, MAKELPARAM(TRUE, 0));
	::SendMessage(button_down, WM_SETFONT, (WPARAM)font, MAKELPARAM(TRUE, 0));
}