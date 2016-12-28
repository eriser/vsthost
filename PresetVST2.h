#ifndef PRESETVST2_H
#define PRESETVST2_H

#include <iostream>
#include <string.h>

#include "pluginterfaces\vst2.x\aeffectx.h"
#include "pluginterfaces\vst2.x\vstfxstore.h"

#include "BaseVST2.h"
#include "Preset.h"

namespace VSTHost {
	class PresetVST2 : public Preset, public BaseVST2 {
private:
	bool isSaved;
	char path[kVstMaxEffectNameLen + 2 + 4 + 1];	// 2 na "./", 4 na ".fxp", 1 na wszelki wypadek

	VstPatchChunkInfo info;

	fxProgram *program;
	char *chunk;
	int size, chunkSize;
public:
	PresetVST2(AEffect *plugin);
	~PresetVST2();
	bool SetState();
	void LoadFromFile();
	void GetState();
	void SaveToFile();
	void AddExtension();
};
} // namespace

#endif