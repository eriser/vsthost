#ifndef HOST_H
#define HOST_H

#include <cstdint>
#define NOMINMAX // kolizja makra MAX z windows.h oraz std::numeric_limits<T>::max()

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
	void Process(float** input, float** output);
	void Process(char* input, char* output);
	void Process(std::int8_t* input, std::int8_t* output);
	void Process(std::int16_t* input, std::int16_t* output);
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

	void ConvertFrom16Bits(std::int8_t* input, float** output);
	void ConvertFrom16Bits(std::int16_t* input, float** output);
	void ConvertTo16Bits(float** input, std::int8_t* output);
	void ConvertTo16Bits(float** input, std::int16_t* output);
	void test_conv(std::int16_t* input, std::int16_t* output);

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