#include "EditorVST.h"
using namespace std;

EditorVST::EditorVST(const char *title, AEffect *plugin) : Editor(title), VSTBase(plugin) {
	preset = new VSTPreset(plugin);
	plugin->resvd1 = (VstIntPtr)this;
	GetPresets();
	SetRect();
	AdjustRect();
	wnd = CreateWindow(className, title, WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, 
		rect->left, rect->top, rect->right, rect->bottom, NULL, NULL, instance, (LPVOID)this);
}

EditorVST::~EditorVST() {
	if (presets) {
		for (int i = 0; i <= GetNumPrograms(); i++) {
			if (presets[i]) delete[] presets[i];
		}
		delete[] presets;
	}
}

void EditorVST::Show() {
	if (wnd) 
		Dispatcher(effEditOpen, 0, 0, wnd);
	Editor::Show();
}

LRESULT CALLBACK EditorVST::WindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
	switch(Msg) {
		case WM_CREATE:
			OnCreate(hWnd);	// podaje uchwyt, bo funkcja OnCreate sie wywoluje 
			break;			// zanim CreateWindow zwraca uchwyt do pola klasy
		case WM_COMMAND:
			if (HIWORD(wParam) == CBN_SELENDOK) {
				int index = SendMessage(list, CB_GETCURSEL, NULL, NULL);
				if (index == GetNumPrograms()) preset->Load();
				else Dispatcher(effSetProgram, 0, index);
				InvalidateRect(hWnd, NULL, false);
			}
			else if (HIWORD(wParam) == BN_CLICKED) {
				preset->Save();
			}
			break;
		case WM_CLOSE:
			DestroyWindow(hWnd);
			Dispatcher(effEditClose);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, Msg, wParam, lParam);
	}
	return 0;
}

void EditorVST::SetRect() {
	rect = new RECT;
	ERect *erect = new ERect;
	if (Dispatcher(effEditGetRect, 0, 0, &erect)) {
		rect->left = erect->left;
		rect->right = erect->right;
		rect->top = erect->top;
		rect->bottom = erect->bottom;
	}
	else {
		rect->left = 0;
		rect->top = 0;
		rect->right = 600;
		rect->bottom = 300;
	}
}

void EditorVST::GetPresets() {
	presets = new char *[GetNumPrograms() + 1];
	int currentProgram = Dispatcher(effGetProgram);
	bool programChanged = false;
	for (int i = 0; i < GetNumPrograms(); i++) {
		presets[i] = new char[kVstMaxProgNameLen + 1]();
		if (!Dispatcher(effGetProgramNameIndexed, i, 0, presets[i])) {
			Dispatcher(effSetProgram, 0, i);
			Dispatcher(effGetProgramName, 0, 0, presets[i]);
			if (!programChanged) programChanged = true;
		}
	}
	//programs[GetNumPrograms()] = new char[kVstMaxProgNameLen + 1];
	presets[GetNumPrograms()] = "Custom";
	if (programChanged) Dispatcher(effSetProgram, 0, currentProgram);
}

void EditorVST::OnCreate(HWND wnd) {
	ERect *erect = new ERect;
	Dispatcher(effEditGetRect, 0, 0, &erect);
	// lista
	int listHeight = (GetNumPrograms() + 1) * 23, listWidth = (rect->right - rect->left) / 3;
	int y = erect->bottom + 23 - GetSystemMetrics(SM_CYCAPTION) - GetSystemMetrics(SM_CYFRAME);
	list = CreateWindow("combobox", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWN, 
		0, y, listWidth, listHeight, wnd, NULL, instance, NULL);
	for (int i = 0; i <= GetNumPrograms(); i++) SendMessage(list, CB_ADDSTRING, 0, (LPARAM)presets[i]);
	// przycisk
	button = CreateWindow("button", "Save", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 
		listWidth + 10, y, 90, 22, wnd, NULL, instance, NULL);
	if (preset->Load()) SendMessage(list, CB_SETCURSEL, GetNumPrograms(), NULL);
	else SendMessage(list, CB_SETCURSEL, 0, NULL);
}

void EditorVST::ParameterChanged() {
	SendMessage(list, CB_SETCURSEL, GetNumPrograms(), NULL);
}