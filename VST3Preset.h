#ifndef VST3PRESET_H
#define VST3PRESET_H
#include "public.sdk/source/common/memorystream.h"
#include "pluginterfaces/vst/ivstcomponent.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "Preset.h"
#include <string>

namespace VSTHost {
class VST3Preset : public Preset {
private:
	Steinberg::Vst::IComponent* processor;
	Steinberg::Vst::IEditController* edit;
	Steinberg::MemoryStream edit_stream, processor_stream;
	std::string name;
public:
	VST3Preset(Steinberg::Vst::IComponent* pc, Steinberg::Vst::IEditController* ec, std::string n);
	~VST3Preset();
	bool SetState();
	void LoadFromFile();
	void GetState();
	void SaveToFile();
};
} // namespace

#endif