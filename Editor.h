#ifndef EDITOR_H
#define EDITOR_H
#include <windows.h>
#include <thread>

#include "Preset.h"

class Editor {
private:
	void RegisterWC();
public:
	Editor(const char *title);
	virtual ~Editor();
	virtual void Show();
	static LRESULT CALLBACK Wrapper(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) = 0;
	virtual void SetRect() = 0;
	void AdjustRect();
	virtual void GetPresets() = 0;
	virtual void OnCreate(HWND wnd) = 0;
protected:
	static const char *className;
	HINSTANCE instance;
	HWND wnd, list, button;
	RECT *rect;
	Preset *preset;
	char **presets;
private:
	static WNDCLASSEX *wc;
};

#endif