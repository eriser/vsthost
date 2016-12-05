#ifndef PLUGIN_H
#define PLUGIN_H

#include <windows.h>
#include <process.h>
#include <string>
#include <vector>
#include <iostream>

#include "pluginterfaces/vst/vsttypes.h"
#include "Preset.h"
class Plugin {
public:
	Plugin(HMODULE m) : module(m) {}
	virtual ~Plugin() {   }
	virtual void Process(Steinberg::Vst::Sample32** input, Steinberg::Vst::Sample32** output) = 0;
	virtual bool IsValid() = 0;
	virtual void PrintInfo() = 0;
	virtual std::string GetPluginName() = 0;
	virtual bool IsVST3() = 0;
	virtual bool isVST() = 0;
	virtual std::vector<std::string> GetPresets() = 0;
	virtual void SetPreset(int i) = 0;
	virtual bool HasEditor() = 0;
	virtual void SaveState() { if (state) state->GetState(); }
	virtual void LoadState() { if (state) state->SetState(); }
	virtual void SaveStateToFile() { if (state) state->SaveToFile(); }
	virtual void LoadStateFromFile() { if (state) state->LoadFromFile(); }
	static void SetBlockSize(Steinberg::Vst::TSamples bs) { block_size = bs; }
	static void SetSampleRate(Steinberg::Vst::SampleRate sr) { sample_rate = sr; }
	static void SetSpeakerArrangement(Steinberg::Vst::SpeakerArrangement sa) { speaker_arrangement = sa; }
protected:
	HMODULE module;
	static Steinberg::Vst::TSamples block_size;
	static Steinberg::Vst::SampleRate sample_rate;
	static Steinberg::Vst::SpeakerArrangement speaker_arrangement;
	Preset* state;
};

#endif