#include "PluginLoader.h"

#include <Windows.h>

#include "pluginterfaces/base/ipluginbase.h"
#include "pluginterfaces/vst2.x/aeffect.h"

#include "PluginVST3.h"
#include "PluginVST2.h"

extern "C" {
	typedef AEffect* (*VSTInitProc)(audioMasterCallback host);
	typedef bool (PLUGIN_API *VST3InitProc)();
}

namespace VSTHost {
std::unique_ptr<Plugin> PluginLoader::Load(const std::string& path, Steinberg::Vst::TSamples bs, Steinberg::Vst::SampleRate sr) {
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
			plugin = new PluginVST3(module, factory, bs, sr);
		}
		else {
			proc = ::GetProcAddress(module, "PluginVST2Main");
			if (!proc)
				proc = ::GetProcAddress(module, "main"); // older than vst2.4
			if (proc) { // the library is a vst2 plugin
				AEffect* effect = nullptr;
				VSTInitProc init_proc = reinterpret_cast<VSTInitProc>(proc);
				effect = init_proc(PluginVST2::HostCallbackWrapper);
				plugin = new PluginVST2(module, effect, bs, sr);
			}
		}
	}
	if (plugin && !plugin->IsValid())
		plugin = nullptr;
	return std::unique_ptr<Plugin>(plugin);
}
} // namespace