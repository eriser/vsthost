#ifndef PLUGINLOADER_H
#define PLUGINLOADER_H

#include <string>
#include <memory>

#ifndef UNICODE
#define UNICODE_OFF
#endif
#include "pluginterfaces/vst/vsttypes.h"
#include "pluginterfaces/base/funknown.h"

namespace VSTHost {
class Plugin;
class PluginLoader {
public:
	static std::unique_ptr<Plugin> Load(const std::string& path, Steinberg::FUnknown* context);
};
} // namespace

#endif