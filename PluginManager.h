#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <vector>
#include <memory>
#include <string>

#include "pluginterfaces/vst/vsttypes.h"

namespace VSTHost {
class Plugin;
class PluginLoader;
class PluginManager {
	using IndexType = std::vector<std::unique_ptr<Plugin>>::size_type;
	class PluginIterator;
public:
	PluginManager(Steinberg::Vst::TSamples bs, Steinberg::Vst::SampleRate sr);
	IndexType Size() const;
	Plugin& GetAt(IndexType i) const;
	Plugin& operator[](IndexType i) const;
	Plugin& Front() const;
	Plugin& Back() const;
	bool Add(const std::string& path);
	void Delete(IndexType i);
	void Swap(IndexType i, IndexType j);

	const std::string& GetDefaultPluginListPath();
	bool LoadPluginList(const std::string& path = kPluginList);
	bool SavePluginList(const std::string& path = kPluginList) const;

	void SetBlockSize(Steinberg::Vst::TSamples bs);
	void SetSampleRate(Steinberg::Vst::SampleRate sr);

	const static std::string kPluginList;
private:
	Steinberg::Vst::TSamples def_block_size;		// default block size & sample rate
	Steinberg::Vst::SampleRate def_sample_rate;		// for new plugins
	std::vector<std::unique_ptr<Plugin>> plugins;
};
} // namespace

#endif