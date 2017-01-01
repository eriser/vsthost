#ifndef PLUGINLOADER_H
#define PLUGINLOADER_H

#include <string>
#include <memory>

namespace VSTHost {
class Plugin;
class PluginLoader {
public:
	PluginLoader(std::string path);
	~PluginLoader();
	Plugin* GetPlugin();
private:
	Plugin* plugin = { nullptr };
};
} // namespace

#endif