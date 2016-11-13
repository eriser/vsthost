#ifndef HOSTGUI_H
#define HOSTGUI_H

#include <Windows.h>

class Host;
class HostGUI {
	static const TCHAR* kClassName;
	static const TCHAR* kCaption;
	static const int kWindowWidth, kWindowHeight;
	static const int kListWidth, kListHeight;
	void OnCreate(HWND hWnd);
	static LRESULT CALLBACK Wrapper(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool RegisterWC();
public:
	HostGUI(Host& h);
	void Show();
	LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
private:
	Host& host;
	WNDCLASSEX wc;
	RECT rect;	// potrzebne?
	HWND wnd, plugin_list, button_up, button_down;
};


#endif