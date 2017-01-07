#ifndef PRESETVST2_H
#define PRESETVST2_H

#include "pluginterfaces\vst2.x\aeffectx.h"
#include "pluginterfaces\vst2.x\vstfxstore.h"

#include "Preset.h"

#ifndef __cpp_constexpr // msvc 13 doesn't support constexpr it seems
#define constexpr
#endif

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
	static constexpr bool SwapNeeded();
	static const size_t kProgramUnionSize;	// sizeof(fxProgram::content)
	static const std::string kExtension;
	fxProgram* program;
	size_t fxprogram_size; // size of fxprogram in this particular instance, without 2 first values
	bool program_chunks;
	PluginVST2& plugin;
};
} // namespace

#endif