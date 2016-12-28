#ifndef PLUGINVST3WINDOW_H
#define PLUGINVST3WINDOW_H
#include "PluginWindow.h"

#include "pluginterfaces/gui/iplugview.h"

namespace VSTHost {
class PluginVST3;
class PluginVST3Window : public PluginWindow {
	bool Initialize();
public:
	PluginVST3Window(PluginVST3& p, Steinberg::IPlugView* pv);
	~PluginVST3Window();
	bool Initialize(HWND parent);
	HMENU CreateMenu();
	LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void Show();
	void Hide();
	void SetRect();
private:
	PluginVST3& plugin;
	Steinberg::IPlugView* plugin_view = { nullptr };
};
} // namespace

#endif