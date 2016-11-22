#ifndef VSTPLUGINGUI_H
#define VSTPLUGINGUI_H
#include "PluginGUI.h"
#include "VSTBase.h"

class VSTPlugin;
class VSTPluginGUI : public PluginGUI, public VSTBase {
	bool Initialize();
public:
	VSTPluginGUI(VSTPlugin& p);
	~VSTPluginGUI() {}
	bool Initialize(HWND parent);
	HMENU CreateMenu();
	LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void Show();
	void SetRect();
private:
	VSTPlugin& plugin;
};

#endif