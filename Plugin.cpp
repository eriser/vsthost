#include "Plugin.h"

Steinberg::Vst::TSamples Plugin::block_size = 128;
Steinberg::Vst::SampleRate Plugin::sample_rate = 44100.0;
Steinberg::Vst::SpeakerArrangement Plugin::speaker_arrangement = Steinberg::Vst::SpeakerArr::kStereo;