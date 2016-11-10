#include "Host.h"

#include <limits>
#include <cstring>

const std::string Host::kPluginsPath{ "plugins.txt" };

Host::Host(std::int64_t block_size, double sample_rate, bool stereo)
	: block_size(block_size), sample_rate(sample_rate) {
	speaker_arrangement = stereo ? Steinberg::Vst::SpeakerArr::kStereo : Steinberg::Vst::SpeakerArr::kMono;
	buffers[0] = nullptr;
	buffers[1] = nullptr;
	AllocateBuffers();
	LoadPluginList();
	for (auto p : plugins)
		p->CreateEditor();
}

Host::~Host() {
	for (auto p : plugins)
		delete p;
	FreeBuffers();
}

Steinberg::tresult Host::LoadPlugin(std::string path) {
	std::string name(path);
	std::string::size_type pos = 0;
	if ((pos = name.find_last_of('\\')) != std::string::npos)
		name = name.substr(pos + 1);
	else if ((pos = name.find_last_of('/')) != std::string::npos)
		name = name.substr(pos + 1);
	Plugin* plugin = nullptr;
	PluginLoader loader(path);
	if (loader.IsVST() || loader.IsVST3()) {
		if (loader.IsVST()) {
			AEffect* effect = nullptr;
			auto init = static_cast<PluginLoader::VSTInitProc>(loader.GetInitProc());
			effect = init(VSTPlugin::hostCallback);
			plugin = new VSTPlugin(loader.GetModule(), effect, block_size, sample_rate, speaker_arrangement);
		}
		else if (loader.IsVST3()) {
			Steinberg::IPluginFactory* factory = nullptr;
			GetFactoryProc getFactory = static_cast<GetFactoryProc>(loader.GetInitProc());
			if (getFactory) {
				factory = getFactory();
				plugin = new VST3Plugin(loader.GetModule(), factory, block_size, sample_rate, speaker_arrangement);
			}
		}
		if (plugin->IsValid()) {
			std::cout << "Loaded " << name << "." << std::endl;
			plugins.push_back(plugin);
			return Steinberg::kResultTrue;
		}
		else {
			std::cout << name << " is not a supported VST plugin." << std::endl;
			if (plugin)
				delete plugin;
			return Steinberg::kResultFalse;
		}
	}
	else {
		std::cout << name << " is not a VST plugin." << std::endl;
		if (loader.GetModule())
			FreeLibrary(loader.GetModule());
		return Steinberg::kResultFalse;
	}
}

void Host::Process(float** input, float** output) {
	if (plugins.size() == 1)
		plugins.front()->Process(input, output);
	else if (plugins.size() > 1) {
		plugins.front()->Process(input, buffers[1]);
		unsigned int i;
		for (i = 1; i < plugins.size() - 1; i++) {
			plugins[i]->Process(buffers[i % 2], buffers[(i + 1) % 2]);
		}
		plugins.back()->Process(buffers[i % 2], output);
	}
	else
		for (unsigned i = 0; i < GetChannelCount(); ++i)
			std::memcpy(output[i], input[i], sizeof(input[0][0]) * block_size);
}

void Host::Process(char* input, char* output) { // char != int8_t
	if (std::numeric_limits<char>::min() < 0)
		Process(reinterpret_cast<std::int8_t*>(input), reinterpret_cast<std::int8_t*>(output));
}

void Host::Process(std::int8_t* input, std::int8_t* output) {
	Process(reinterpret_cast<std::int16_t*>(input), reinterpret_cast<std::int16_t*>(output));
}

void Host::test_conv(std::int16_t* input, std::int16_t* output) {
	ConvertFrom16Bits(input, buffers[0]);
	std::memcpy(buffers[1][0], buffers[0][0], sizeof(buffers[0][0][0]) * block_size);
	std::memcpy(buffers[1][1], buffers[0][1], sizeof(buffers[0][1][0]) * block_size);
	ConvertTo16Bits(buffers[1], output);
}

void Host::Process(std::int16_t* input, std::int16_t* output) {
	ConvertFrom16Bits(input, buffers[0]);
	if (plugins.size() == 1) {
		plugins.front()->Process(buffers[0], buffers[1]);
		ConvertTo16Bits(buffers[1], output);
	}
	else if (plugins.size() > 1) {
		plugins.front()->Process(buffers[0], buffers[1]);
		unsigned int i;
		for (i = 1; i < plugins.size() - 1; i++) {
			plugins[i]->Process(buffers[i % 2], buffers[(i + 1) % 2]);
		}
		plugins.back()->Process(buffers[i % 2], buffers[(i + 1) % 2]);
		ConvertTo16Bits(buffers[(i + 1) % 2], output);
	}
	else
		std::memcpy(output, input, sizeof(input[0]) * block_size * GetChannelCount());
}

void Host::SetSampleRate(double sr) {
	sample_rate = sr;
}

void Host::SetBlockSize(std::int64_t bs) {
	if (bs != block_size) {
		block_size = bs;
		FreeBuffers();
		AllocateBuffers();
	}
}

void Host::SetSpeakerArrangement(std::uint64_t sa) {
	if (sa != speaker_arrangement) {
		FreeBuffers();
		speaker_arrangement = sa;
		AllocateBuffers();
	}
}

void Host::SetActive(bool ia) {
	is_active = ia;
	//for (p : plugins);

}

Steinberg::tresult PLUGIN_API Host::getName(Steinberg::Vst::String128 name) {
	Steinberg::String str("Host");
	str.copyTo16(name, 0, 127);
	return Steinberg::kResultTrue;
}

Steinberg::tresult PLUGIN_API Host::createInstance(Steinberg::TUID cid, Steinberg::TUID iid, void** obj) {
	return 0;
}

void Host::LoadPluginList() {
	std::string line;
	std::ifstream paths(kPluginsPath); // jak nie ma pliku to stworzyc pusty.
	if (paths.is_open())
		while (getline(paths, line))
			if (!line.empty())
				LoadPlugin(line);
			else 
				std::cout << "Could not open " << kPluginsPath << '.' << std::endl;
}

Steinberg::uint32 Host::GetChannelCount() {
	return static_cast<Steinberg::uint32>(Steinberg::Vst::SpeakerArr::getChannelCount(speaker_arrangement));
}

void Host::AllocateBuffers() {
	for (auto &b : buffers) {
		if (b)
			FreeBuffers();
		b = new Steinberg::Vst::Sample32*[GetChannelCount()]{};
		for (unsigned i = 0; i < GetChannelCount(); ++i)
			b[i] = new Steinberg::Vst::Sample32[block_size];
	}
}

void Host::FreeBuffers() {
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

void Host::ConvertFrom16Bits(std::int8_t* input, float** output) {
	ConvertFrom16Bits(reinterpret_cast<std::int16_t*>(input), output);
	/*
	std::int16_t* input16 = reinterpret_cast<std::int16_t*>(input);
	for (unsigned i = 0, in_i = 0; i < block_size; ++i) {
		for (unsigned j = 0; j < GetChannelCount(); ++j, ++in_i) {
			//SWAP_16(input16[in_i]);
			output[j][i] = input16[in_i] / static_cast<float>(std::numeric_limits<std::int16_t>::max());
			//SWAP_16(input16[in_i]);
		}
	}
	*/
}

void Host::ConvertFrom16Bits(std::int16_t* input, float** output) {
	for (unsigned i = 0, in_i = 0; i < block_size; ++i)
		for (unsigned j = 0; j < GetChannelCount(); ++j, ++in_i)
			output[j][i] = input[in_i] / static_cast<float>(std::numeric_limits<std::int16_t>::max());
}

void Host::ConvertTo16Bits(float** input, std::int8_t* output) {
	ConvertTo16Bits(input, reinterpret_cast<std::int16_t*>(output));
	/*
	std::int16_t* output16 = reinterpret_cast<std::int16_t*>(output);
	for (unsigned i = 0, out_i = 0; i < block_size; ++i) {
		for (unsigned j = 0; j < GetChannelCount(); ++j, ++out_i) {
			if (input[j][i] > 1)
				output16[out_i] = std::numeric_limits<std::int16_t>::max();
			else if (input[j][i] < -1)
				output16[out_i] = std::numeric_limits<std::int16_t>::min();
			else
				output16[out_i] = static_cast<std::int16_t>(input[j][i] * std::numeric_limits<std::int16_t>::max());
			//SWAP_16(output16[out_i]);
		}
	}
	*/
}

void Host::ConvertTo16Bits(float** input, std::int16_t* output) {
	for (unsigned i = 0, out_i = 0; i < block_size; ++i)
		for (unsigned j = 0; j < GetChannelCount(); ++j, ++out_i)
			if (input[j][i] > 1)
				output[out_i] = std::numeric_limits<std::int16_t>::max();
			else if (input[j][i] < -1)
				output[out_i] = std::numeric_limits<std::int16_t>::min();
			else
				output[out_i] = static_cast<std::int16_t>(input[j][i] * std::numeric_limits<std::int16_t>::max());
}