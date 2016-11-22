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
const int HostGUI::kListWidth = 120;
const int HostGUI::kListHeight = 200;

HostGUI::HostGUI(Host& h) : Window(kWindowWidth, kWindowHeight), host(h) { }

bool HostGUI::Initialize(HWND parent) {
	if (RegisterWC(kClassName)) {
		return wnd = CreateWindow(kClassName, kCaption, WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
			rect.left, rect.top, rect.right, rect.bottom, parent, NULL, GetModuleHandle(NULL), (LPVOID)this);
	}
	else return false;
}

void HostGUI::CreateEditors() {
	/*
	for (auto p : host.plugins) {
		if (p->IsVST3()) {
			// p->CreateEditor(wnd);
		}

	}
	*/
}

void HostGUI::OnCreate(HWND hWnd) {
	plugin_list = CreateWindow(TEXT("listbox"), NULL, WS_CHILD | WS_VISIBLE | LBS_STANDARD,
		20, 20, kListWidth, kListHeight, hWnd, (HMENU)5, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);
	if (!plugin_list) std::cout << GetLastError() << std::endl;
	//for (auto& p : host.plugins) {
	//	SendMessage(plugin_list, LB_ADDSTRING, 0, (LPARAM)p->GetPluginName().c_str());
	//}
	//SendMessage(plugin_list, LB_ADDSTRING, 0, (LPARAM)host.plugins[i]->GetPluginName().c_str());
}

LRESULT CALLBACK HostGUI::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case WM_CREATE:
			OnCreate(hWnd);
			std::cout << "on create" << std::endl;
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
	if (p->isVST()) {
		editor = new VSTPluginGUI(*dynamic_cast<VSTPlugin*>(p));
		std::cout << "jestem tu" << std::endl;
	}
		
	else if (p->IsVST3())
		editor = new VST3PluginGUI(*dynamic_cast<VST3Plugin*>(p));
	if (editor && editor->Initialize(wnd)) {
		editor->Show();
		editors.push_back(editor);
	}
	else delete editor;
}