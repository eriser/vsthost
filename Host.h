#ifndef HOST_H
#define HOST_H

#include <cstdint>
#include <vector>
#include <string>
#include <thread>
#include <memory>

#ifndef UNICODE
#define UNICODE_OFF
#endif
#include "base/source/fobject.h"
#include "pluginterfaces/vst/ivsthostapplication.h"

#define NOMINMAX // kolizja makra MAX z windows.h oraz std::numeric_limits<T>::max()

namespace VSTHost {
class Plugin;
class HostWindow;
class Host : public Steinberg::FObject, Steinberg::Vst::IHostApplication {
	friend class HostWindow;
public:
	Host(std::int64_t block_size, double sample_rate);
	~Host();
	bool LoadPlugin(std::string path);
	void Process(float** input, float** output);
	void Process(char* input, char* output);
	void Process(std::int8_t* input, std::int8_t* output);
	void Process(std::int16_t* input, std::int16_t* output);
	void SetSampleRate(Steinberg::Vst::SampleRate sr);
	void SetBlockSize(Steinberg::Vst::TSamples bs);

	Steinberg::tresult PLUGIN_API getName(Steinberg::Vst::String128 name);
	Steinberg::tresult PLUGIN_API createInstance(Steinberg::TUID cid, Steinberg::TUID iid, void** obj);

	OBJ_METHODS(Host, FObject)
	REFCOUNT_METHODS(FObject)
	DEFINE_INTERFACES
		DEF_INTERFACE(IHostApplication)
	END_DEFINE_INTERFACES(FObject)

	void CreateGUIThread();
	void CreateGUI();
	bool LoadPluginList(std::string path = kPluginList);
	bool SavePluginList(std::string path = kPluginList);
	const static std::string kPluginList;
private:
	void SwapPlugins(size_t i, size_t j);
	void DeletePlugin(size_t i);
	Steinberg::uint32 GetChannelCount() const;
	void AllocateBuffers();
	void FreeBuffers();

	void ConvertFrom16Bits(std::int8_t* input, float** output);
	void ConvertFrom16Bits(std::int16_t* input, float** output);
	void ConvertTo16Bits(float** input, std::int8_t* output);
	void ConvertTo16Bits(float** input, std::int16_t* output);

	std::vector<std::unique_ptr<Plugin>> plugins;
	std::unique_ptr<HostWindow> gui;
	std::thread ui_thread;

	Steinberg::Vst::TSamples block_size;
	Steinberg::Vst::SampleRate sample_rate;
	Steinberg::Vst::Sample32** buffers[2];
};
}

#endif