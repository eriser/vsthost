#ifndef WINDOW_H
#define WINDOW_H

#include <Windows.h>

class Window {
public:
	Window(int width, int height);
	virtual ~Window() {} // todo: chyba sporo tego bedzie w destruktorze
	virtual bool Initialize(HWND parent) = 0;
	static LRESULT CALLBACK Wrapper(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual void Go();
	virtual void Show();
	virtual void Hide();
	void Refresh();
protected:
	virtual void OnCreate(HWND hWnd);
	virtual LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
	virtual bool RegisterWC(const TCHAR* class_name);
	WNDCLASSEX* wc;
	RECT rect;
	HWND wnd;
};

#endif