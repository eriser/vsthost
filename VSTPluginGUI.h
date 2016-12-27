#ifndef VSTPLUGINGUI_H
#define VSTPLUGINGUI_H
#include "PluginGUI.h"
#include "VSTBase.h"

namespace VSTHost {
class VSTPlugin;
class VSTPluginGUI : public PluginGUI, public VSTBase {
	bool Initialize();
public:
	VSTPluginGUI(VSTPlugin& p, AEffect* aeffect);
	~VSTPluginGUI() {}
	bool Initialize(HWND parent);
	HMENU CreateMenu();
	LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void Show();
	void Hide();
	void SetRect();
private:
	VSTPlugin& plugin;
};
} // namespace

#endif