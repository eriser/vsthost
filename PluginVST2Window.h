#ifndef PLUGINVST2WINDOW_H
#define PLUGINVST2WINDOW_H

#include "PluginWindow.h"

namespace VSTHost {
class PluginVST2;
class PluginVST2Window : public PluginWindow {
	bool Initialize();
public:
	PluginVST2Window(PluginVST2& p);
	~PluginVST2Window() {}
	bool Initialize(HWND parent);
	HMENU CreateMenu();
	void Show();
	void Hide();
	void SetRect();
};
} // namespace

#endif