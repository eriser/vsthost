#ifndef VST3PLUGIN_H
#define VST3PLUGIN_H

#include "base/source/fobject.h"
#include "pluginterfaces/base/ipluginbase.h"
#include "pluginterfaces/vst/ivstcomponent.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"

#include "public.sdk/source/vst/hosting/parameterchanges.h"
//DEF_CLASS_IID(Steinberg::Vst::IComponent)
//DEF_CLASS_IID(Steinberg::Vst::IComponentHandler)
//DEF_CLASS_IID(Steinberg::Vst::IAudioProcessor)
//#include <Windows.h>

#include "Plugin.h"

class VST3Plugin : public Plugin, public Steinberg::FObject, public Steinberg::Vst::IComponentHandler {
	// extern "C" ?
	typedef bool (PLUGIN_API *ExitModuleProc)();
public:
	OBJ_METHODS(VST3Plugin, FObject)
	REFCOUNT_METHODS(FObject)
	DEFINE_INTERFACES
		DEF_INTERFACE(IComponentHandler)
	END_DEFINE_INTERFACES(FObject)
	VST3Plugin(HMODULE m, Steinberg::IPluginFactory* f);
	~VST3Plugin();
	void Process(Steinberg::Vst::Sample32** input, Steinberg::Vst::Sample32** output);
	void ProcessOutputParameterChanges();
	bool IsValid();
	void PrintInfo();
	void PrintFactory();
	void PrintClass(const Steinberg::PClassInfo& ci, int i);
	void PrintClass2(const Steinberg::PClassInfo2 &ci, int i);
	void PrintBusInfo();
	void PrintParameters();
	std::string GetPluginName();
	void SetupAudio();
	bool IsVST3() { return true; }
	bool isVST() { return false; }
	Steinberg::IPlugView * CreateView();
	Steinberg::tresult PLUGIN_API beginEdit(Steinberg::Vst::ParamID id);
	Steinberg::tresult PLUGIN_API performEdit(Steinberg::Vst::ParamID id, Steinberg::Vst::ParamValue valueNormalized);
	Steinberg::tresult PLUGIN_API endEdit(Steinberg::Vst::ParamID id);
	Steinberg::tresult PLUGIN_API restartComponent(Steinberg::int32 flags);
	std::vector<std::string> GetPresets();
	void SetPreset(int i);
	bool HasEditor();
	void SaveState();
	void LoadState();
private:
	bool has_editor{ false };
	Steinberg::FUnknown* UnknownCast();
	Steinberg::IPluginFactory* factory;
	Steinberg::FObject* plugin;
	Steinberg::Vst::IComponent* processorComponent;
	Steinberg::Vst::IEditController* editController;
	Steinberg::Vst::IAudioProcessor* audio;
	Steinberg::Vst::ProcessData pd;
	Steinberg::Vst::AudioBusBuffers* abb_in{ nullptr };
	Steinberg::Vst::AudioBusBuffers* abb_out{ nullptr };

	Steinberg::int32 current_param_idx, offset;
	Steinberg::Vst::IParamValueQueue* current_queue;
};

#endif