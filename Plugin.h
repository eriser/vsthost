#ifndef PLUGIN_H
#define PLUGIN_H

#include <Windows.h>
#include <memory>
#include <string>
#include <mutex>

#ifndef UNICODE
#define UNICODE_OFF
#endif
#include "pluginterfaces/vst/vsttypes.h"

namespace VSTHost {
class PluginWindow;
class Preset;
class Plugin {
public:
	// basic plugin interface
	Plugin(HMODULE m);
	virtual ~Plugin();
	virtual bool IsValid() const = 0;
	virtual void Initialize() = 0;
	virtual std::basic_string<TCHAR> GetPluginName() const = 0;
	std::string GetPluginFileName() const;
	virtual void Process(Steinberg::Vst::Sample32** input, Steinberg::Vst::Sample32** output) = 0;
	virtual void UpdateBlockSize() = 0;
	virtual void UpdateSampleRate() = 0;
	virtual void UpdateSpeakerArrangement() = 0;
	static void SetBlockSize(Steinberg::Vst::TSamples bs);;
	static void SetSampleRate(Steinberg::Vst::SampleRate sr);
	static void SetSpeakerArrangement(Steinberg::Vst::SpeakerArrangement sa);
	// presets
	virtual Steinberg::int32 GetProgramCount() const = 0;
	virtual void SetProgram(Steinberg::int32 id) = 0;
	// parameters
	virtual Steinberg::int32 GetParameterCount() const = 0;
	virtual Steinberg::Vst::ParamValue GetParameter(Steinberg::Vst::ParamID id) const = 0;
	virtual void SetParameter(Steinberg::Vst::ParamID id, Steinberg::Vst::ParamValue value) = 0;
	// active and bypass flags
	void SetActive(bool active_);
	bool IsActive();
	bool IsBypassed();
	virtual void SetBypass(bool bypass_) = 0;
	virtual bool BypassProcess() const = 0;
	// editor
	virtual bool HasEditor() const = 0;
	virtual void CreateEditor(HWND hWnd) = 0;
	void ShowEditor();
	void HideEditor();
	bool IsEditorVisible();
	// state
	void SaveState();
	void LoadState();
	void SaveStateToFile();
	void LoadStateFromFile();

	const static std::string Plugin::kPluginDirectory;
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