#ifndef PRESETVST3_H
#define PRESETVST3_H

#include <string>
#include <Windows.h>

#ifndef UNICODE
#define UNICODE_OFF
#endif
#include "public.sdk/source/common/memorystream.h"

#include "Preset.h"

namespace VSTHost {
class PluginVST3;
class PresetVST3 : public Preset {
public:
	PresetVST3(PluginVST3& p);
	~PresetVST3();
	void SetState();
	void LoadFromFile();
	void GetState();
	void SaveToFile();
private:
	Steinberg::MemoryStream edit_stream, processor_stream;
	static const std::string kExtension;
	Steinberg::FUID fuid;
	PluginVST3& plugin;
};
} // namespace

#endif