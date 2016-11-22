#include <iostream>

#ifndef VST_FORCE_DEPRECATED
#define VST_FORCE_DEPRECATED 0
#endif
#include "pluginterfaces/vst2.x/aeffect.h"
#include "pluginterfaces/vst2.x/aeffectx.h"

#ifndef VSTBASE_H
#define VSTBASE_H

class VSTBase {
public:
	VSTBase(AEffect *aeffect) : plugin(aeffect) {}
	~VSTBase();
	AEffect *GetAEffect();
protected:
	int Dispatcher(int opcode, int index = 0, int value = 0, void* ptr = NULL, float opt = 0.);
	void Process(float** inputs, float** outputs, int sampleFrames);
	void ProcessReplacing(float** inputs, float** outputs, int sampleFrames);
	void DoubleProcessReplacing(double** inputs, double** outputs, int sampleFrames);
	void SetParameter(int index, float parameter);
	float GetParameter(int index);
	int GetMagic();
	int GetUniqueID();
	int GetVersion();
	int GetNumPrograms();
	int GetNumParams();
	int GetNumInputs();
	int GetNumOutputs();
	int GetFlags();	// zmienic nazwy metod nizej na Flag...()?
	bool HasEditor();
	bool CanReplacing();
	bool CanDoubleReplacing();
	bool ProgramChunks();
	bool IsSynth();
	bool NoSoundInStop();
	void PrintFlags();
private:
	AEffect *plugin;
};

#endif