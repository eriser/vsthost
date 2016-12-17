#include "pluginterfaces\vst2.x\aeffect.h"
#include "pluginterfaces\vst2.x\aeffectx.h"

#include "VSTBase.h"
#include "Plugin.h"

#ifndef VSTPLUGIN_H
#define VSTPLUGIN_H

class VSTPlugin : public Plugin, public VSTBase {
	//interfejs
public:
	VSTPlugin(HMODULE m, AEffect* plugin);
	~VSTPlugin();
	VstIntPtr VSTCALLBACK HostCallback(AEffect *effect, VstInt32 opcode, VstInt32 index, VstInt32 value, void *ptr, float opt);
	static VstIntPtr VSTCALLBACK HostCallbackWrapper(AEffect *effect, VstInt32 opcode, VstInt32 index, VstInt32 value, void *ptr, float opt);
	void CreateEditor(HWND hWnd);
	void InThread();
	bool IsVST3() { return false; }
	bool isVST() { return true; }
	void StartPlugin();
	void Process(float **input, float **output);
	bool IsValid();
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
protected:
	bool soft_bypass{ false };
	void StartProcessing();
	void StopProcessing();
	bool BypassProcess();
};

#endif