#include "pluginterfaces\vst2.x\aeffect.h"
#include "pluginterfaces\vst2.x\aeffectx.h"

#include "VSTBase.h"
#include "Plugin.h"

#ifndef VSTPLUGIN_H
#define VSTPLUGIN_H

class VSTPlugin : public Plugin, public VSTBase {
	//interfejs
public:
	VSTPlugin(HMODULE m, AEffect* plugin, Steinberg::Vst::TSamples& bs, Steinberg::Vst::SampleRate& sr, Steinberg::Vst::SpeakerArrangement& sa);
	~VSTPlugin();
	static VstIntPtr VSTCALLBACK hostCallback(AEffect *effect, VstInt32 opcode, VstInt32 index, VstInt32 value, void *ptr, float opt);
	void CreateEditor(HWND hWnd);
	void InThread();
	bool IsVST3() { return false; }
	bool isVST() { return true; }
	void ResumePlugin();
	void SuspendPlugin();
	void SetSampleRate(float sampleRate);
	void SetBlockSize(int blockSize);
	void StartPlugin();
	void Process(float **input, float **output);
	bool IsValid();
	void PrintPrograms();
	void PrintParameters();
	bool CanDo(char *canDo);
	void PrintCanDos();
	int GetVendorVersion();
	int GetVSTVersion();
	void PrintInfo();
	std::string GetPluginName();
	std::vector<std::string> GetPresets() ;
	void SetPreset(int i);
};

#endif