#include "Host.h"

#define NOMINMAX // kolizja makra MAX z windows.h oraz std::numeric_limits<T>::max()
#include <limits>
#include <cstring>
#include <thread>

#ifndef UNICODE
#define UNICODE_OFF
#endif
#include "base/source/fstring.h"
#include "pluginterfaces/vst/ivsthostapplication.h"

#include "HostWindow.h"
#include "Plugin.h"
#include "PluginManager.h"

namespace VSTHost {
class Host::HostImpl : Steinberg::Vst::IHostApplication {
public:
	HostImpl(std::int64_t block_size, double sample_rate)
		: block_size(block_size), sample_rate(sample_rate), plugins(block_size, sample_rate) {
		buffers[0] = nullptr;
		buffers[1] = nullptr;
		AllocateBuffers();
	}

	~HostImpl() {
		FreeBuffers();
	}

	void Process(float** input, float** output, std::int64_t block_size) {
		std::lock_guard<std::mutex> lock(plugins.GetLock());
		if (plugins.Size() == 1) {
			plugins.Front().Process(input, output, block_size);
		}
		else if (plugins.Size() > 1) {
			plugins.Front().Process(input, buffers[1], block_size);
			unsigned i, last_processed = 1;
			for (i = 1; i < plugins.Size() - 1; i++) {
				last_processed = (i + 1) % 2;
				plugins[i].Process(buffers[i % 2], buffers[last_processed], block_size);
			}
			plugins.Back().Process(buffers[last_processed], output, block_size);
		}
		else {
			for (unsigned i = 0; i < GetChannelCount(); ++i) {
				std::memcpy(output[i], input[i], sizeof(input[0][0]) * block_size);
			}
		}
	}

	void Process(char* input, char* output, std::int64_t block_size) {
		Process(reinterpret_cast<std::int16_t*>(input), reinterpret_cast<std::int16_t*>(output), block_size);
	}

	void Process(std::int16_t* input, std::int16_t* output, std::int64_t block_size) {
		std::lock_guard<std::mutex> lock(plugins.GetLock());
		if (plugins.Size() == 0) {
			std::memcpy(output, input, block_size * 2 * GetChannelCount());
			return;
		}
		ConvertFrom16Bits(input, buffers[0]);
		if (plugins.Size() == 1) {
			plugins.Front().Process(buffers[0], buffers[1], block_size);
			ConvertTo16Bits(buffers[1], output);
		}
		else if (plugins.Size() > 1) {
			unsigned last_processed = 0;			// remember where the most recently processed buffer is,
			for (unsigned i = 0; i < plugins.Size(); ++i) {	// so that it could be moved to output.
				last_processed = (i + 1) % 2;
				plugins[i].Process(buffers[i % 2], buffers[last_processed], block_size);
			}
			ConvertTo16Bits(buffers[last_processed], output);
		}
	}

	void SetSampleRate(Steinberg::Vst::SampleRate sr) {
		sample_rate = sr;
		plugins.SetSampleRate(sample_rate);
		for (decltype(plugins.Size()) i = 0; i < plugins.Size(); ++i)
			plugins.GetAt(i).SetSampleRate(sample_rate);
	}

	void SetBlockSize(Steinberg::Vst::TSamples bs) {
		if (bs != block_size) {
			block_size = bs;
			plugins.SetBlockSize(block_size);
			FreeBuffers();
			AllocateBuffers();
			for (decltype(plugins.Size()) i = 0; i < plugins.Size(); ++i)
				plugins[i].SetBlockSize(block_size);
		}
	}

	void CreateGUIThread() {
		std::thread gui_thread(&Host::HostImpl::CreateGUI, this);
		gui_thread.detach();
	}

	void CreateGUI() {
		gui = std::unique_ptr<HostWindow>(new HostWindow(plugins));
		gui->Initialize(NULL);
		gui->Go();
	}

	bool LoadPluginList(const std::string& path) {
		return plugins.LoadPluginList(path);
	}

	bool SavePluginList(const std::string& path) {
		return plugins.SavePluginList(path);
	}

	bool LoadPluginList() {
		return plugins.LoadPluginList();
	}

	bool SavePluginList() {
		return plugins.SavePluginList();
	}

	Steinberg::tresult PLUGIN_API getName(Steinberg::Vst::String128 name) {
		Steinberg::String str("VSTHost");
		str.copyTo16(name, 0, 127);
		return Steinberg::kResultTrue;
	}

	Steinberg::tresult PLUGIN_API createInstance(Steinberg::TUID cid, Steinberg::TUID iid, void** obj) {
		*obj = nullptr;
		return Steinberg::kResultFalse;
	}

	Steinberg::tresult PLUGIN_API queryInterface(const Steinberg::TUID _iid, void** obj) {
		QUERY_INTERFACE(_iid, obj, Steinberg::FUnknown::iid, Steinberg::Vst::IHostApplication)
		QUERY_INTERFACE(_iid, obj, Steinberg::Vst::IHostApplication::iid, Steinberg::Vst::IHostApplication)
		*obj = 0;
		return Steinberg::kNoInterface;
	}

	Steinberg::uint32 PLUGIN_API addRef() {
		return 1;
	}

	Steinberg::uint32 PLUGIN_API release() {
		return 1;
	}

private:
	Steinberg::uint32 GetChannelCount() const {
		return 2; // only stereo
	}

	void AllocateBuffers() {
		for (auto &b : buffers) {
			if (b)
				FreeBuffers();
			b = new Steinberg::Vst::Sample32*[GetChannelCount()]{};
			for (unsigned i = 0; i < GetChannelCount(); ++i)
				b[i] = new Steinberg::Vst::Sample32[block_size];
		}
	}

	void FreeBuffers() {
		if (buffers[0] && buffers[1]) {
			for (auto &b : buffers) {
				for (unsigned i = 0; i < GetChannelCount(); ++i)
					if (b[i])
						delete[] b[i];
				delete b;
				b = nullptr;
			}
		}
	}

	void ConvertFrom16Bits(std::int8_t* input, float** output) {
		ConvertFrom16Bits(reinterpret_cast<std::int16_t*>(input), output);
	}

	void ConvertFrom16Bits(std::int16_t* input, float** output) {
		for (unsigned i = 0, in_i = 0; i < block_size; ++i)
			for (unsigned j = 0; j < GetChannelCount(); ++j, ++in_i)
				output[j][i] = input[in_i] / static_cast<float>(std::numeric_limits<std::int16_t>::max());
	}

	void ConvertTo16Bits(float** input, std::int8_t* output) {
		ConvertTo16Bits(input, reinterpret_cast<std::int16_t*>(output));
	}

	void ConvertTo16Bits(float** input, std::int16_t* output) {
		for (unsigned i = 0, out_i = 0; i < block_size; ++i)
			for (unsigned j = 0; j < GetChannelCount(); ++j, ++out_i)
				if (input[j][i] > 1)
					output[out_i] = std::numeric_limits<std::int16_t>::max();
				else if (input[j][i] < -1)
					output[out_i] = std::numeric_limits<std::int16_t>::min();
				else
					output[out_i] = static_cast<std::int16_t>(input[j][i] * std::numeric_limits<std::int16_t>::max());
	}

	PluginManager plugins;
	std::unique_ptr<HostWindow> gui;
	std::thread ui_thread;
	std::mutex processing_lock;

	Steinberg::Vst::TSamples block_size;
	Steinberg::Vst::SampleRate sample_rate;
	Steinberg::Vst::Sample32** buffers[2];
};

Host::Host(std::int64_t max_num_samples, double sample_rate) : impl(new Host::HostImpl(max_num_samples, sample_rate)) {

}

Host::~Host() {

}

void Host::Process(float** input, float** output, std::int64_t num_samples) {
	impl->Process(input, output, num_samples);
}

void Host::Process(char* input, char* output, std::int64_t num_samples) {
	impl->Process(input, output, num_samples);
}

void Host::Process(std::int16_t* input, std::int16_t* output, std::int64_t num_samples) {
	impl->Process(input, output, num_samples);
}

void Host::SetSampleRate(double sr) {
	impl->SetSampleRate(sr);
}

void Host::SetBlockSize(std::int64_t bs) {
	impl->SetSampleRate(bs);
}

void Host::CreateGUIThread() {
	impl->CreateGUIThread();
} // ?

void Host::CreateGUI() {
	impl->CreateGUI();
} // ?

bool Host::LoadPluginList(const std::string& path) {
	return impl->LoadPluginList(path);
}

bool Host::SavePluginList(const std::string& path) {
	return impl->SavePluginList(path);
}

bool Host::LoadPluginList() {
	return impl->LoadPluginList();
}

bool Host::SavePluginList() {
	return impl->SavePluginList();
}
} // namespace