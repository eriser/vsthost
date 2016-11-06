#ifndef EDITORVST_H
#define EDITORVST_H

#include "pluginterfaces\vst2.x\aeffectx.h"

#include "VSTBase.h"
#include "VSTPreset.h"
#include "Editor.h"

class EditorVST : public Editor, public VSTBase {
public:
	EditorVST(const char *title, AEffect *plugin);
	~EditorVST();
	void Show();
	LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void SetRect();
	void GetPresets();
	void OnCreate(HWND wnd);
	void ParameterChanged();
};

#endif