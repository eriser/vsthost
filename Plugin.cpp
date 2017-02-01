#include "Plugin.h"

#include "Preset.h"
#include "PluginWindow.h"

namespace VSTHost {
const std::string Plugin::kPluginDirectory{ "plugins\\" };

Plugin::Plugin(HMODULE m, Steinberg::Vst::TSamples bs, Steinberg::Vst::SampleRate sr) : module(m), block_size(bs), sample_rate(sr) {

}

Plugin::~Plugin() {
	if (module)
		::FreeLibrary(module);
}

std::string Plugin::GetPluginFileName() const {
	char buf[MAX_PATH] = { 0 };
	::GetModuleFileNameA(module, buf, MAX_PATH);
	std::string ret(buf);
	std::string::size_type pos = 0;
	if ((pos = ret.find_last_of('\\')) != std::string::npos)
		ret = ret.substr(pos + 1);
	return ret;
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

bool Plugin::IsActive() const {
	return active;
}

bool Plugin::IsBypassed() const {
	return bypass;
}

bool Plugin::IsEditorVisible() const {
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
	return 2;
}
} // namespace