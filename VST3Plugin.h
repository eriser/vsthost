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
	OBJ_METHODS(VST3Plugin, FObject)
	REFCOUNT_METHODS(FObject)
	DEFINE_INTERFACES
		DEF_INTERFACE(IComponentHandler)
	END_DEFINE_INTERFACES(FObject)
	VST3Plugin(HMODULE m, Steinberg::IPluginFactory* f);
	~VST3Plugin();
	bool IsValid();
	void Initialize();
	void Process(Steinberg::Vst::Sample32** input, Steinberg::Vst::Sample32** output);
	void ProcessOutputParameterChanges();
	void CreateEditor(HWND hWnd);
	void PrintInfo();
	void PrintFactory();
	void PrintClass(const Steinberg::PClassInfo& ci, int i);
	void PrintClass2(const Steinberg::PClassInfo2 &ci, int i);
	void PrintBusInfo();
	void PrintParameters();
	std::string GetPluginName();
	Steinberg::IPlugView * CreateView();
	Steinberg::tresult PLUGIN_API beginEdit(Steinberg::Vst::ParamID id);
	Steinberg::tresult PLUGIN_API performEdit(Steinberg::Vst::ParamID id, Steinberg::Vst::ParamValue valueNormalized);
	Steinberg::tresult PLUGIN_API endEdit(Steinberg::Vst::ParamID id);
	Steinberg::tresult PLUGIN_API restartComponent(Steinberg::int32 flags);
	std::vector<std::string> GetPresets();
	bool HasEditor();
	Steinberg::uint32 GetProgramCount();
	void SetProgram(Steinberg::int32 id);
	Steinberg::uint32 GetParameterCount();
	Steinberg::Vst::ParamValue GetParameter(Steinberg::Vst::ParamID id);
	void SetParameter(Steinberg::Vst::ParamID id, Steinberg::Vst::ParamValue value);
	void SaveState();
	void LoadState();
	void UpdateBlockSize();
	void UpdateSampleRate();
	void UpdateSpeakerArrangement();
	void SetBypass(bool bypass_);
	void Resume();
	void Suspend();
private:
	Steinberg::Vst::ParamID bypass_param_id{ -1 };
	Steinberg::Vst::ParamID program_change_param_id{ -1 };
	Steinberg::int32 program_change_param_idx{ -1 };
	void StartProcessing();
	void StopProcessing();
	bool BypassProcess();
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

	Steinberg::Vst::IUnitInfo* unit_info{ nullptr };
	Steinberg::uint32 program_count{ 0 };
	Steinberg::Vst::ProgramListID program_list_root{ Steinberg::Vst::kNoProgramListId };

};

#endif