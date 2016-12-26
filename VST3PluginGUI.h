#ifndef VST3PLUGINGUI_H
#define VST3PLUGINGUI_H
#include "PluginGUI.h"

#include "pluginterfaces/gui/iplugview.h"

class VST3Plugin;
class VST3PluginGUI : public PluginGUI {
	bool Initialize();
public:
	VST3PluginGUI(VST3Plugin& p, Steinberg::IPlugView* pv);
	~VST3PluginGUI();
	bool Initialize(HWND parent);
	HMENU CreateMenu();
	LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void Show();
	void Hide();
	void SetRect();
private:
	VST3Plugin& plugin;
	Steinberg::IPlugView* plugin_view = { nullptr };
};

#endif