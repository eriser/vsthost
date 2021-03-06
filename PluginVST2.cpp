#include "PluginVST2.h"

#include <iostream>

#include "pluginterfaces\vst2.x\aeffectx.h"

#include "PresetVST2.h"
#include "PluginVST2Window.h"

namespace VSTHost {
PluginVST2::PluginVST2(HMODULE m, AEffect* p)
	: Plugin(m), plugin(p) {
	plugin->resvd1 = reinterpret_cast<VstIntPtr>(this);

	// try setting correct number of inputs and outputs if there's a need
	if (plugin->numInputs != 2 || plugin->numOutputs != 2) {
		VstSpeakerArrangement in{}, out{};
		in.numChannels = 2;
		in.type = VstSpeakerArrangementType::kSpeakerArrStereo;
		in.speakers[0].type = VstSpeakerType::kSpeakerL;
		in.speakers[0].name[0] = 'L';
		in.speakers[0].name[1] = '\0';
		in.speakers[1].type = VstSpeakerType::kSpeakerR;
		in.speakers[1].name[0] = 'R';
		in.speakers[1].name[1] = '\0';
		out.numChannels = 2;
		out.type = VstSpeakerArrangementType::kSpeakerArrStereo;
		out.speakers[0].type = VstSpeakerType::kSpeakerL;
		out.speakers[0].name[0] = 'L';
		out.speakers[0].name[1] = '\0';
		out.speakers[1].type = VstSpeakerType::kSpeakerR;
		out.speakers[1].name[0] = 'R';
		out.speakers[1].name[1] = '\0';
		Dispatcher(AEffectXOpcodes::effSetSpeakerArrangement, 0, reinterpret_cast<VstIntPtr>(&in), &out);
	}
}

PluginVST2::~PluginVST2() {
	SetActive(false);
	gui.reset();	// gui and state have to be destroyed before the rest of the plugin is freed
	state.reset();
	Dispatcher(AEffectOpcodes::effClose);
	// turns out offClose opcode handles freeing AEffect object and I musn't do that
	plugin.release();
}

Plugin::IsValidCodes PluginVST2::IsValid() const {
	if (plugin) {
		VstPlugCategory c = static_cast<VstPlugCategory>(plugin->dispatcher(plugin.get(), AEffectXOpcodes::effGetPlugCategory, 0, 0, nullptr, 0.));
		if (c != kPlugCategSynth && c >= kPlugCategUnknown && c <= kPlugCategRestoration) {
			if (plugin->numInputs == 2 && plugin->numOutputs == 2) {
				if (plugin->magic == kEffectMagic)
					return IsValidCodes::kValid;
				else
					return IsValidCodes::kInvalid;
			}
			else
				return IsValidCodes::kWrongInOutNum;
		}
		else
			return IsValidCodes::kIsNotEffect;
	}
	else
		return IsValidCodes::kInvalid;
}

void PluginVST2::Initialize(Steinberg::Vst::TSamples bs, Steinberg::Vst::SampleRate sr) {
	sample_rate = sr;
	block_size = bs;
	Dispatcher(AEffectOpcodes::effOpen);
	Dispatcher(AEffectOpcodes::effSetSampleRate, 0, 0, nullptr, static_cast<float>(sample_rate));
	Dispatcher(AEffectOpcodes::effSetBlockSize, 0, static_cast<VstIntPtr>(block_size));
	state = std::unique_ptr<Preset>(new PresetVST2(*this));
	soft_bypass = CanDo("bypass");
	SetActive(true);
}

std::basic_string<TCHAR> PluginVST2::GetPluginName() const {
	char name[kVstMaxProductStrLen] = { 0 }; // vst2 does not support unicode
	if (plugin->dispatcher(plugin.get(), AEffectXOpcodes::effGetEffectName, 0, 0, reinterpret_cast<void*>(name), 0.) ||
		plugin->dispatcher(plugin.get(), AEffectXOpcodes::effGetProductString, 0, 0, reinterpret_cast<void*>(name), 0.)) {
		std::string tmp(name);
		return std::basic_string<TCHAR>(tmp.begin(), tmp.end());
	}
	else {
		TCHAR buf[MAX_PATH] = { 0 };
		GetModuleFileName(module, buf, MAX_PATH);
		std::basic_string<TCHAR> tmp(buf);
		std::basic_string<TCHAR>::size_type pos = 0;
		if ((pos = tmp.find_last_of('\\')) != std::basic_string<TCHAR>::npos)
			tmp = tmp.substr(pos + 1);
		if ((pos = tmp.find_last_of('.')) != std::basic_string<TCHAR>::npos)
			tmp = tmp.substr(0, pos);
		return tmp;
	}
}

void PluginVST2::Process(Steinberg::Vst::Sample32** input, Steinberg::Vst::Sample32** output, Steinberg::Vst::TSamples block_size) {
	if (IsActive() && !BypassProcess()) {
		std::lock_guard<std::mutex> lock(plugin_lock);
		StartProcessing();
		if (0 != (plugin->flags & VstAEffectFlags::effFlagsCanReplacing))
			plugin->processReplacing(plugin.get(), input, output, block_size);
		else
			plugin->process(plugin.get(), input, output, block_size);
		StopProcessing();
	}
	else {
		for (unsigned i = 0; i < GetChannelCount(); ++i)
			std::memcpy(static_cast<void*>(output[i]), static_cast<void*>(input[i]), sizeof(input[0][0]) * block_size);
	}
}

void PluginVST2::SetBlockSize(Steinberg::Vst::TSamples bs) {
	bool was_active;
	if (was_active = IsActive())
		SetActive(false);
	block_size = bs;
	Dispatcher(AEffectOpcodes::effSetBlockSize, 0, static_cast<VstIntPtr>(block_size));
	if (was_active)
		SetActive(true);
}

void PluginVST2::SetSampleRate(Steinberg::Vst::SampleRate sr) {
	bool was_active;
	if (was_active = IsActive())
		SetActive(false);
	sample_rate = sr;
	Dispatcher(AEffectOpcodes::effSetSampleRate, 0, 0, nullptr, static_cast<float>(sample_rate));
	if (was_active)
		SetActive(true);
}

Steinberg::int32 PluginVST2::GetProgramCount() const {
	return plugin->numPrograms;
}

void PluginVST2::SetProgram(Steinberg::int32 id) {
	Dispatcher(AEffectXOpcodes::effBeginSetProgram);
	Dispatcher(AEffectOpcodes::effSetProgram, 0, id);
	Dispatcher(AEffectXOpcodes::effEndSetProgram);
}

Steinberg::int32 PluginVST2::GetParameterCount() const {
	return plugin->numParams;
}

Steinberg::Vst::ParamValue PluginVST2::GetParameter(Steinberg::Vst::ParamID id) const {
	return plugin->getParameter(plugin.get(), id);
}

void PluginVST2::SetParameter(Steinberg::Vst::ParamID id, Steinberg::Vst::ParamValue value) {
	plugin->setParameter(plugin.get(), id, static_cast<float>(value));
}

void PluginVST2::SetBypass(bool bypass_) {
	if (bypass != bypass_) {
		bypass = bypass_;
		if (soft_bypass)
			Dispatcher(AEffectXOpcodes::effSetBypass, 0, bypass);
	}
}

bool PluginVST2::BypassProcess() const {	// wywolanie process omijaj tylko wtedy, jak
	return bypass && !soft_bypass;	// bypass == true i wtyczka nie obsluguje soft bypass
}

bool PluginVST2::HasEditor() const {
	return static_cast<bool>(plugin->flags & effFlagsHasEditor);
}

void PluginVST2::CreateEditor(HWND hWnd) {
	if (!gui && HasEditor()) {
		gui = std::unique_ptr<PluginWindow>(new PluginVST2Window(*this));
		gui->Initialize(hWnd);
	}
}

VstIntPtr VSTCALLBACK PluginVST2::HostCallbackWrapper(AEffect *effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void *ptr, float opt) {
	if (opcode == AudioMasterOpcodes::audioMasterVersion)
		return 2400;
	PluginVST2* plugin = reinterpret_cast<PluginVST2*>(effect->resvd1);
	if (plugin)
		return plugin->HostCallback(effect, opcode, index, value, ptr, opt);
	return 0;
}

void PluginVST2::Resume() {
	Dispatcher(AEffectOpcodes::effMainsChanged, 0, true);
	StopProcessing();
	active = true;
}

void PluginVST2::Suspend() {
	StopProcessing();
	Dispatcher(AEffectOpcodes::effMainsChanged, 0, false);
	active = false;
}

void PluginVST2::StartProcessing() {
	Dispatcher(AEffectXOpcodes::effStartProcess);
}

void PluginVST2::StopProcessing() {
	Dispatcher(AEffectXOpcodes::effStopProcess);
}

VstIntPtr VSTCALLBACK PluginVST2::Dispatcher(VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt) {
	return plugin->dispatcher(plugin.get(), opcode, index, value, ptr, opt);
}

VstIntPtr VSTCALLBACK PluginVST2::HostCallback(AEffect *effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void *ptr, float opt) {
	switch (opcode) {
		case AudioMasterOpcodes::audioMasterVersion:
			return 2400;
		case AudioMasterOpcodes::audioMasterIdle:
			return 0; // plugin gives idle time to host
		case AudioMasterOpcodesX::audioMasterGetTime:
		case AudioMasterOpcodesX::audioMasterProcessEvents:
		case AudioMasterOpcodesX::audioMasterIOChanged:
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
			std::memcpy(ptr, "jperek", 7); // kVstMaxVendorStrLen
			return 1;
		case AudioMasterOpcodesX::audioMasterGetProductString:
			std::memcpy(ptr, "VSTHost", 8); // kVstMaxProductStrLen
			return 1;
		case AudioMasterOpcodesX::audioMasterGetVendorVersion:
			return 1000;
		case AudioMasterOpcodesX::audioMasterVendorSpecific:
			return 0;
		case AudioMasterOpcodesX::audioMasterCanDo:
			if (std::strcmp(static_cast<char*>(ptr), "sendVstEvents") == 0)
				return -1;
			else if (std::strcmp(static_cast<char*>(ptr), "sendVstMidiEvent") == 0)
				return -1;
			else if (std::strcmp(static_cast<char*>(ptr), "sendVstTimeInfo") == 0)
				return -1;
			else if (std::strcmp(static_cast<char*>(ptr), "receiveVstEvents") == 0)
				return -1;
			else if (std::strcmp(static_cast<char*>(ptr), "receiveVstMidiEvent") == 0)
				return -1;
			else if (std::strcmp(static_cast<char*>(ptr), "reportConnectionChanges") == 0)
				return -1;
			else if (std::strcmp(static_cast<char*>(ptr), "acceptIOChanges") == 0)
				return 1;
			else if (std::strcmp(static_cast<char*>(ptr), "sizeWindow") == 0)
				return -1;
			else if (std::strcmp(static_cast<char*>(ptr), "offline") == 0)
				return -1;
			else if (std::strcmp(static_cast<char*>(ptr), "openFileSelector") == 0)
				return -1;
			else if (std::strcmp(static_cast<char*>(ptr), "closeFileSelector") == 0)
				return -1;
			else if (std::strcmp(static_cast<char*>(ptr), "startStopProcess") == 0)
				return 1;
			else if (std::strcmp(static_cast<char*>(ptr), "shellCategory") == 0)
				return -1;
			else if (std::strcmp(static_cast<char*>(ptr), "sendVstMidiEventFlagIsRealtime") == 0)
				return -1;
			return 0;
		case AudioMasterOpcodesX::audioMasterGetLanguage:
			return VstHostLanguage::kVstLangEnglish;
		case AudioMasterOpcodesX::audioMasterGetDirectory: {
			auto buf = GetPluginDirectory();
			std::memcpy(ptr, buf.c_str(), buf.size());
			return 1;
		}
		case AudioMasterOpcodesX::audioMasterUpdateDisplay:
			if (gui)
				gui->Refresh();
			return 1;
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

bool PluginVST2::CanDo(const char *canDo) const {
	return (plugin->dispatcher(plugin.get(), AEffectXOpcodes::effCanDo, 0, 0, (void *)canDo, 0.) != 0);
}

Steinberg::int32 PluginVST2::GetVendorVersion() const {
	return plugin->dispatcher(plugin.get(), AEffectXOpcodes::effGetVendorVersion, 0, 0, nullptr, 0.);
}

Steinberg::int32 PluginVST2::GetVSTVersion() const {
	return plugin->dispatcher(plugin.get(), AEffectXOpcodes::effGetVstVersion, 0, 0, nullptr, 0.);
}

Steinberg::int32 PluginVST2::GetFlags() const {
	return plugin->flags;
}
} // namespace