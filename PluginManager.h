#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <vector>
#include <memory>
#include <string>
#include <mutex>

#include "pluginterfaces/vst/vsttypes.h"

namespace VSTHost {
class Plugin;
class PluginLoader;
class PluginManager {
	using IndexType = std::vector<std::unique_ptr<Plugin>>::size_type;
public:
	class PluginIterator {
	public:
		PluginIterator(std::vector<std::unique_ptr<Plugin>>::iterator i);
		bool operator!=(PluginIterator& i);
		PluginIterator operator++();
		PluginIterator operator++(int);
		Plugin& operator*();
	private:
		std::vector<std::unique_ptr<Plugin>>::iterator it;
	};
	PluginManager(Steinberg::Vst::TSamples bs, Steinberg::Vst::SampleRate sr);
	IndexType Size() const;
	Plugin& GetAt(IndexType i) const;
	Plugin& operator[](IndexType i) const;
	Plugin& Front() const;
	Plugin& Back() const;
	PluginIterator Begin();
	PluginIterator End();
	std::mutex& GetLock();
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
	std::mutex manager_lock; // used to not delete an element while iterating over all contents
};
} // namespace

#endif