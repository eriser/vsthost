#include "VSTBase.h"

VSTBase::~VSTBase() {
	//if (plugin) 
		//delete plugin;
}

AEffect *VSTBase::GetAEffect() {
	return plugin;
}

int VSTBase::Dispatcher(int opcode, int index, int value, void* ptr, float opt) {
	return plugin->dispatcher(plugin, opcode, index, value, ptr, opt);
}

void VSTBase::Process(float** inputs, float** outputs, int sampleFrames) {
	plugin->process(plugin, inputs, outputs, sampleFrames);
}

void VSTBase::ProcessReplacing(float** inputs, float** outputs, int sampleFrames) {
	plugin->processReplacing(plugin, inputs, outputs, sampleFrames);
}

void VSTBase::DoubleProcessReplacing(double** inputs, double** outputs, VstInt32 sampleFrames){
	plugin->processDoubleReplacing(plugin, inputs, outputs, sampleFrames);
}

void VSTBase::SetParameter(int index, float parameter){
	plugin->setParameter(plugin, index, parameter);
}

float VSTBase::GetParameter(int index) {
	return plugin->getParameter(plugin, index);
}

int VSTBase::GetMagic() {
	return plugin->magic;
}

int VSTBase::GetUniqueID() {
	return plugin->uniqueID;
}

int VSTBase::GetVersion() {
	return plugin->version;
}

int VSTBase::GetNumPrograms() {
	return plugin->numPrograms;
}

int VSTBase::GetNumParams() {
	return plugin->numParams;
}

int VSTBase::GetNumInputs() {
	return plugin->numInputs;
}

int VSTBase::GetNumOutputs() {
	return plugin->numOutputs;
}

int VSTBase::GetFlags() {
	return plugin->flags;
}

bool VSTBase::HasEditor() {
	return static_cast<bool>(GetFlags() & effFlagsHasEditor);
}

bool VSTBase::CanReplacing() {
	return (GetFlags() & effFlagsCanReplacing);
}

bool VSTBase::CanDoubleReplacing(){
	return (GetFlags() & effFlagsCanDoubleReplacing);
}

bool VSTBase::ProgramChunks(){
	return (GetFlags() & effFlagsProgramChunks);
}

bool VSTBase::IsSynth(){
	return (GetFlags() & effFlagsIsSynth);
}

bool VSTBase::NoSoundInStop(){
	return (GetFlags() & effFlagsNoSoundInStop);
}

void VSTBase::PrintFlags() {
	for (int i = 0; i < 13; i++) {
		if (i == 6 || i == 7) continue;
		switch(1 << i) {
			case effFlagsHasEditor:
				std::cout << "effFlagsHasEditor: ";
				break;
			case effFlagsCanReplacing:
				std::cout << "effFlagsCanReplacing: ";
				break;
			case effFlagsProgramChunks:
				std::cout << "effFlagsProgramChunks: ";
				break;
			case effFlagsIsSynth:
				std::cout << "effFlagsIsSynth: ";
				break;
			case effFlagsNoSoundInStop:
				std::cout << "effFlagsNoSoundInStop: ";
				break;
			case effFlagsCanDoubleReplacing:
				std::cout << "effFlagsCanDoubleReplacing (VST 2.4): ";
				break;
			case effFlagsHasClip:
				std::cout << "effFlagsHasClip (Deprecated): ";
				break;
			case effFlagsHasVu:
				std::cout << "effFlagsHasVu (Deprecated): ";
				break;
			case effFlagsCanMono:
				std::cout << "effFlagsCanMono (Deprecated): ";
				break;
			case effFlagsExtIsAsync:
				std::cout << "effFlagsExtIsAsync (Deprecated): ";
				break;
			case effFlagsExtHasBuffer:
				std::cout << "effFlagsExtHasBuffer (Deprecated): ";
				break;
		}
		std::cout << (GetFlags() & (1 << i) ? "Yes" : "No") << std::endl;
	}
}