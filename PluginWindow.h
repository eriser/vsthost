#ifndef PLUGINWINDOW_H
#define PLUGINWINDOW_H
#include "Window.h"

namespace VSTHost {
class Plugin;
class PluginWindow : public Window {
public:
	PluginWindow(int width, int height, Plugin& p);
	virtual ~PluginWindow() {}
	virtual bool Initialize(HWND parent) = 0;
	virtual void SetRect() = 0;
	bool IsActive();
protected:
	enum MenuItem {
		Bypass = 10000, Active, Close, 
		State, Load, Save, LoadFromFile, SaveToFile, 
		Presets, Preset = 20000
	};
	void ApplyOffset();
	virtual HMENU CreateMenu() = 0;
	static const TCHAR* kClassName;
	LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool RegisterWC(const TCHAR* class_name);
	static WNDCLASSEX* wc_static;
	static int offset;
	bool is_active{ false };
	Plugin& plugin;
};
} // namespace

#endif