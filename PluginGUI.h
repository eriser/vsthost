#ifndef PLUGINGUI_H
#define PLUGINGUI_H
#include "Window.h"

class PluginGUI : public Window {
public:
	PluginGUI(int width, int height);
	virtual ~PluginGUI() {}
	virtual bool Initialize(HWND parent) = 0;
	virtual void SetRect() = 0;
protected:
	enum MenuItem {
		Bypass = 10000, Hide, Load, Save
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
};

#endif