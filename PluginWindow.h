#ifndef PLUGINWINDOW_H
#define PLUGINWINDOW_H
#include "Window.h"

namespace VSTHost {
class PluginWindow : public Window {
public:
	PluginWindow(int width, int height);
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
	void OnCreate(HWND hWnd);
	virtual HMENU CreateMenu() = 0;
	static const TCHAR* kClassName;
	//virtual void OnCreate(HWND hWnd);
	//LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
	bool RegisterWC(const TCHAR* class_name);
	static WNDCLASSEX* wc_static;
	static int offset;
	bool is_active{ false };

};
} // namespace

#endif