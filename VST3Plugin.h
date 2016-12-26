#ifndef VST3PLUGIN_H
#define VST3PLUGIN_H

#include "base/source/fobject.h"
#include "pluginterfaces/base/ipluginbase.h"
#include "pluginterfaces/vst/ivstcomponent.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"

#include "pluginterfaces/vst/ivstunits.h"

#include "public.sdk/source/vst/hosting/parameterchanges.h"
//DEF_CLASS_IID(Steinberg::Vst::IComponent)
//DEF_CLASS_IID(Steinberg::Vst::IComponentHandler)
//DEF_CLASS_IID(Steinberg::Vst::IAudioProcessor)
//#include <Windows.h>

#include "Plugin.h"

class VST3PluginGUI;
class VST3Plugin : public Plugin, public Steinberg::FObject, public Steinberg::Vst::IComponentHandler {
	friend class VST3PluginGUI;
	// extern "C" ?
	typedef bool (PLUGIN_API *ExitModuleProc)();
public:
	// basic plugin interface
	VST3Plugin(HMODULE m, Steinberg::IPluginFactory* f);
	~VST3Plugin();
	bool IsValid();
	void Initialize();
	std::string GetPluginName();
	void Process(Steinberg::Vst::Sample32** input, Steinberg::Vst::Sample32** output);
	void UpdateBlockSize();
	void UpdateSampleRate();
	void UpdateSpeakerArrangement();
	// presets
	Steinberg::int32 GetProgramCount();
	void SetProgram(Steinberg::int32 id);
	// parameters
	Steinberg::int32 GetParameterCount();
	Steinberg::Vst::ParamValue GetParameter(Steinberg::Vst::ParamID id);
	void SetParameter(Steinberg::Vst::ParamID id, Steinberg::Vst::ParamValue value);
	// active and bypass flags
	void SetBypass(bool bypass_);
	bool BypassProcess();
	// editor
	bool HasEditor();
	void CreateEditor(HWND hWnd);

	void PrintFactory();
	void PrintClass(const Steinberg::PClassInfo& ci, int i);
	void PrintClass2(const Steinberg::PClassInfo2 &ci, int i);
	void PrintBusInfo();
	void PrintParameters();
	void PrintInfo();

	// vst3 specific
	Steinberg::IPlugView* CreateView();
	Steinberg::tresult PLUGIN_API beginEdit(Steinberg::Vst::ParamID id);
	Steinberg::tresult PLUGIN_API performEdit(Steinberg::Vst::ParamID id, Steinberg::Vst::ParamValue valueNormalized);
	Steinberg::tresult PLUGIN_API endEdit(Steinberg::Vst::ParamID id);
	Steinberg::tresult PLUGIN_API restartComponent(Steinberg::int32 flags);
	OBJ_METHODS(VST3Plugin, FObject)
		REFCOUNT_METHODS(FObject)
		DEFINE_INTERFACES
		DEF_INTERFACE(IComponentHandler)
	END_DEFINE_INTERFACES(FObject)
private:
	void Resume();
	void Suspend();
	void StartProcessing();
	void StopProcessing();

	// soft bypass
	Steinberg::Vst::ParamID bypass_param_id{ -1 };
	// vst3 presets handling
	Steinberg::Vst::ParamID program_change_param_id{ -1 };
	Steinberg::int32 program_change_param_idx{ -1 };
	Steinberg::Vst::IUnitInfo* unit_info{ nullptr };
	Steinberg::int32 program_count{ 0 };
	Steinberg::Vst::ProgramListID program_list_root{ Steinberg::Vst::kNoProgramListId };
	// parameter changes
	void ProcessOutputParameterChanges();
	Steinberg::int32 current_param_idx, offset;
	Steinberg::Vst::IParamValueQueue* current_queue;
	// has editor flag for optimization
	bool has_editor{ false };
	// vst3 general
	Steinberg::FUnknown* UnknownCast();
	Steinberg::IPluginFactory* factory;
	Steinberg::FObject* plugin;
	Steinberg::Vst::IComponent* processorComponent;
	Steinberg::Vst::IEditController* editController;
	// audio related
	Steinberg::Vst::IAudioProcessor* audio;
	Steinberg::Vst::ProcessData pd;
	Steinberg::Vst::AudioBusBuffers* abb_in{ nullptr };
	Steinberg::Vst::AudioBusBuffers* abb_out{ nullptr };
};

#endif