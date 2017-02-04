#ifndef PLUGIN_H
#define PLUGIN_H

#define NOMINMAX
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
	virtual void Initialize(Steinberg::Vst::TSamples bs, Steinberg::Vst::SampleRate sr) = 0;
	virtual std::basic_string<TCHAR> GetPluginName() const = 0;
	std::string GetPluginFileName() const;
	virtual void Process(Steinberg::Vst::Sample32** input, Steinberg::Vst::Sample32** output, Steinberg::Vst::TSamples block_size) = 0;
	virtual void SetBlockSize(Steinberg::Vst::TSamples bs) = 0;
	virtual void SetSampleRate(Steinberg::Vst::SampleRate sr) = 0;
	// presets
	virtual Steinberg::int32 GetProgramCount() const = 0;
	virtual void SetProgram(Steinberg::int32 id) = 0;
	// parameters
	virtual Steinberg::int32 GetParameterCount() const = 0;
	virtual Steinberg::Vst::ParamValue GetParameter(Steinberg::Vst::ParamID id) const = 0;
	virtual void SetParameter(Steinberg::Vst::ParamID id, Steinberg::Vst::ParamValue value) = 0;
	// active and bypass flags
	void SetActive(bool active_);
	bool IsActive() const;
	bool IsBypassed() const;
	virtual void SetBypass(bool bypass_) = 0;
	virtual bool BypassProcess() const = 0;
	// editor
	virtual bool HasEditor() const = 0;
	virtual void CreateEditor(HWND hWnd) = 0;
	void ShowEditor();
	void HideEditor();
	bool IsEditorVisible() const;
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
	std::mutex plugin_lock; // locked when plugin is processing or setting itself (in)active
	bool active{ false };
	bool bypass{ false };
	Steinberg::Vst::TSamples block_size;
	Steinberg::Vst::SampleRate sample_rate;
	std::unique_ptr<Preset> state;
	std::unique_ptr<PluginWindow> gui;
};
} // namespace

#endif