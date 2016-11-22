#ifndef HOSTGUI_H
#define HOSTGUI_H
#include "Window.h"

#include <vector>

class Host;
class Plugin;
class PluginGUI;
class HostGUI : public Window {
	static const TCHAR* kClassName;
	static const TCHAR* kCaption;
	static const int kWindowWidth, kWindowHeight;
	static const int kListWidth, kListHeight;
	void OnCreate(HWND hWnd);
public:
	HostGUI(Host& h);
	bool Initialize(HWND parent);
	void CreateEditors();
	void Go();
	bool IsEditorMessage(MSG* msg);
	LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void AddEditor(Plugin* p);
private:
	Host& host;
	std::vector<PluginGUI*> editors;
	HWND plugin_list, button_up, button_down;
};


#endif