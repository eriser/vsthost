#include "PluginManager.h"

#include <iostream>
#include <fstream>

#include "Plugin.h"
#include "PluginLoader.h"

namespace VSTHost {
const std::string PluginManager::kPluginList{ "vsthost.ini" };

PluginManager::PluginManager(Steinberg::Vst::TSamples bs, Steinberg::Vst::SampleRate sr)
	: def_block_size(bs), def_sample_rate(sr) {

}

PluginManager::IndexType PluginManager::Size() const {
	return plugins.size();
}

Plugin& PluginManager::GetAt(PluginManager::IndexType i) const {
	return *plugins[i];
}

Plugin& PluginManager::operator[](IndexType i) const {
	return GetAt(i);
}

Plugin& PluginManager::Front() const {
	return *plugins[0]; // checking whether there is at least 1 plugin is on the user
}

Plugin& PluginManager::Back() const {
	return *plugins[Size() - 1]; // same
}


bool PluginManager::Add(const std::string& path) {
	auto plugin = PluginLoader::Load(path, def_block_size, def_sample_rate);
	if (plugin) { // host now owns what plugin points at
		std::cout << "Loaded " << path << "." << std::endl;
		plugin->Initialize();
		plugins.push_back(std::move(plugin));
		return true;
	}
	std::cout << "Could not load " << path << "." << std::endl;
	return false;
}

void PluginManager::Delete(IndexType i) {
	if (i < Size())
		plugins.erase(plugins.begin() + i);
}

void PluginManager::Swap(IndexType i, IndexType j) {
	if (i < Size() && j < Size())
		std::swap(plugins[i], plugins[j]);
}

const std::string& PluginManager::GetDefaultPluginListPath() {
	return kPluginList;
}

bool PluginManager::LoadPluginList(const std::string& path) {
	std::string line;
	std::ifstream list(path);
	if (list.is_open()) {
		while (getline(list, line))
			if (!line.empty())
				Add(line);
		for (decltype(Size()) i = 0; i < Size(); ++i)
			GetAt(i).LoadStateFromFile();
		list.close();
		return true;
	}
	else
		std::cout << "Could not open " << path << '.' << std::endl;
	return false;
}

bool PluginManager::SavePluginList(const std::string& path) const {
	std::ofstream list(path, std::ofstream::out | std::ofstream::trunc);
	if (list.is_open()) {
		for (decltype(Size()) i = 0; i < Size(); ++i) {
			list << Plugin::kPluginDirectory + GetAt(i).GetPluginFileName() << std::endl;
		}
		list.close();
		return true;
	}
	return false;
}

void PluginManager::SetBlockSize(Steinberg::Vst::TSamples bs) {
	def_block_size = bs;
}

void PluginManager::SetSampleRate(Steinberg::Vst::SampleRate sr) {
	def_sample_rate = sr;
}
} // namespace