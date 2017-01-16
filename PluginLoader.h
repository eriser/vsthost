#ifndef PLUGINLOADER_H
#define PLUGINLOADER_H

#include <string>
#include <memory>

#ifndef UNICODE
#define UNICODE_OFF
#endif
#include "pluginterfaces/vst/vsttypes.h"

namespace VSTHost {
class Plugin;
class PluginLoader {
public:
	static std::unique_ptr<Plugin> Load(const std::string& path, Steinberg::Vst::TSamples bs, Steinberg::Vst::SampleRate sr);
};
} // namespace

#endif