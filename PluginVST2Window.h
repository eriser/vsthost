#ifndef PLUGINVST2WINDOW_H
#define PLUGINVST2WINDOW_H
#include "PluginWindow.h"
#include "BaseVST2.h"

namespace VSTHost {
class PluginVST2;
class PluginVST2Window : public PluginWindow, public BaseVST2 {
	bool Initialize();
public:
	PluginVST2Window(PluginVST2& p, AEffect* aeffect);
	~PluginVST2Window() {}
	bool Initialize(HWND parent);
	HMENU CreateMenu();
	LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void Show();
	void Hide();
	void SetRect();
private:
	PluginVST2& plugin;
};
} // namespace

#endif