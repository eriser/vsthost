#ifndef HOST_H
#define HOST_H

#include <vector>
#include <iostream>
#include <fstream>
#include <string>

#include "base/source/fobject.h"
#include "base/source/fstring.h"
#include "pluginterfaces/base/ipluginbase.h"
#include "pluginterfaces/vst/ivsthostapplication.h"
//DEF_CLASS_IID(Steinberg::Vst::IHostApplication)

#include "PluginLoader.h"
#include "VSTPlugin.h"
#include "VST3Plugin.h"

class Host : public Steinberg::FObject, Steinberg::Vst::IHostApplication {
public:
	Host(Steinberg::Vst::TSamples bs, Steinberg::Vst::SampleRate sr, Steinberg::Vst::SpeakerArrangement sa = Steinberg::Vst::SpeakerArr::kStereo);
	~Host();
	Steinberg::tresult LoadPlugin(std::string path);
	void Process(Steinberg::Vst::Sample32** input, Steinberg::Vst::Sample32** output);
	void Process(Steinberg::int8* input, Steinberg::int8* output);
	void Process(Steinberg::int16* input, Steinberg::int16* output);
	void SetSampleRate(Steinberg::Vst::SampleRate sr);
	void SetBlockSize(Steinberg::Vst::TSamples bs);
	void SetSpeakerArrangement(Steinberg::Vst::SpeakerArrangement sa);
	void SetActive(bool ia);

	Steinberg::tresult PLUGIN_API getName(Steinberg::Vst::String128 name);
	Steinberg::tresult PLUGIN_API createInstance(Steinberg::TUID cid, Steinberg::TUID iid, void** obj);

	OBJ_METHODS(Host, FObject)
	REFCOUNT_METHODS(FObject)
	DEFINE_INTERFACES
		DEF_INTERFACE(IHostApplication)
	END_DEFINE_INTERFACES(FObject)

private:
	void LoadPluginList();
	Steinberg::uint32 GetChannelCount();
	void AllocateBuffers();
	void FreeBuffers();

	void ConvertFrom16Bits(Steinberg::int8* input, Steinberg::Vst::Sample32** output);
	void ConvertFrom16Bits(Steinberg::int16* input, Steinberg::Vst::Sample32** output);
	void ConvertTo16Bits(Steinberg::Vst::Sample32** input, Steinberg::int8* output);
	void ConvertTo16Bits(Steinberg::Vst::Sample32** input, Steinberg::int16* output);

	const static std::string kPluginsPath;
	std::thread ui_thread;
	std::vector<Plugin*> plugins;
	Steinberg::Vst::TSamples block_size;
	Steinberg::Vst::SampleRate sample_rate;
	Steinberg::Vst::SpeakerArrangement speaker_arrangement;
	Steinberg::Vst::Sample32** buffers[2];
	bool is_active{ false };
	
};

#endif