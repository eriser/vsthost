#include "PluginLoader.h"

#include <iostream>
#include <Windows.h>

#include "pluginterfaces/base/ipluginbase.h"
#include "pluginterfaces/vst2.x/aeffect.h"

#include "VST3Plugin.h"
#include "VSTPlugin.h"

extern "C" {
	typedef AEffect* (*VSTInitProc)(audioMasterCallback host);
	typedef bool (PLUGIN_API *VST3InitProc)();
}

PluginLoader::PluginLoader(std::string path) {
	auto module = ::LoadLibraryA(path.c_str()); // unicode pending
	if (module) {
		auto proc = GetProcAddress(module, "GetPluginFactory");
		if (proc) { // the library is a vst3 plugin
			VST3InitProc init_proc = reinterpret_cast<VST3InitProc>(GetProcAddress(module, "InitDll"));
			if (init_proc) // calling init proc to initialize the library
				static_cast<VST3InitProc>(init_proc)();
			Steinberg::IPluginFactory* factory = nullptr;
			GetFactoryProc getFactory = reinterpret_cast<GetFactoryProc>(proc);
			factory = getFactory(); // retrieving factory pointer from factory proc
			plugin = new VST3Plugin(module, factory);
		}
		else {
			proc = GetProcAddress(module, "VSTPluginMain");
			if (!proc)
				proc = GetProcAddress(module, "main"); // older than vst2.4
			if (proc) { // the library is a vst2 plugin
				AEffect* effect = nullptr;
				VSTInitProc init_proc = reinterpret_cast<VSTInitProc>(proc);
				effect = init_proc(VSTPlugin::HostCallbackWrapper);
				plugin = new VSTPlugin(module, effect);
			}
		}
	}
}

PluginLoader::~PluginLoader() {
	if (plugin)
		;// delete plugin; // proper destructors + smart pointers pending
}

Plugin* PluginLoader::GetPlugin() {
	if (plugin && plugin->IsValid()) {
		auto ret = plugin;
		plugin = nullptr;
		return ret;
	}
	else
		return nullptr;
}