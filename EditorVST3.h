#ifndef EDITORVST3_H
#define EDITORVST3_H
#include "Editor.h"

#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "pluginterfaces/gui/iplugview.h"

class EditorVST3 : public Editor {
public:
	EditorVST3(const char *title, Steinberg::Vst::IEditController *ec);
	~EditorVST3();
	void Show();
	LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void SetRect();
	void GetPresets();
	void OnCreate(HWND wnd);
	void ParameterChanged();
private:
	Steinberg::Vst::IEditController *editController;
	Steinberg::IPlugView *pluginView = { nullptr };
};


#endif