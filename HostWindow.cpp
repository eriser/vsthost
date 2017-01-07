#include "HostWindow.h"

#include "Host.h"
#include "PluginVST2.h"
#include "PluginVST3.h"
#include "PluginVST3Window.h"
#include "PluginVST2Window.h"

#include <iostream>

namespace VSTHost {
const TCHAR* HostWindow::button_labels[Items::BUTTON_COUNT] = { 
	TEXT("Add"), TEXT("Delete"), TEXT("Move Up"), 
	TEXT("Move Down"), TEXT("Show Editor"), TEXT("Hide Editor") };
const TCHAR* HostWindow::kClassName = TEXT("HostWindow");
const TCHAR* HostWindow::kCaption = TEXT("HostWindow");
const int HostWindow::kWindowWidth = 300;
const int HostWindow::kWindowHeight = 500;
const int HostWindow::kListWidth = 150;
const int HostWindow::kListHeight = 250;
const int HostWindow::kButtonWidth = 30;
const int HostWindow::kButtonHeight = 120; // move to enum?
bool HostWindow::registered = false;

HostWindow::HostWindow(Host& h) : Window(kWindowWidth, kWindowHeight), font(NULL), host(h) { }

HostWindow::~HostWindow() {
	if (font)
		::DeleteObject(font);
	for (auto b : buttons)
		if (b != NULL)
			::DestroyWindow(b);
	if (plugin_list)
		::DestroyWindow(plugin_list);
	// wnd freed through base class dtor
	// unregistering class seems not worth it
}

bool HostWindow::RegisterWC(const TCHAR* class_name) {
	if (!registered)
		registered = Window::RegisterWC(class_name);
	return registered;
}

bool HostWindow::Initialize(HWND parent) {
	if (RegisterWC(kClassName)) {
		wnd = CreateWindow(kClassName, kCaption, WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
			rect.left, rect.top, rect.right, rect.bottom, parent, NULL, GetModuleHandle(NULL), reinterpret_cast<LPVOID>(this));
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

void HostWindow::OnCreate(HWND hWnd) {
	plugin_list = CreateWindow(TEXT("listbox"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | WS_HSCROLL | LBS_NOTIFY,
		20, 20, kListWidth, kListHeight, hWnd, (HMENU)Items::PluginList, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
	for (int i = Items::Add; i < Items::BUTTON_COUNT; ++i) {
		buttons[i] = CreateWindow(TEXT("button"), button_labels[i], WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			20 + kListWidth + 20, 20 + i * 40, kButtonHeight, kButtonWidth, hWnd, (HMENU)i, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
	}
}


LRESULT CALLBACK HostWindow::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
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
					if (HIWORD(wParam) == LBN_SELCHANGE)
						SelectPlugin(GetPluginSelection());
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

void HostWindow::PopulatePluginList() {
	if (plugin_list) {
		SendMessage(plugin_list, LB_RESETCONTENT, NULL, NULL);
		int i = 0;
		for (auto& p : host.plugins) {
			int pos = SendMessage(plugin_list, LB_ADDSTRING, 0, (LPARAM)p->GetPluginName().c_str());
			SendMessage(plugin_list, LB_SETITEMDATA, pos, (LPARAM)i++);
		}
	}
}

void HostWindow::SetFont() {
	NONCLIENTMETRICS metrics;
	metrics.cbSize = sizeof(NONCLIENTMETRICS);
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &metrics, 0);
	font = CreateFontIndirect(&metrics.lfMessageFont);
	SendMessage(wnd, WM_SETFONT, (WPARAM)font, MAKELPARAM(TRUE, 0));
	SendMessage(plugin_list, WM_SETFONT, (WPARAM)font, MAKELPARAM(TRUE, 0));
	for (int i = Items::Add; i < Items::BUTTON_COUNT; ++i)
		SendMessage(buttons[i], WM_SETFONT, (WPARAM)font, MAKELPARAM(TRUE, 0));
}

void HostWindow::OpenDialog() {
	char filename[MAX_PATH]{};
	if (!ofn) {
		ofn = std::unique_ptr<OPENFILENAMEA>(new OPENFILENAMEA());
		ofn->lStructSize = sizeof(*ofn);
		ofn->hwndOwner = wnd;
		ofn->lpstrFilter = "VST Plugins (*.dll, *.vst3)\0*.dll;*.vst3\0VST2 Plugins (*.dll)\0*.dll\0VST3 Plugins (*.vst3)\0*.vst3\0";
		ofn->nMaxFile = sizeof(filename);
		ofn->lpstrInitialDir = Plugin::kPluginDirectory.c_str();
		ofn->Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
	}
	ofn->lpstrFile = filename;
	if (::GetOpenFileNameA(ofn.get())) {
		auto count = GetPluginCount();
		if (host.LoadPlugin(std::string(filename))) {
			host.plugins.back()->CreateEditor(wnd);
			PopulatePluginList();
			SelectPlugin(count);
		}
	}
}

void HostWindow::SelectPlugin(size_t i) {
	if (plugin_list) {
		SendMessage(plugin_list, LB_SETCURSEL, i, 0);
		SetFocus(plugin_list);
		auto count = GetPluginCount();
		if (count >= 2) {
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
			EnableWindow(buttons[Items::Delete], false);
		}
		else {
			EnableWindow(buttons[Items::Show], !host.plugins[i]->IsEditorVisible());
			EnableWindow(buttons[Items::Hide], host.plugins[i]->IsEditorVisible());
			EnableWindow(buttons[Items::Delete], true);
		}
	}
}

size_t HostWindow::GetPluginCount() {
	return host.plugins.size();
}

size_t HostWindow::GetPluginSelection() {
	if (plugin_list)
		return SendMessage(plugin_list, LB_GETCURSEL, NULL, NULL);
	else 
		return -1;
}

void HostWindow::CreateEditors() {
	for (auto &p : host.plugins)
		p->CreateEditor(wnd);
}
} // namespace