#ifndef PRESETVST3_H
#define PRESETVST3_H
#include "public.sdk/source/common/memorystream.h"
#include "pluginterfaces/vst/ivstcomponent.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "Preset.h"
#include <string>

namespace VSTHost {
class PresetVST3 : public Preset {
private:
	Steinberg::Vst::IComponent* processor;
	Steinberg::Vst::IEditController* edit;
	Steinberg::MemoryStream edit_stream, processor_stream;
	std::string name;
public:
	PresetVST3(Steinberg::Vst::IComponent* pc, Steinberg::Vst::IEditController* ec, std::string n);
	~PresetVST3();
	bool SetState();
	void LoadFromFile();
	void GetState();
	void SaveToFile();
};
} // namespace

#endif