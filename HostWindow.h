#ifndef HOSTWINDOW_H
#define HOSTWINDOW_H
#include "Window.h"

#include <vector>

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
	void SelectPlugin(unsigned i);
	unsigned GetPluginCount();
	LRESULT GetPluginSelection();
public:
	HostWindow(Host& h);
	~HostWindow();
	bool Initialize(HWND parent);
	void CreateEditors();
	LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void AddEditor(Plugin* p);
	void PopulatePluginList();
	void OpenDialog();
private:
	Host& host;
	OPENFILENAME *ofn = { nullptr };
	HWND plugin_list;
	HWND buttons[Items::BUTTON_COUNT];
};
} // namespace

#endif