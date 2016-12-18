#ifndef VSTPLUGIN_H
#define VSTPLUGIN_H

#ifndef VST_FORCE_DEPRECATED
#define VST_FORCE_DEPRECATED 0
#endif

#include "pluginterfaces\vst2.x\aeffect.h"
#include "pluginterfaces\vst2.x\aeffectx.h"

#include "VSTBase.h"
#include "Plugin.h"

class VSTPluginGUI;
class VSTPlugin : public Plugin {
	friend class VSTPluginGUI;
public:
	VSTPlugin(HMODULE m, AEffect* p);
	~VSTPlugin();
	bool IsValid();
	void Initialize();
	VstIntPtr VSTCALLBACK HostCallback(AEffect* effect, VstInt32 opcode, VstInt32 index, VstInt32 value, void *ptr, float opt);
	static VstIntPtr VSTCALLBACK HostCallbackWrapper(AEffect *effect, VstInt32 opcode, VstInt32 index, VstInt32 value, void *ptr, float opt);
	VstIntPtr VSTCALLBACK Dispatcher(VstInt32 opcode, VstInt32 index = 0, VstIntPtr value = 0, void* ptr = nullptr, float opt = 0.);
	void CreateEditor(HWND hWnd);
	void Process(Steinberg::Vst::Sample32** input, Steinberg::Vst::Sample32** output);
	void PrintPrograms();
	void PrintParameters();
	bool CanDo(const char *canDo);
	void PrintCanDos();
	int GetVendorVersion();
	int GetVSTVersion();
	void PrintInfo();
	std::string GetPluginName();
	std::vector<std::string> GetPresets() ;
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
	AEffect* GetAEffect() { return plugin; }; // temporary
protected:
	bool soft_bypass{ false };
	void StartProcessing();
	void StopProcessing();
	bool BypassProcess();
private:
	AEffect* plugin;
};

#endif