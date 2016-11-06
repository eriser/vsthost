#ifndef PLUGINLOADER_H
#define PLUGINLOADER_H

#include "pluginterfaces/base/ipluginbase.h"
#include "pluginterfaces/vst2.x/aeffect.h"

#include <Windows.h>
#include <string>

class PluginLoader {
public:
	//extern "C" {?
	typedef AEffect *(*VSTInitProc)(audioMasterCallback host);
	typedef bool (PLUGIN_API *VST3InitProc)();
	//typedef bool (PLUGIN_API *ExitModuleProc)();
	//}
public:
	PluginLoader(std::string path);
	~PluginLoader();
	bool IsVST();
	bool IsVST3();
	void* GetInitProc();
	HMODULE GetModule();
private:
	HMODULE module = { 0 };
	void* Proc = { nullptr };
	bool isVST = { false };
	bool isVST3 = { false };
};

#endif