#include "VSTPlugin.h"

#include "VSTPreset.h"

using namespace std;

VSTPlugin::VSTPlugin(HMODULE m, AEffect* plugin) : Plugin(m), VSTBase(plugin) {
	//editor = new EditorVST((char *)GetPluginName().c_str(), GetAEffect());
	//PrintInfo();
	state = new VSTPreset(GetAEffect());
	GetAEffect()->resvd1 = reinterpret_cast<VstIntPtr>(this);
}

VSTPlugin::~VSTPlugin() {
	if (module) 
		FreeLibrary(module);
}

VstIntPtr VSTCALLBACK VSTPlugin::HostCallback(AEffect *effect, VstInt32 opcode, VstInt32 index, VstInt32 value, void *ptr, float opt) {
	switch (opcode) {
		case AudioMasterOpcodes::audioMasterVersion:
			return 2400;
		case AudioMasterOpcodes::audioMasterIdle:
			return 0; // plugin gives idle time to host
		case AudioMasterOpcodesX::audioMasterGetTime:
		case AudioMasterOpcodesX::audioMasterProcessEvents:
			return 0; // unsupported
		case AudioMasterOpcodesX::audioMasterIOChanged:
			//SuspendPlugin();
			// numInputs and/or numOutputs and/or initialDelay (and/or numParameters: to be avoid) have changed
			// 
			//ResumePlugin();
			return 1;
		case AudioMasterOpcodesX::audioMasterSizeWindow:
			return 0; // unsupported
		case AudioMasterOpcodesX::audioMasterGetSampleRate:
			return static_cast<VstIntPtr>(sample_rate);
		case AudioMasterOpcodesX::audioMasterGetBlockSize:
			return static_cast<VstIntPtr>(block_size);
		case AudioMasterOpcodesX::audioMasterGetInputLatency:
		case AudioMasterOpcodesX::audioMasterGetOutputLatency:
		case AudioMasterOpcodesX::audioMasterGetCurrentProcessLevel:
		case AudioMasterOpcodesX::audioMasterGetAutomationState:
		case AudioMasterOpcodesX::audioMasterOfflineStart:
		case AudioMasterOpcodesX::audioMasterOfflineRead:
		case AudioMasterOpcodesX::audioMasterOfflineWrite:
		case AudioMasterOpcodesX::audioMasterOfflineGetCurrentPass:
		case AudioMasterOpcodesX::audioMasterOfflineGetCurrentMetaPass:
			return 0; // unsupported
		case AudioMasterOpcodesX::audioMasterGetVendorString:
			memcpy(ptr, "jperek", 7); // kVstMaxVendorStrLen
			return 1;
		case AudioMasterOpcodesX::audioMasterGetProductString:
			memcpy(ptr, "VSTHost", 8); // kVstMaxProductStrLen
			return 1;
		case AudioMasterOpcodesX::audioMasterGetVendorVersion:
			return 1000;
		case AudioMasterOpcodesX::audioMasterVendorSpecific:
			return 0;
		case AudioMasterOpcodesX::audioMasterCanDo:
			if (strcmp(static_cast<char*>(ptr), "sendVstEvents") == 0)
				return -1;
			else if (strcmp(static_cast<char*>(ptr), "sendVstMidiEvent") == 0)
				return -1;
			else if (strcmp(static_cast<char*>(ptr), "sendVstTimeInfo") == 0)
				return -1;
			else if (strcmp(static_cast<char*>(ptr), "receiveVstEvents") == 0)
				return -1;
			else if (strcmp(static_cast<char*>(ptr), "receiveVstMidiEvent") == 0)
				return -1;
			else if (strcmp(static_cast<char*>(ptr), "reportConnectionChanges") == 0)
				return -1;
			else if (strcmp(static_cast<char*>(ptr), "acceptIOChanges") == 0)
				return 1;
			else if (strcmp(static_cast<char*>(ptr), "sizeWindow") == 0)
				return -1;
			else if (strcmp(static_cast<char*>(ptr), "offline") == 0)
				return -1;
			else if (strcmp(static_cast<char*>(ptr), "openFileSelector") == 0)
				return -1;
			else if (strcmp(static_cast<char*>(ptr), "closeFileSelector") == 0)
				return -1;
			else if (strcmp(static_cast<char*>(ptr), "startStopProcess") == 0)
				return -1;
			else if (strcmp(static_cast<char*>(ptr), "shellCategory") == 0)
				return -1;
			else if (strcmp(static_cast<char*>(ptr), "sendVstMidiEventFlagIsRealtime") == 0)
				return -1;
			return 0;
		case AudioMasterOpcodesX::audioMasterGetLanguage:
			return VstHostLanguage::kVstLangEnglish;
		case AudioMasterOpcodesX::audioMasterGetDirectory: {
			char buf[128]{};
			auto length = GetModuleFileName(module, buf, 128);
			std::string tmp(buf, length);
			std::string::size_type pos = 0;
			if ((pos = tmp.find_last_of('\\')) != std::string::npos) {
				memcpy(ptr, tmp.c_str(), tmp.size());
				return 1;
			}
			else 
				return 0;
		}
		case AudioMasterOpcodesX::audioMasterUpdateDisplay:
			//if (gui)  // dont have acces to editors hwnd
			return 0;
		case AudioMasterOpcodesX::audioMasterBeginEdit:
			//index
			return 1;
		case AudioMasterOpcodesX::audioMasterEndEdit:
			//index
			return 1;
		case AudioMasterOpcodesX::audioMasterOpenFileSelector:
		case AudioMasterOpcodesX::audioMasterCloseFileSelector:
		default:
			return 0;
	}
}

VstIntPtr VSTCALLBACK VSTPlugin::HostCallbackWrapper(AEffect *effect, VstInt32 opcode, VstInt32 index, VstInt32 value, void *ptr, float opt) {
	if (opcode == AudioMasterOpcodes::audioMasterVersion)
		return 2400;
	VSTPlugin* plugin = reinterpret_cast<VSTPlugin*>(effect->resvd1);
	if (plugin)
		return plugin->HostCallback(effect, opcode, index, value, ptr, opt);
	return 0;
}

void VSTPlugin::ResumePlugin() {
	Dispatcher(effMainsChanged, 0, 1);
}

void VSTPlugin::SuspendPlugin() {
	Dispatcher(effMainsChanged, 0, 0);
}

void VSTPlugin::SetSampleRate(float sampleRate) {
	Dispatcher(effSetSampleRate, 0, 0, NULL, sampleRate);
}

void VSTPlugin::SetBlockSize(int blockSize) {
	Dispatcher(effSetBlockSize, 0, blockSize);
}

void VSTPlugin::StartPlugin() {
	Dispatcher(effOpen);
	float sampleRate = 44100.0f;
	SetSampleRate(sampleRate);
	int blockSize = 512;
	SetBlockSize(blockSize);
	ResumePlugin();
}

void VSTPlugin::Process(float **input, float **output) {
	if (CanReplacing())
		ProcessReplacing(input, output, block_size);
	else 
		VSTBase::Process(input, output, block_size);
}

bool VSTPlugin::IsValid() {
	VstPlugCategory c = static_cast<VstPlugCategory>(Dispatcher(effGetPlugCategory));
	return GetMagic() == kEffectMagic && c != kPlugCategSynth && c >= kPlugCategUnknown && c <= kPlugCategRestoration;
}

void VSTPlugin::PrintPrograms() {
	char ProgramName[kVstMaxProgNameLen + 1] = {0};
	int currentProgram = Dispatcher(effGetProgram);
	bool programChanged = false;
	for (int i = 0; i < GetNumPrograms(); i++) {
		if (Dispatcher(effGetProgramNameIndexed, i, 0, ProgramName)) std::cout << ProgramName << std::endl;
		else {
			Dispatcher(effSetProgram, 0, i);
			Dispatcher(effGetProgramName, 0, 0, ProgramName);
			if (!programChanged) programChanged = true;
		}
	}
	if (programChanged) Dispatcher(effSetProgram, 0, currentProgram);
}

void VSTPlugin::PrintParameters() {	// + 1, bo wyjatki wyrzucalo
	char ParamLabel[kVstMaxParamStrLen + 1] = {0};
	char ParamDisplay[kVstMaxParamStrLen + 1] = {0};
	char ParamName[kVstMaxParamStrLen + 1] = {0};
	VstParameterProperties properties;
	for (int i = 0; i < GetNumParams(); i++) {
		Dispatcher(effGetParamLabel, i, 0, ParamLabel);
		Dispatcher(effGetParamDisplay, i, 0, ParamDisplay);
		Dispatcher(effGetParamName, i, 0, ParamName);
		std::cout << "Parameter " << i << ":" << endl
		<< "Label: " << ParamLabel << endl
		<< "Display: " << ParamDisplay << endl
		<< "Name: " << ParamName << endl
		<< "Value: " << GetParameter(i) << endl;
		if (Dispatcher(effGetParameterProperties, i, 0, &properties)) {
			for (int j = 0; j <= 6; j++) {
				cout << "Flag ";
				switch(1 << i) {
				case kVstParameterIsSwitch:
					cout << "kVstParameterIsSwitch: " << (properties.flags & (1 << i) ? "Yes" : "No") << endl;
					break;
				case kVstParameterUsesIntegerMinMax:
					cout << "kVstParameterUsesIntegerMinMax: ";
					if (properties.flags & (1 << i)) {
						cout << "Yes" << endl
						<< "minInteger: " << properties.minInteger << endl
						<< "maxInteger: " << properties.maxInteger << endl;
					}
					else cout << "No" << endl;
					break;
				case kVstParameterUsesFloatStep:
					cout << "kVstParameterUsesFloatStep: ";
					if (properties.flags & (1 << i)) {
						cout << "Yes" << endl
						<< "stepFloat: " << properties.stepFloat << endl
						<< "smallStepFloat: " << properties.smallStepFloat << endl
						<< "largeStepFloat: " << properties.largeStepFloat << endl;
					}
					else cout << "No" << endl;
					break;
				case kVstParameterUsesIntStep:
					cout << "kVstParameterUsesIntStep: ";
					if (properties.flags & (1 << i)) {
						cout << "Yes" << endl
						<< "largeStepInteger: " << properties.largeStepInteger << endl;
					}
					else cout << "No" << endl;
					break;
				case kVstParameterSupportsDisplayIndex:
					cout << "kVstParameterSupportsDisplayIndex: ";
					if (properties.flags & (1 << i)) {
						cout << "Yes" << endl
						<< "displayIndex: " << properties.displayIndex << endl;
					}
					else cout << "No" << endl;
					break;
				case kVstParameterSupportsDisplayCategory:
					cout << "kVstParameterSupportsDisplayCategory: ";
					if (properties.flags & (1 << i)) {
						cout << "Yes" << endl
						<< "category: " << properties.category << endl
						<< "numParametersInCategory: " << properties.numParametersInCategory << endl
						<< "categoryLabel: " << properties.categoryLabel << endl;
					}
					else cout << "No" << endl;
					break;
				case kVstParameterCanRamp:
					cout << "kVstParameterCanRamps: " << (properties.flags & (1 << i) ? "Yes" : "No") << endl;
					break;
				}
			}
		}
	}
}

bool VSTPlugin::CanDo(char *canDo) {
	return (Dispatcher(effCanDo, 0, 0, (void *)canDo) != 0);
}

void VSTPlugin::PrintCanDos() {
	char *PlugCanDos[] = {"sendVstEvents", "sendVstMidiEvent", "receiveVstEvents", "receiveVstMidiEvent", 
		"receiveVstTimeInfo", "offline", "midiProgramNames", "bypass"};
	for (int i = 0; i < (sizeof(PlugCanDos) / sizeof(char *)); i++) {
		cout << PlugCanDos[i] << ": " << (CanDo(PlugCanDos[i]) ? "Yes" : "No") << endl;
	}
}

int VSTPlugin::GetVendorVersion() {
	return Dispatcher(effGetVendorVersion);
}

int VSTPlugin::GetVSTVersion() {
	return Dispatcher(effGetVstVersion);
}

void VSTPlugin::PrintInfo() {
	char *separator = "================\n";
	char EffectName[kVstMaxEffectNameLen + 1] = {0};
	char VendorString[kVstMaxVendorStrLen + 1] = {0};
	char ProductString[kVstMaxProductStrLen + 1] = {0};
	Dispatcher(effGetEffectName, 0, 0, (void *)EffectName);
	Dispatcher(effGetVendorString, 0, 0, (void *)VendorString);
	Dispatcher(effGetProductString, 0, 0, (void *)ProductString);
	VstInt32 VendorVersion = GetVendorVersion();
	VstInt32 VSTVersion = GetVSTVersion();
	cout << "VST Plugin " << EffectName << endl
		<< "Version: " << GetVersion() << endl
		<< "Vendor: " << VendorString << endl
		<< "Product: " << ProductString << endl
		<< "Vendor Version: " << VendorVersion << endl
		<< "UniqueID: " << GetUniqueID() << endl
		<< "VST Version: " << VSTVersion << endl
		<< separator << "Flags: " << endl;
	PrintFlags();
	cout << separator << "Presets(" << GetNumPrograms() << "):" << endl;
	PrintPrograms();
	cout << separator << "Parameters(" << GetNumParams() << "):" << endl;
	//PrintParameters();
	cout << separator << "Inputs: " << GetNumInputs() << endl
		<< "Outputs: " << GetNumOutputs() << endl
		<< separator << "CanDo:" << endl;
	PrintCanDos();
	cout << separator;
}

std::string VSTPlugin::GetPluginName() {
	TCHAR name[kVstMaxProductStrLen + 1] = { 0 };
	if (Dispatcher(effGetEffectName, 0, 0, (void *)name));
	else if (Dispatcher(effGetProductString, 0, 0, (void *)name));
	else {
		GetModuleFileName(module, name, kVstMaxProductStrLen + 1);
		std::string tmp(name);
		std::string::size_type pos = 0;
		if ((pos = tmp.find_last_of('\\')) != std::string::npos)
			tmp = tmp.substr(pos + 1);
		if ((pos = tmp.find_last_of('.')) != std::string::npos)
			tmp = tmp.substr(0, pos);
		return tmp;
	}
	return std::string(name);
}

std::vector<std::string> VSTPlugin::GetPresets() {
	std::vector<std::string> v;
	int currentProgram = Dispatcher(effGetProgram);
	bool programChanged = false;
	for (int i = 0; i < GetNumPrograms(); ++i) {
		char tmp[kVstMaxProgNameLen + 1] = { 0 };
		if (!Dispatcher(effGetProgramNameIndexed, i, 0, tmp)) {
			Dispatcher(effSetProgram, 0, i);
			Dispatcher(effGetProgramName, 0, 0, tmp);
			if (!programChanged) programChanged = true;
		}
		v.emplace_back(tmp);
	}
	if (programChanged) Dispatcher(effSetProgram, 0, currentProgram);
	return v;
}

void VSTPlugin::SetPreset(int i) {

}

bool VSTPlugin::HasEditor() {
	return VSTBase::HasEditor();
}

void VSTPlugin::SaveState() {
	Plugin::SaveState();
}

void VSTPlugin::LoadState() {
	Plugin::LoadState();
}