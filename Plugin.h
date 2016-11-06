#ifndef PLUGIN_H
#define PLUGIN_H

#include <windows.h>
#include <process.h>
#include <string>
#include <iostream>

#include "pluginterfaces/vst/vsttypes.h"

#include "Editor.h"

class Plugin {
public:
	Plugin(HMODULE m, Steinberg::Vst::TSamples &bs, Steinberg::Vst::SampleRate &sr, Steinberg::Vst::SpeakerArrangement &sa)
		: module(m), editor(nullptr), block_size(bs), sample_rate(sr), speaker_arrangement(sa) {}
	virtual ~Plugin() { if (editor) delete editor;  }
	virtual void Process(Steinberg::Vst::Sample32** input, Steinberg::Vst::Sample32** output) = 0;
	virtual bool IsValid() = 0;
	virtual void PrintInfo() = 0;
	virtual std::string GetPluginName() = 0;
	void ShowEditor() { if (editor) editor->Show(); }
	virtual void CreateEditor() = 0;
	virtual void InThread() = 0;
protected:
	std::thread ui_thread;
	HMODULE module;
	Editor* editor;
	Steinberg::Vst::TSamples &block_size;
	Steinberg::Vst::SampleRate &sample_rate;
	Steinberg::Vst::SpeakerArrangement &speaker_arrangement;
};

#endif