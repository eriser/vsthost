#ifndef PLUGINVST3_H
#define PLUGINVST3_H

#undef max
#include <limits>

#ifndef UNICODE
#define UNICODE_OFF
#endif
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "pluginterfaces/vst/ivstmessage.h"
#include "pluginterfaces/vst/ivstunits.h"

#include "Plugin.h"
#include "PluginLoader.h"

namespace VSTHost {
class PluginVST3Window;
class PresetVST3;
class Steinberg::IPluginFactory;
struct Steinberg::PClassInfo;
struct Steinberg::PClassInfo2;
class Steinberg::Vst::IComponent;
class Steinberg::Vst::IConnectionPoint;
class ParameterValueQueue;
class PluginVST3 : public Plugin, public Steinberg::Vst::IComponentHandler {
	friend class PluginVST3Window;
	friend class PresetVST3;
	friend std::unique_ptr<Plugin> PluginLoader::Load(const std::string& path, Steinberg::FUnknown* context);
public:
	// basic plugin interface
	PluginVST3(HMODULE m, Steinberg::IPluginFactory* f, Steinberg::FUnknown* c);
	~PluginVST3();
	bool IsValid() const;
	void Initialize(Steinberg::Vst::TSamples bs, Steinberg::Vst::SampleRate sr);
	std::basic_string<TCHAR> GetPluginName() const;
	void Process(Steinberg::Vst::Sample32** input, Steinberg::Vst::Sample32** output, Steinberg::Vst::TSamples block_size);
	void SetBlockSize(Steinberg::Vst::TSamples bs);
	void SetSampleRate(Steinberg::Vst::SampleRate sr);
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

	// vst3 interfaces specific
	Steinberg::tresult PLUGIN_API beginEdit(Steinberg::Vst::ParamID id);
	Steinberg::tresult PLUGIN_API performEdit(Steinberg::Vst::ParamID id, Steinberg::Vst::ParamValue valueNormalized);
	Steinberg::tresult PLUGIN_API endEdit(Steinberg::Vst::ParamID id);
	Steinberg::tresult PLUGIN_API restartComponent(Steinberg::int32 flags);
	Steinberg::tresult PLUGIN_API queryInterface(const Steinberg::TUID _iid, void** obj);
	Steinberg::uint32 PLUGIN_API addRef();
	Steinberg::uint32 PLUGIN_API release();
private:
	void Resume();
	void Suspend();
	void StartProcessing();
	void StopProcessing();

	Steinberg::FUnknown* context;
	// can stereo
	bool can_stereo{ false };
	// soft bypass
	Steinberg::Vst::ParamID bypass_param_id{ std::numeric_limits<std::uint32_t>::max() };
	// vst3 presets handling
	Steinberg::Vst::ParamID program_change_param_id{ std::numeric_limits<std::uint32_t>::max() };
	Steinberg::int32 program_change_param_idx{ -1 };
	Steinberg::Vst::IUnitInfo* unit_info{ nullptr };
	Steinberg::int32 program_count{ 0 };
	Steinberg::Vst::ProgramListID program_list_root{ Steinberg::Vst::kNoProgramListId };
	// parameter changes
	void ProcessOutputParameterChanges();
	Steinberg::int32 current_param_idx, offset;
	ParameterValueQueue* current_queue;
	// has editor flag for optimization
	bool has_editor{ false };
	// vst3 general
	Steinberg::int32 class_index; // index of the class produced by factory which is valid 
	Steinberg::IPluginFactory* factory;
	Steinberg::FUnknown* plugin;
	Steinberg::Vst::IComponent* processor_component;
	Steinberg::Vst::IEditController* edit_controller;
	bool processor_component_initialized{ false };
	bool edit_controller_initialized{ false };
	Steinberg::Vst::IConnectionPoint* connection_point_component{ nullptr };
	Steinberg::Vst::IConnectionPoint* connection_point_controller{ nullptr };
	// audio related
	Steinberg::Vst::IAudioProcessor* audio;
	Steinberg::Vst::ProcessData pd;
};
} // namespace

#endif