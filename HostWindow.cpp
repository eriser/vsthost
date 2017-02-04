#include "HostWindow.h"

#include <iostream>

#include "PluginVST2.h"
#include "PluginVST3.h"
#include "PluginVST3Window.h"
#include "PluginVST2Window.h"
#include "PluginManager.h"

namespace VSTHost {
const TCHAR* HostWindow::button_labels[Items::BUTTON_COUNT] = { 
	TEXT("Add"), TEXT("Delete"), TEXT("Move Up"), TEXT("Move Down"), 
	TEXT("Show Editor"), TEXT("Hide Editor"), TEXT("Save List") };
const TCHAR* HostWindow::kClassName = TEXT("HostWindow");
const TCHAR* HostWindow::kCaption = TEXT("HostWindow");
const int HostWindow::kWindowWidth = 150;
const int HostWindow::kWindowHeight = 160;
const int HostWindow::kListWidth = 150;
const int HostWindow::kListHeight = 250;
const int HostWindow::kButtonWidth = 120;
const int HostWindow::kButtonHeight = 30;
bool HostWindow::registered = false;

HostWindow::HostWindow(PluginManager& pm) : Window(kWindowWidth, kWindowHeight), font(NULL), plugins(pm) { }

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
	buttons[Items::BUTTON_COUNT - 1] = CreateWindow(TEXT("button"), button_labels[Items::BUTTON_COUNT - 1], WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		20, 25 + kListHeight, kButtonWidth + 20 + kListWidth, kButtonHeight, hWnd, 
		(HMENU)(Items::BUTTON_COUNT - 1), (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
	for (int i = Items::Add; i < Items::BUTTON_COUNT - 1; ++i) {
		//HMENU idx{};
		//idx += i; // otherwise the compiler warned about invalid conversion
		buttons[i] = CreateWindow(TEXT("button"), button_labels[i], WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			20 + kListWidth + 20, 20 + i * 40, kButtonWidth, kButtonHeight, hWnd, (HMENU)i, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
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
						auto count = plugins.Size();
						plugins.Delete(sel);
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
						plugins.Swap(sel, sel - 1);
						PopulatePluginList();
						SelectPlugin(sel - 1);
					}
					break;
				case Items::Down:
					if (HIWORD(wParam) == BN_CLICKED) {
						auto sel = GetPluginSelection();
						plugins.Swap(sel, sel + 1);
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
						if (plugins[sel].HasEditor()) {
							plugins[sel].ShowEditor();
							EnableWindow(buttons[Items::Show], false);
							EnableWindow(buttons[Items::Hide], true);
						}
					}
					break;
				case Items::Hide:
					if (HIWORD(wParam) == BN_CLICKED) {
						auto sel = GetPluginSelection();
						if (plugins[sel].HasEditor()) {
							plugins[sel].HideEditor();
							EnableWindow(buttons[Items::Show], true);
							EnableWindow(buttons[Items::Hide], false);
						}
					}
					break;
				case Items::Save:
					if (HIWORD(wParam) == BN_CLICKED)
						plugins.SavePluginList();
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
		for (decltype(plugins.Size()) i = 0; i < plugins.Size(); ++i) {
			int pos = SendMessage(plugin_list, LB_ADDSTRING, 0, (LPARAM)plugins[i].GetPluginName().c_str());
			SendMessage(plugin_list, LB_SETITEMDATA, pos, (LPARAM)i);
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
		auto count = plugins.Size();
		if (plugins.Add(std::string(filename))) {
			plugins.Back().CreateEditor(wnd);
			PopulatePluginList();
			SelectPlugin(count);
		}
	}
}

void HostWindow::SelectPlugin(size_t i) {
	if (plugin_list) {
		SendMessage(plugin_list, LB_SETCURSEL, i, 0);
		SetFocus(plugin_list);
		auto count = plugins.Size();
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
			EnableWindow(buttons[Items::Show], plugins[i].HasEditor() && !plugins[i].IsEditorVisible());
			EnableWindow(buttons[Items::Hide], plugins[i].IsEditorVisible());
			EnableWindow(buttons[Items::Delete], true);
		}
	}
}

size_t HostWindow::GetPluginSelection() {
	if (plugin_list)
		return SendMessage(plugin_list, LB_GETCURSEL, NULL, NULL);
	else 
		return -1;
}

void HostWindow::CreateEditors() {
	for (decltype(plugins.Size()) i = 0; i < plugins.Size(); ++i)
		plugins[i].CreateEditor(wnd);
}
} // namespace