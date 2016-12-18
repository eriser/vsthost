#include "Host.h"
#include "HostGUI.h"
#include "VSTPlugin.h"
#include "VST3Plugin.h"
#include "VST3PluginGUI.h"
#include "VSTPluginGUI.h"

#include <iostream>

const TCHAR* HostGUI::button_labels[Items::BUTTON_COUNT] = { 
	TEXT("Add"), TEXT("Delete"), TEXT("Move Up"), 
	TEXT("Move Down"), TEXT("Show Editor"), TEXT("Hide Editor") };
const TCHAR* HostGUI::kClassName = TEXT("HostGUI");
const TCHAR* HostGUI::kCaption = TEXT("HostGUI");
const int HostGUI::kWindowWidth = 300;
const int HostGUI::kWindowHeight = 500;
const int HostGUI::kListWidth = 150;
const int HostGUI::kListHeight = 250;
const int HostGUI::kButtonWidth = 30;
const int HostGUI::kButtonHeight = 120; // move to enum?

HostGUI::HostGUI(Host& h) : Window(kWindowWidth, kWindowHeight), host(h) { }

HostGUI::~HostGUI() {
	if (ofn)
		delete ofn;
	//::DeleteObject(font)
}

bool HostGUI::Initialize(HWND parent) {
	if (RegisterWC(kClassName)) {
		wnd = CreateWindow(kClassName, kCaption, WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
			rect.left, rect.top, rect.right, rect.bottom, parent, NULL, GetModuleHandle(NULL), (LPVOID)this);
		if (wnd) {
			SetFont();
			CreateEditors();
			PopulatePluginList();
			SelectPlugin(0);
		}
		return 0 != wnd;
	}
	else return false;
}

void HostGUI::OnCreate(HWND hWnd) {
	plugin_list = CreateWindow(TEXT("listbox"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | WS_HSCROLL | LBS_NOTIFY,
		20, 20, kListWidth, kListHeight, hWnd, (HMENU)Items::PluginList, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);
	for (int i = Items::Add; i < Items::BUTTON_COUNT; ++i) {
		buttons[i] = CreateWindow(TEXT("button"), button_labels[i], WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			20 + kListWidth + 20, 20 + i * 40, kButtonHeight, kButtonWidth, hWnd, (HMENU)i, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);
	}
	//if (!plugin_list) std::cout << GetLastError() << std::endl;
}


LRESULT CALLBACK HostGUI::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case WM_CREATE:
			OnCreate(hWnd);
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case Items::Add:
					if (HIWORD(wParam) == BN_CLICKED)
						OpenDialog();
					break;
				case Items::Delete:
					if (HIWORD(wParam) == BN_CLICKED) {
						auto sel = GetPluginSelection();
						auto count = GetPluginCount();
						host.DeletePlugin(sel);
						// todo: need to call destructor here and free stuff regarding editor
						PopulatePluginList();
						if (sel == count - 1)
							SelectPlugin(sel - 1);
						else
							SelectPlugin(sel);
					}
					break;
				case Items::Up:
					if (HIWORD(wParam) == BN_CLICKED) {
						auto sel = GetPluginSelection();
						host.SwapPlugins(sel, sel - 1);
						PopulatePluginList();
						SelectPlugin(sel - 1);
					}
					break;
				case Items::Down:
					if (HIWORD(wParam) == BN_CLICKED) {
						auto sel = GetPluginSelection();
						host.SwapPlugins(sel, sel + 1);
						PopulatePluginList();
						SelectPlugin(sel + 1);
					}
					break;
				case Items::PluginList:
					if (HIWORD(wParam) == LBN_SELCHANGE) {
						SelectPlugin(GetPluginSelection());
						/*
						auto sel = GetPluginSelection();
						if (sel > 0)
							EnableWindow(buttons[Items::Up], true);
						else
							EnableWindow(buttons[Items::Up], false);
						if (sel < GetPluginCount() - 1)
							EnableWindow(buttons[Items::Down], true);
						else
							EnableWindow(buttons[Items::Down], false);
						*/
					}
					break;
				case Items::Show:
					if (HIWORD(wParam) == BN_CLICKED) {
						auto sel = GetPluginSelection();
						if (host.plugins[sel]->HasEditor()) {
							host.plugins[sel]->ShowEditor();
							EnableWindow(buttons[Items::Show], false);
							EnableWindow(buttons[Items::Hide], true);
						}
					}
					break;
				case Items::Hide:
					if (HIWORD(wParam) == BN_CLICKED) {
						auto sel = GetPluginSelection();
						if (host.plugins[sel]->HasEditor()) {
							host.plugins[sel]->HideEditor();
							EnableWindow(buttons[Items::Show], true);
							EnableWindow(buttons[Items::Hide], false);
						}
					}
					break;
				default:
					break;
			}
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

void HostGUI::AddEditor(Plugin* p) {
	p->CreateEditor(wnd);
	/*
	PluginGUI* editor = nullptr;
	if (p->HasEditor()) {
		if (p->isVST()) editor = new VSTPluginGUI(*dynamic_cast<VSTPlugin*>(p));
		else if (p->IsVST3()) editor = new VST3PluginGUI(*dynamic_cast<VST3Plugin*>(p));
		if (editor && !editor->Initialize(wnd)) {
			delete editor;
			editor = nullptr;
		}
	}
	editors.push_back(editor);
	*/
}

void HostGUI::PopulatePluginList() {
	if (plugin_list) {
		SendMessage(plugin_list, LB_RESETCONTENT, NULL, NULL);
		int i = 0;
		auto v = host.GetPluginNames();
		for (auto& s : v) {
			int pos = SendMessage(plugin_list, LB_ADDSTRING, 0, (LPARAM)s.c_str());
			SendMessage(plugin_list, LB_SETITEMDATA, pos, (LPARAM)i++);
		}
	}
}

void HostGUI::SetFont() {
	NONCLIENTMETRICS metrics;
	metrics.cbSize = sizeof(NONCLIENTMETRICS);
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &metrics, 0);
	HFONT font = CreateFontIndirect(&metrics.lfMessageFont);
	SendMessage(wnd, WM_SETFONT, (WPARAM)font, MAKELPARAM(TRUE, 0));
	SendMessage(plugin_list, WM_SETFONT, (WPARAM)font, MAKELPARAM(TRUE, 0));
	for (int i = Items::Add; i < Items::BUTTON_COUNT; ++i)
		SendMessage(buttons[i], WM_SETFONT, (WPARAM)font, MAKELPARAM(TRUE, 0));
}

void HostGUI::OpenDialog() {
	static char filename[256];
	if (!ofn) {
		ofn = new OPENFILENAME;
		ZeroMemory(ofn, sizeof(*ofn));
		ofn->lStructSize = sizeof(*ofn);
		ofn->hwndOwner = wnd;
		ofn->lpstrFilter = "VST Plugins (*.dll, *.vst3)\0*.dll;*.vst3\0VST2 Plugins (*.dll)\0*.dll\0VST3 Plugins (*.vst3)\0*.vst3\0";
		ofn->lpstrFile = filename;
		ofn->nMaxFile = sizeof(filename);
		ofn->lpstrInitialDir = ".\\";
		ofn->Flags = OFN_FILEMUSTEXIST;
	}
	ofn->lpstrFile[0] = '\0';
	if (GetOpenFileName(ofn)) {
		auto count = GetPluginCount();
		if (host.LoadPlugin(std::string(filename))) {
			AddEditor(host.plugins.back());
			PopulatePluginList();
			SelectPlugin(count);
		}
	}
}

void HostGUI::SelectPlugin(unsigned i) {
	if (plugin_list) {
		SendMessage(plugin_list, LB_SETCURSEL, i, 0);
		SetFocus(plugin_list);
		auto count = GetPluginCount();
		if (count > 2) {
			EnableWindow(buttons[Items::Up], i > 0);
			EnableWindow(buttons[Items::Down], i < count - 1);
		}
		else {
			EnableWindow(buttons[Items::Up], false);
			EnableWindow(buttons[Items::Down], false);
		}
		if (count == 0) {
			EnableWindow(buttons[Items::Show], false);
			EnableWindow(buttons[Items::Hide], false);
		}
		else {
			EnableWindow(buttons[Items::Show], !host.plugins[i]->IsGUIActive());
			EnableWindow(buttons[Items::Hide], host.plugins[i]->IsGUIActive());
		}

	}
}

unsigned HostGUI::GetPluginCount() {
	return host.plugins.size();
}

LRESULT HostGUI::GetPluginSelection() {
	if (plugin_list)
		return SendMessage(plugin_list, LB_GETCURSEL, NULL, NULL);
	else 
		return -1;
}

void HostGUI::CreateEditors() {
	for (auto &p : host.plugins)
		AddEditor(p);
}