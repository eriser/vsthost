#ifndef PRESETVST2_H
#define PRESETVST2_H

#include "pluginterfaces\vst2.x\aeffectx.h"
#include "pluginterfaces\vst2.x\vstfxstore.h"

#include "Preset.h"

namespace VSTHost {
class PluginVST2;
class PresetVST2 : public Preset {
public:
	PresetVST2(PluginVST2& p);
	~PresetVST2();
	void SetState();
	void LoadFromFile();
	void GetState();
	void SaveToFile();
private:
	void SwapProgram();
	bool ProgramChunks() const;
	static bool SwapNeeded();
	static const size_t kProgramUnionSize;	// sizeof(fxProgram::content)
	static const std::string kExtension;
	fxProgram* program;
	size_t fxprogram_size; // size of fxprogram in this particular instance, without 2 first values
	bool program_chunks;
	PluginVST2& plugin;
};
} // namespace

#endif