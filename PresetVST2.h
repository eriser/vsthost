#ifndef PRESETVST2_H
#define PRESETVST2_H

#include <iostream>
#include <string.h>

#include "pluginterfaces\vst2.x\aeffectx.h"
#include "pluginterfaces\vst2.x\vstfxstore.h"

#include "Preset.h"

namespace VSTHost {
class PluginVST2;
class PresetVST2 : public Preset {
private:
	bool isSaved;
	char path[kVstMaxEffectNameLen + 2 + 4 + 1];	// 2 na "./", 4 na ".fxp", 1 na wszelki wypadek

	VstPatchChunkInfo info;

	fxProgram *program;
	char *chunk;
	int size, chunkSize;
public:
	PresetVST2(PluginVST2& p);
	~PresetVST2();
	bool SetState();
	void LoadFromFile();
	void GetState();
	void SaveToFile();
	void AddExtension();
private:
	bool ProgramChunks();
	PluginVST2& plugin;
};
} // namespace

#endif