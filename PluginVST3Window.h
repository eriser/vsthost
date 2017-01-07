#ifndef PLUGINVST3WINDOW_H
#define PLUGINVST3WINDOW_H

#include <Windows.h> // deleting this line shows the need for using tchar macro

#include "pluginterfaces/gui/iplugview.h"

#include "PluginWindow.h"

namespace VSTHost {
class PluginVST3;
class PluginVST3Window : public PluginWindow {
	bool Initialize();
public:
	PluginVST3Window(PluginVST3& p, Steinberg::IPlugView* pv);
	~PluginVST3Window();
	bool Initialize(HWND parent);
	HMENU CreateMenu() const;
	void Show();
	void Hide();
	void SetRect();
private:
	Steinberg::IPlugView* plugin_view = { nullptr };
};
} // namespace

#endif