#include "Plugin.h"
#include "PluginGUI.h"

Steinberg::Vst::TSamples Plugin::block_size = 128;
Steinberg::Vst::SampleRate Plugin::sample_rate = 44100.0;
Steinberg::Vst::SpeakerArrangement Plugin::speaker_arrangement = Steinberg::Vst::SpeakerArr::kStereo;

bool Plugin::IsGUIActive() {
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