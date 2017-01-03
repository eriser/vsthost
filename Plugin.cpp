#include "Plugin.h"

#include "Preset.h"
#include "PluginWindow.h"

namespace VSTHost {
const std::wstring Plugin::kPluginDirectory{ L".\\..\\plugins\\" };
Steinberg::Vst::TSamples Plugin::block_size = 128;
Steinberg::Vst::SampleRate Plugin::sample_rate = 44100.0;
Steinberg::Vst::SpeakerArrangement Plugin::speaker_arrangement = Steinberg::Vst::SpeakerArr::kStereo;

Plugin::Plugin(HMODULE m) : module(m) {

}

Plugin::~Plugin() {
	if (module)
		::FreeLibrary(module);
}

std::wstring Plugin::GetPluginFileName() {
	wchar_t buf[MAX_PATH] = { 0 };
	::GetModuleFileNameW(module, buf, MAX_PATH);
	std::wstring ret(buf);
	std::wstring::size_type pos = 0;
	if ((pos = ret.find_last_of('\\')) != std::basic_string<TCHAR>::npos)
		ret = ret.substr(pos + 1);
	return ret;
}

void Plugin::SetBlockSize(Steinberg::Vst::TSamples bs) {
	block_size = bs;
}

void Plugin::SetSampleRate(Steinberg::Vst::SampleRate sr) {
	sample_rate = sr;
}

void Plugin::SetSpeakerArrangement(Steinberg::Vst::SpeakerArrangement sa) {
	speaker_arrangement = sa;
}

void Plugin::SetActive(bool active_) {
	if (active != active_) {
		std::lock_guard<std::mutex> lock(processing);
		if (active = active_)
			Resume();
		else
			Suspend();
	}
}

bool Plugin::IsActive() {
	return active;
}

bool Plugin::IsBypassed() {
	return bypass;
}

bool Plugin::IsEditorVisible() {
	if (gui)
		return gui->IsActive();
	else
		return false;
}

void Plugin::ShowEditor() {
	if (gui)
		gui->Show(); 
}

void Plugin::HideEditor() {
	if (gui)
		gui->Hide();
}

void Plugin::SaveState() {
	if (state)
		state->GetState();
}

void Plugin::LoadState() {
	if (state)
		state->SetState();
}

void Plugin::SaveStateToFile() {
	if (state)
		state->SaveToFile();
}

void Plugin::LoadStateFromFile() {
	if (state)
		state->LoadFromFile();
}

Steinberg::uint32 Plugin::GetChannelCount() {
	return static_cast<Steinberg::uint32>(Steinberg::Vst::SpeakerArr::getChannelCount(speaker_arrangement));
}
} // namespace