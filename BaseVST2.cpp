#include "BaseVST2.h"

namespace VSTHost {
BaseVST2::~BaseVST2() {
	//if (plugin) 
		//delete plugin;
}

AEffect *BaseVST2::GetAEffect() {
	return plugin;
}

int BaseVST2::Dispatcher(int opcode, int index, int value, void* ptr, float opt) {
	return plugin->dispatcher(plugin, opcode, index, value, ptr, opt);
}

void BaseVST2::Process(float** inputs, float** outputs, int sampleFrames) {
	plugin->process(plugin, inputs, outputs, sampleFrames);
}

void BaseVST2::ProcessReplacing(float** inputs, float** outputs, int sampleFrames) {
	plugin->processReplacing(plugin, inputs, outputs, sampleFrames);
}

void BaseVST2::DoubleProcessReplacing(double** inputs, double** outputs, VstInt32 sampleFrames){
	plugin->processDoubleReplacing(plugin, inputs, outputs, sampleFrames);
}

void BaseVST2::SetParameter(int index, float parameter){
	plugin->setParameter(plugin, index, parameter);
}

float BaseVST2::GetParameter(int index) {
	return plugin->getParameter(plugin, index);
}

int BaseVST2::GetMagic() {
	return plugin->magic;
}

int BaseVST2::GetUniqueID() {
	return plugin->uniqueID;
}

int BaseVST2::GetVersion() {
	return plugin->version;
}

int BaseVST2::GetNumPrograms() {
	return plugin->numPrograms;
}

int BaseVST2::GetNumParams() {
	return plugin->numParams;
}

int BaseVST2::GetNumInputs() {
	return plugin->numInputs;
}

int BaseVST2::GetNumOutputs() {
	return plugin->numOutputs;
}

int BaseVST2::GetFlags() {
	return plugin->flags;
}

bool BaseVST2::HasEditor() {
	return static_cast<bool>(GetFlags() & effFlagsHasEditor);
}

bool BaseVST2::CanReplacing() {
	return 0 != (GetFlags() & effFlagsCanReplacing);
}

bool BaseVST2::CanDoubleReplacing(){
	return 0 != (GetFlags() & effFlagsCanDoubleReplacing);
}

bool BaseVST2::ProgramChunks(){
	return 0 != (GetFlags() & effFlagsProgramChunks);
}

bool BaseVST2::IsSynth(){
	return 0 != (GetFlags() & effFlagsIsSynth);
}

bool BaseVST2::NoSoundInStop(){
	return 0 != (GetFlags() & effFlagsNoSoundInStop);
}

void BaseVST2::PrintFlags() {
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
} // namespace