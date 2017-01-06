#ifndef HOSTWINDOW_H
#define HOSTWINDOW_H
#include "Window.h"

#include <vector>
#include <memory>

namespace VSTHost {
class Host;
class Plugin;
class PluginWindow;
class HostWindow : public Window {
	enum Items {
		Add = 0, Delete, Up, Down, Show, Hide, BUTTON_COUNT, PluginList
	};
	static const TCHAR* button_labels[Items::BUTTON_COUNT];
	static const TCHAR* kClassName;
	static const TCHAR* kCaption;
	static const int kWindowWidth, kWindowHeight;
	static const int kListWidth, kListHeight;
	static const int kButtonWidth, kButtonHeight;
	void OnCreate(HWND hWnd);
	void SetFont();
	void SelectPlugin(size_t i);
	size_t GetPluginCount();
	LRESULT GetPluginSelection();
public:
	HostWindow(Host& h);
	~HostWindow();
	bool Initialize(HWND parent);
	void CreateEditors();
	LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void PopulatePluginList();
	void OpenDialog();
	bool RegisterWC(const TCHAR* class_name);
private:
	static bool registered;
	HFONT font;
	HWND plugin_list;
	HWND buttons[Items::BUTTON_COUNT];
	std::unique_ptr<OPENFILENAMEA> ofn;
	Host& host;
};
} // namespace

#endif