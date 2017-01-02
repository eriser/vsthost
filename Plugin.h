#ifndef PLUGIN_H
#define PLUGIN_H

#include <windows.h>
#include <process.h>
#include <string>
#include <vector>
#include <iostream>
#include <memory>
#include <mutex>

#include "pluginterfaces/vst/vsttypes.h"
#include "Preset.h"

namespace VSTHost {
class PluginWindow;
class Plugin {
public:
	// basic plugin interface
	Plugin(HMODULE m);
	virtual ~Plugin();
	virtual bool IsValid() = 0;
	virtual void Initialize() = 0;
	virtual std::string GetPluginName() = 0;
	virtual void Process(Steinberg::Vst::Sample32** input, Steinberg::Vst::Sample32** output) = 0;
	virtual void UpdateBlockSize() = 0;
	virtual void UpdateSampleRate() = 0;
	virtual void UpdateSpeakerArrangement() = 0;
	static void SetBlockSize(Steinberg::Vst::TSamples bs);;
	static void SetSampleRate(Steinberg::Vst::SampleRate sr);
	static void SetSpeakerArrangement(Steinberg::Vst::SpeakerArrangement sa);
	// presets
	virtual Steinberg::int32 GetProgramCount() = 0;
	virtual void SetProgram(Steinberg::int32 id) = 0;
	// parameters
	virtual Steinberg::int32 GetParameterCount() = 0;
	virtual Steinberg::Vst::ParamValue GetParameter(Steinberg::Vst::ParamID id) = 0;
	virtual void SetParameter(Steinberg::Vst::ParamID id, Steinberg::Vst::ParamValue value) = 0;
	// active and bypass flags
	void SetActive(bool active_);
	bool IsActive();
	bool IsBypassed();
	virtual void SetBypass(bool bypass_) = 0;
	virtual bool BypassProcess() = 0;
	// editor
	virtual bool HasEditor() = 0;
	virtual void CreateEditor(HWND hWnd) = 0;
	void ShowEditor();
	void HideEditor();
	bool IsEditorVisible();
	// state
	void SaveState();
	void LoadState();
	void SaveStateToFile();
	void LoadStateFromFile();

	virtual void PrintInfo() = 0;
protected:
	virtual void Resume() = 0;
	virtual void Suspend() = 0;
	virtual void StartProcessing() = 0;
	virtual void StopProcessing() = 0;
	static Steinberg::uint32 GetChannelCount();

	HMODULE module;
	std::mutex processing; // locked when plugin is processing or setting itself (in)active
	bool active{ false };
	bool bypass{ false };
	static Steinberg::Vst::TSamples block_size;
	static Steinberg::Vst::SampleRate sample_rate;
	static Steinberg::Vst::SpeakerArrangement speaker_arrangement;
	std::unique_ptr<Preset> state;
	std::unique_ptr<PluginWindow> gui;
};
} // namespace

#endif