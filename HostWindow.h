#ifndef HOSTWINDOW_H
#define HOSTWINDOW_H
#include "Window.h"

#include <vector>
#include <memory>

#include "Host.h"

namespace VSTHost {
class Plugin;
class PluginWindow;
class PluginManager;
class HostWindow : public Window {
	enum Items {
		Add = 0, Delete, Up, Down, Show, Hide, Save, BUTTON_COUNT, PluginList
	};
	static const TCHAR* button_labels[Items::BUTTON_COUNT];
	static const TCHAR* kClassName;
	static const TCHAR* kCaption;
	static const int kWindowWidth, kWindowHeight;
	static const int kListWidth, kListHeight;
	static const int kButtonWidth, kButtonHeight;
	void OnCreate(HWND hWnd);
	void SetFont();
	size_t GetPluginSelection();
public:
	HostWindow(PluginManager& pm);
	~HostWindow();
	bool Initialize(HWND parent);
	void CreateEditors();
	LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void PopulatePluginList();
	void SelectPlugin(size_t i);
	void OpenDialog();
	bool RegisterWC(const TCHAR* class_name);
private:
	static bool registered;
	HFONT font;
	HWND plugin_list;
	HWND buttons[Items::BUTTON_COUNT];
	std::unique_ptr<OPENFILENAMEA> ofn;
	PluginManager& plugins;
};
} // namespace

#endif