#ifndef PRESETVST3_H
#define PRESETVST3_H
#include "public.sdk/source/common/memorystream.h"
#include "pluginterfaces/vst/ivstcomponent.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "Preset.h"
#include <string>

namespace VSTHost {
class PluginVST3;
class PresetVST3 : public Preset {
public:
	PresetVST3(PluginVST3& p, std::string n);
	~PresetVST3();
	bool SetState();
	void LoadFromFile();
	void GetState();
	void SaveToFile();
private:
	Steinberg::MemoryStream edit_stream, processor_stream;
	std::string name;
	PluginVST3& plugin;
};
} // namespace

#endif