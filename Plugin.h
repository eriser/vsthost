#ifndef PLUGIN_H
#define PLUGIN_H

#include <windows.h>
#include <process.h>
#include <string>
#include <vector>
#include <iostream>

#include "pluginterfaces/vst/vsttypes.h"


class Plugin {
public:
	Plugin(HMODULE m, Steinberg::Vst::TSamples &bs, Steinberg::Vst::SampleRate &sr, Steinberg::Vst::SpeakerArrangement &sa)
		: module(m), block_size(bs), sample_rate(sr), speaker_arrangement(sa) {}
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
protected:
	HMODULE module;
	Steinberg::Vst::TSamples &block_size;
	Steinberg::Vst::SampleRate &sample_rate;
	Steinberg::Vst::SpeakerArrangement &speaker_arrangement;
};

#endif