#include "PluginLoader.h"

#include <Windows.h>
#include <iostream>

#include "pluginterfaces/base/ipluginbase.h"
#include "pluginterfaces/vst2.x/aeffect.h"

#include "PluginVST3.h"
#include "PluginVST2.h"

extern "C" {
	typedef AEffect* (*VSTInitProc)(audioMasterCallback host);
	typedef bool (PLUGIN_API *VST3InitProc)();
}

namespace VSTHost {
std::unique_ptr<Plugin> PluginLoader::Load(const std::string& path, Steinberg::FUnknown* context) {
	Plugin* plugin = nullptr;
	auto module = ::LoadLibraryA(path.c_str());
	if (module) {
		auto proc = ::GetProcAddress(module, "GetPluginFactory");
		if (proc) { // the library is a vst3 plugin
			VST3InitProc init_proc = reinterpret_cast<VST3InitProc>(::GetProcAddress(module, "InitDll"));
			if (init_proc) // calling init proc to initialize the library
				static_cast<VST3InitProc>(init_proc)();
			Steinberg::IPluginFactory* factory = nullptr;
			GetFactoryProc getFactory = reinterpret_cast<GetFactoryProc>(proc);
			factory = getFactory(); // retrieving factory pointer from factory proc
			plugin = new PluginVST3(module, factory, context);
		}
		else {
			proc = ::GetProcAddress(module, "VSTPluginMain");
			if (!proc)
				proc = ::GetProcAddress(module, "main"); // older than vst2.4
			if (proc) { // the library is a vst2 plugin
				AEffect* effect = nullptr;
				VSTInitProc init_proc = reinterpret_cast<VSTInitProc>(proc);
				effect = init_proc(PluginVST2::HostCallbackWrapper);
				plugin = new PluginVST2(module, effect);
			}
			else
				std::cerr << "Error loading plugin: Could not locate plugin entry procedure in " << path << "." << std::endl;
		}
	}
	else {
		std::cerr << "Error loading plugin: ";
		switch (::GetLastError()) {
			case ERROR_MOD_NOT_FOUND:
				std::cerr << "Could not locate " << path << ".";
				break;
			case ERROR_BAD_EXE_FORMAT: {
				constexpr int bits = sizeof(void*) == 8 ? 64 : 32;
				std::cerr << "Bad DLL format (need " << bits << "bit version).";
				break;
			}
			default:
				std::cerr << "Could not load " << path << ".";
		}
		std::cerr << std::endl;
	}
	if (plugin && plugin->IsValid() != Plugin::IsValidCodes::kValid) {
		std::cerr << "Error loading plugin: ";
		switch (plugin->IsValid()) {
			case Plugin::IsValidCodes::kIsNotEffect:
				std::cerr << path << " is not an effect.";
				break;
			case Plugin::IsValidCodes::kWrongInOutNum:
				std::cerr << path << " does not support stereo.";
				break;
			case Plugin::IsValidCodes::kInvalid:
				std::cerr << path << " is invalid.";
				break;
			default:
				std::cerr << "Unknown error loading " << path << ".";
		}
		std::cerr << std::endl;
		delete plugin;
		plugin = nullptr;
	}
	return std::unique_ptr<Plugin>(plugin);
}
} // namespace