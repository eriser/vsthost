#include <iostream>
#include <string.h>

#include "pluginterfaces\vst2.x\aeffectx.h"
#include "pluginterfaces\vst2.x\vstfxstore.h"

#include "VSTBase.h"
#include "Preset.h"

#ifndef VSTPRESET_H
#define VSTPRESET_H

class VSTPreset : public Preset, public VSTBase {
private:
	bool isSaved;
	char path[kVstMaxEffectNameLen + 2 + 4 + 1];	// 2 na "./", 4 na ".fxp", 1 na wszelki wypadek

	VstPatchChunkInfo info;

	fxProgram *program;
	char *chunk;
	int size, chunkSize;
public:
	VSTPreset(AEffect *plugin);
	~VSTPreset();
	bool Load();
	void LoadFromFile();
	void Save();
	void SaveToFile();
	void AddExtension();
};

#endif