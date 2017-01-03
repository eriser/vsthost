#ifndef PLUGINVST2_H
#define PLUGINVST2_H

#ifndef VST_FORCE_DEPRECATED
#define VST_FORCE_DEPRECATED 0
#endif

#include "pluginterfaces\vst2.x\aeffect.h"

#include "Plugin.h"

namespace VSTHost {
class PluginVST2Window;
class PresetVST3;
class PluginVST2 : public Plugin {
	friend class PluginVST2Window;
	friend class PresetVST2;
public:
	// basic plugin interface
	PluginVST2(HMODULE m, AEffect* p);
	~PluginVST2();
	bool IsValid();
	void Initialize();
	std::basic_string<TCHAR> GetPluginName();
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
	// vst2 callback procedure wrapper
	static VstIntPtr VSTCALLBACK HostCallbackWrapper(AEffect *effect, VstInt32 opcode, VstInt32 index, VstInt32 value, void *ptr, float opt);
	
	void PrintPrograms();
	void PrintParameters();
	void PrintCanDos();
	void PrintInfo();
private:
	void Resume();
	void Suspend();
	void StartProcessing();
	void StopProcessing();
	// vst2 specific
	VstIntPtr VSTCALLBACK Dispatcher(VstInt32 opcode, VstInt32 index = 0, VstIntPtr value = 0, void* ptr = nullptr, float opt = 0.);
	VstIntPtr VSTCALLBACK HostCallback(AEffect* effect, VstInt32 opcode, VstInt32 index, VstInt32 value, void *ptr, float opt);
	bool CanDo(const char *canDo);
	int GetVendorVersion();
	int GetVSTVersion();
	Steinberg::int32 GetFlags();
	// soft bypass
	bool soft_bypass{ false };
	std::unique_ptr<AEffect> plugin;
};
} // namespace

#endif