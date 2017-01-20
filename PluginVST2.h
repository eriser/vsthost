#ifndef PLUGINVST2_H
#define PLUGINVST2_H

#ifndef VST_FORCE_DEPRECATED
#define VST_FORCE_DEPRECATED 0
#endif

#include "pluginterfaces\vst2.x\aeffect.h"

#include "Plugin.h"
#include "PluginLoader.h"

namespace VSTHost {
class PluginVST2Window;
class PresetVST3;
class PluginVST2 : public Plugin {
	friend class PluginVST2Window;
	friend class PresetVST2;
	friend std::unique_ptr<Plugin> PluginLoader::Load(const std::string& path, Steinberg::Vst::TSamples bs, Steinberg::Vst::SampleRate sr);
	PluginVST2(HMODULE m, AEffect* p, Steinberg::Vst::TSamples bs, Steinberg::Vst::SampleRate sr);
public:
	~PluginVST2();
	// basic plugin interface
	bool IsValid() const;
	void Initialize();
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
	// vst2 callback procedure wrapper
	static VstIntPtr VSTCALLBACK HostCallbackWrapper(AEffect *effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void *ptr, float opt);
private:
	void Resume();
	void Suspend();
	void StartProcessing();
	void StopProcessing();
	// vst2 specific
	VstIntPtr VSTCALLBACK Dispatcher(VstInt32 opcode, VstInt32 index = 0, VstIntPtr value = 0, void* ptr = nullptr, float opt = 0.);
	VstIntPtr VSTCALLBACK HostCallback(AEffect* effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void *ptr, float opt);
	bool CanDo(const char *canDo) const;
	Steinberg::int32 GetVendorVersion() const;
	Steinberg::int32 GetVSTVersion() const;
	Steinberg::int32 GetFlags() const;
	// soft bypass
	bool soft_bypass{ false };
	std::unique_ptr<AEffect> plugin;
};
} // namespace

#endif