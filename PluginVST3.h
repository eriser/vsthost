#ifndef PLUGINVST3_H
#define PLUGINVST3_H

#ifndef UNICODE
#define UNICODE_OFF
#endif
#include "base/source/fobject.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "pluginterfaces/vst/ivstmessage.h"
#include "pluginterfaces/vst/ivstunits.h"
#include "public.sdk/source/vst/hosting/parameterchanges.h"

#include "Plugin.h"

namespace VSTHost {
class PluginVST3Window;
class PresetVST3;
class Steinberg::IPluginFactory;
struct Steinberg::PClassInfo;
struct Steinberg::PClassInfo2;
class Steinberg::Vst::IComponent;
class Steinberg::Vst::IConnectionPoint;
class PluginVST3 : public Plugin, public Steinberg::FObject, public Steinberg::Vst::IComponentHandler {
	friend class PluginVST3Window;
	friend class PresetVST3;
public:
	// basic plugin interface
	PluginVST3(HMODULE m, Steinberg::IPluginFactory* f);
	~PluginVST3();
	bool IsValid() const;
	void Initialize();
	std::basic_string<TCHAR> GetPluginName() const;
	void Process(Steinberg::Vst::Sample32** input, Steinberg::Vst::Sample32** output);
	void UpdateBlockSize();
	void UpdateSampleRate();
	void UpdateSpeakerArrangement();
	// presets
	Steinberg::int32 GetProgramCount() const;
	void SetProgram(Steinberg::int32 id);
	// parameters
	Steinberg::int32 GetParameterCount() const;
	Steinberg::Vst::ParamValue GetParameter(Steinberg::Vst::ParamID id) const;
	void SetParameter(Steinberg::Vst::ParamID id, Steinberg::Vst::ParamValue value);
	// active and bypass flags
	void SetBypass(bool bypass_);
	bool BypassProcess() const;
	// editor
	bool HasEditor() const;
	void CreateEditor(HWND hWnd);

	void PrintFactory() const;
	void PrintClass(const Steinberg::PClassInfo& ci, int i) const;
	void PrintClass2(const Steinberg::PClassInfo2 &ci, int i) const;
	void PrintBusInfo() const;
	void PrintParameters() const;
	void PrintInfo() const;

	// vst3 specific
	Steinberg::tresult PLUGIN_API beginEdit(Steinberg::Vst::ParamID id);
	Steinberg::tresult PLUGIN_API performEdit(Steinberg::Vst::ParamID id, Steinberg::Vst::ParamValue valueNormalized);
	Steinberg::tresult PLUGIN_API endEdit(Steinberg::Vst::ParamID id);
	Steinberg::tresult PLUGIN_API restartComponent(Steinberg::int32 flags);
	OBJ_METHODS(PluginVST3, FObject)
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
	Steinberg::int32 class_index; // index of the class produced by factory which is valid 
	Steinberg::FUnknown* UnknownCast();
	Steinberg::IPluginFactory* factory;
	Steinberg::FObject* plugin;
	Steinberg::Vst::IComponent* processorComponent;
	Steinberg::Vst::IEditController* editController;
	bool processorComponent_initialized{ false };
	bool editController_initialized{ false };
	Steinberg::Vst::IConnectionPoint* iConnectionPointComponent{ nullptr };
	Steinberg::Vst::IConnectionPoint* iConnectionPointController{ nullptr };
	// audio related
	Steinberg::Vst::IAudioProcessor* audio;
	Steinberg::Vst::ProcessData pd;
};
} // namespace

#endif