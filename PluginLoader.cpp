#include "PluginLoader.h"
#include <iostream>

PluginLoader::PluginLoader(std::string path) {
	module = LoadLibraryA(path.c_str());
	if (module) {
		Proc = GetProcAddress(module, "GetPluginFactory");
		if (Proc) {
			isVST3 = true;
			void* initProc = GetProcAddress(module, "InitDll");
			if (initProc)
				static_cast<VST3InitProc>(initProc)();
		}
		else {
			// std::cout << GetLastError() << std::endl;
			Proc = GetProcAddress(module, "VSTPluginMain");
			if (!Proc)
				Proc = GetProcAddress(module, "main");
			if (Proc)
				isVST = true;
			else {
				FreeLibrary(module);
				Proc = nullptr;
				module = 0;
			}
		}
	}
}

PluginLoader::~PluginLoader() {}

bool PluginLoader::IsVST() {
	return isVST;
}

bool PluginLoader::IsVST3() {
	return isVST3;
}

void* PluginLoader::GetInitProc() {
	if (isVST || isVST3) 
		return Proc;
	return nullptr;
}

HMODULE PluginLoader::GetModule() {
	return module;
}