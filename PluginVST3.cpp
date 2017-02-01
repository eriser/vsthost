#include "PluginVST3.h"

#include <iostream>

#include "public.sdk/source/common/memorystream.h"
#include "pluginterfaces/gui/iplugview.h"
#include "base/source/fstring.h"
#include "pluginterfaces/base/ipluginbase.h"
#include "pluginterfaces/vst/ivstcomponent.h"

#include "PluginVST3Window.h"
#include "PresetVST3.h"

extern "C" typedef bool (PLUGIN_API *VST3ExitProc)();

namespace VSTHost {
PluginVST3::PluginVST3(HMODULE m, Steinberg::IPluginFactory* f, Steinberg::Vst::TSamples bs, Steinberg::Vst::SampleRate sr)
	: Plugin(m, bs, sr), factory(f), class_index(0) {
	pd.inputs = nullptr;
	pd.outputs = nullptr;
	pd.inputParameterChanges = nullptr;
	pd.outputParameterChanges = nullptr;
	Steinberg::PClassInfo ci;
	Steinberg::tresult result;
	bool initialized = false;
	for (decltype(factory->countClasses()) i = 0; i < factory->countClasses(); ++i) {
		class_index = i;
		factory->getClassInfo(class_index, &ci);
		result = factory->createInstance(ci.cid, FUnknown::iid, reinterpret_cast<void**>(&plugin));
		if (result == Steinberg::kResultOk && plugin) {
			result = plugin->queryInterface(Steinberg::Vst::IComponent::iid, reinterpret_cast<void**>(&processor_component));
			if (result == Steinberg::kResultOk && processor_component) {
				result = plugin->queryInterface(Steinberg::Vst::IEditController::iid, reinterpret_cast<void**>(&edit_controller));
				if (result != Steinberg::kResultOk && processor_component) {
					Steinberg::FUID controllerCID;
					if (processor_component->getControllerClassId(controllerCID) == Steinberg::kResultTrue && controllerCID.isValid())
						result = factory->createInstance(controllerCID, Steinberg::Vst::IEditController::iid, (void**)&edit_controller);
				}
				if (result == Steinberg::kResultOk)
					if (initialized = (processor_component->initialize(UnknownCast()) == Steinberg::kResultOk)) {
						result = processor_component->queryInterface(Steinberg::Vst::IAudioProcessor::iid, reinterpret_cast<void**>(&audio));
						processor_component_initialized = true;
					}
			}
		}
		if (IsValid() && result == Steinberg::kResultOk)
			break;
		if (initialized) {
			processor_component->terminate();
			processor_component_initialized = false;
		}
		processor_component = nullptr;
		edit_controller = nullptr;
		audio = nullptr;
		initialized = false;
	}
}

PluginVST3::~PluginVST3() {
	SetActive(false);
	gui.reset();
	state.reset(); // gui and state have to be destroyed before the rest of the plugin is freed...
	if (iConnectionPointComponent && iConnectionPointController) {
		iConnectionPointComponent->disconnect(iConnectionPointController);
		iConnectionPointController->disconnect(iConnectionPointComponent);
		iConnectionPointComponent->release();
		iConnectionPointController->release();
	}
	if (pd.inputs)
		delete pd.inputs;
	if (pd.outputs)
		delete pd.outputs;
	if (pd.inputParameterChanges)
		delete pd.inputParameterChanges;
	if (pd.outputParameterChanges)
		delete pd.outputParameterChanges;
	if (audio)
		audio->release();
	if (unit_info)
		unit_info->release();
	if (processor_component) {
		if (processor_component_initialized)
			processor_component->terminate();
		processor_component->release();
	}
	if (edit_controller) {
		if (edit_controller_initialized)
			edit_controller->terminate();
		edit_controller->release();
	}
	if (plugin)
		plugin->release();
	if (factory)
		factory->release();
	void* exitProc = nullptr;
	exitProc = ::GetProcAddress(module, "ExitDll");
	if (exitProc)
		static_cast<VST3ExitProc>(exitProc)();
}

bool PluginVST3::IsValid() const {
	Steinberg::IPluginFactory2* factory2 = nullptr;
	factory->queryInterface(Steinberg::IPluginFactory2::iid, reinterpret_cast<void**>(&factory2));
	if (factory2) {
		Steinberg::PClassInfo2 ci2;
		factory2->getClassInfo2(class_index, &ci2);
		factory2->release();
		if (!std::strcmp(ci2.category, "Audio Module Class") && ci2.subCategories[0] == 'F' && ci2.subCategories[1] == 'x'
			&& edit_controller && audio && processor_component)
			return true;
	}
	return false;
}

void PluginVST3::Initialize() {
	// initialize edit controller (processor component is already initialized)
	edit_controller->initialize(UnknownCast());
	edit_controller_initialized = true;
	edit_controller->setComponentHandler(this);

	// check if plugin has editor and remember it
	auto tmp = edit_controller->createView(Steinberg::Vst::ViewType::kEditor);
	has_editor = tmp != nullptr;
	if (tmp)
		tmp->release();

	// check for bypass parameter (soft bypass) and for preset change parameter
	Steinberg::Vst::ParameterInfo pi;
	static Steinberg::Vst::ParamID kNoParamId = -1;
	for (Steinberg::int32 i = 0; i < edit_controller->getParameterCount() && (bypass_param_id == -1 || program_change_param_id == -1); ++i) {
		edit_controller->getParameterInfo(i, pi);
		if (pi.flags & Steinberg::Vst::ParameterInfo::ParameterFlags::kIsBypass)
			bypass_param_id = pi.id;
		else if (pi.flags & Steinberg::Vst::ParameterInfo::ParameterFlags::kIsProgramChange) {
			program_change_param_id = pi.id;
			program_change_param_idx = i;
		}
	}

	// establish program count
	edit_controller->queryInterface(Steinberg::Vst::IUnitInfo::iid, reinterpret_cast<void**>(&unit_info));
	if (unit_info) {
		auto program_list_count = unit_info->getProgramListCount();
		if (program_list_count > 0) {
			Steinberg::int32 i = 0;
			Steinberg::Vst::UnitInfo unit{};
			while (i < unit_info->getUnitCount() && unit_info->getUnitInfo(i, unit) == Steinberg::kResultTrue && unit.id != Steinberg::Vst::kRootUnitId)
				++i; // there has got to be a root unit id if getUnitCount returns more than zero
			program_list_root = unit.programListId;
			Steinberg::Vst::ProgramListInfo prog_list{};
			i = 0;
			while (i < program_list_count && unit_info->getProgramListInfo(i, prog_list) == Steinberg::kResultTrue) {
				if (prog_list.id == program_list_root) {
					program_count = prog_list.programCount;
					break;
				}
				++i;
			}
		}
	}

	// setup audio stuff
	Steinberg::Vst::ProcessSetup ps{};
	if (audio->canProcessSampleSize(Steinberg::Vst::kSample32) == Steinberg::kResultOk)
		ps.symbolicSampleSize = Steinberg::Vst::kSample32;
	else
		ps.symbolicSampleSize = Steinberg::Vst::kSample64;
	ps.processMode = Steinberg::Vst::kRealtime;
	ps.sampleRate = sample_rate;
	audio->setupProcessing(ps);

	// setup process data
	if (audio->canProcessSampleSize(Steinberg::Vst::kSample32) == Steinberg::kResultOk)
		pd.symbolicSampleSize = Steinberg::Vst::kSample32;
	else
		pd.symbolicSampleSize = Steinberg::Vst::kSample64;
	pd.numSamples = block_size;
	pd.processMode = Steinberg::Vst::kRealtime;
	pd.numInputs = 1;
	pd.numOutputs = 1;
	pd.inputs = new Steinberg::Vst::AudioBusBuffers;
	pd.inputs->numChannels = GetChannelCount();
	pd.outputs = new Steinberg::Vst::AudioBusBuffers;
	pd.outputs->numChannels = GetChannelCount();

	// create parameter changes
	if (edit_controller) {
		auto param_count = edit_controller->getParameterCount();
		pd.inputParameterChanges = new Steinberg::Vst::ParameterChanges(param_count);
		pd.outputParameterChanges = new Steinberg::Vst::ParameterChanges(param_count);
	}
	SetActive(true);

	// synchronize controller and processor
	processor_component->queryInterface(Steinberg::Vst::IConnectionPoint::iid, (void**)&iConnectionPointComponent);
	edit_controller->queryInterface(Steinberg::Vst::IConnectionPoint::iid, (void**)&iConnectionPointController);
	if (iConnectionPointComponent && iConnectionPointController) {
		iConnectionPointComponent->connect(iConnectionPointController);
		iConnectionPointController->connect(iConnectionPointComponent);
	}
	Steinberg::MemoryStream stream;
	if (processor_component->getState(&stream) == Steinberg::kResultTrue) {
		stream.seek(0, Steinberg::IBStream::kIBSeekSet, 0);
		edit_controller->setComponentState(&stream);
	}

	// create plugin state module
	state = std::unique_ptr<Preset>(new PresetVST3(*this));
}

std::basic_string<TCHAR> PluginVST3::GetPluginName() const {
	Steinberg::PClassInfo ci;
	factory->getClassInfo(class_index, &ci);
#ifdef UNICODE // for some reason ci.name is single byte ASCII too
	std::string tmp(ci.name); // if name is not single byte string, this will fail 
	return std::basic_string<TCHAR>(tmp.begin(), tmp.end());
#else
	return std::basic_string<TCHAR>(ci.name);
#endif
}

void PluginVST3::Process(Steinberg::Vst::Sample32** input, Steinberg::Vst::Sample32** output, Steinberg::Vst::TSamples block_size) {
	if (IsActive() && !BypassProcess()) {
		std::lock_guard<std::mutex> lock(processing);
		pd.inputs->channelBuffers32 = input;
		pd.outputs->channelBuffers32 = output;
		pd.numSamples = block_size;
		audio->process(pd);
		ProcessOutputParameterChanges();
		dynamic_cast<Steinberg::Vst::ParameterChanges*>(pd.inputParameterChanges)->clearQueue();
		current_queue = nullptr;
		current_param_idx = -1;
	}
	else {
		for (unsigned i = 0; i < GetChannelCount(); ++i)
			std::memcpy(static_cast<void*>(output[i]), static_cast<void*>(input[i]), sizeof(input[0][0]) * block_size);
	}
}

void PluginVST3::SetBlockSize(Steinberg::Vst::TSamples bs) {
	bool was_active;
	if (was_active = IsActive())
		SetActive(false);
	block_size = bs;
	pd.numSamples = block_size;
	if (was_active)
		SetActive(true);
}

void PluginVST3::SetSampleRate(Steinberg::Vst::SampleRate sr) {
	bool was_active;
	if (was_active = IsActive())
		SetActive(false);
	sample_rate = sr;
	Steinberg::Vst::ProcessSetup ps;
	ps.symbolicSampleSize = Steinberg::Vst::kSample32;
	ps.processMode = Steinberg::Vst::kRealtime;
	ps.sampleRate = sample_rate;
	audio->setupProcessing(ps);
	if (was_active)
		SetActive(true);
}

Steinberg::int32 PluginVST3::GetProgramCount() const {
	return program_count;
}

void PluginVST3::SetProgram(Steinberg::int32 id) {
	if (id < program_count && program_change_param_id != -1) {
		Steinberg::Vst::ParameterInfo param_info{};
		if (edit_controller->getParameterInfo(program_change_param_idx, param_info) == Steinberg::kResultTrue) {
			if (param_info.stepCount > 0 && id <= param_info.stepCount) {
				auto value = static_cast<Steinberg::Vst::ParamValue>(id) / static_cast<Steinberg::Vst::ParamValue>(param_info.stepCount);
				SetParameter(program_change_param_id, value);
			}
		}
	}
}

Steinberg::int32 PluginVST3::GetParameterCount() const {
	return edit_controller->getParameterCount();
}

Steinberg::Vst::ParamValue PluginVST3::GetParameter(Steinberg::Vst::ParamID id) const {
	return edit_controller->getParamNormalized(id);
}

void PluginVST3::SetParameter(Steinberg::Vst::ParamID id, Steinberg::Vst::ParamValue value) {
	beginEdit(id);
	edit_controller->setParamNormalized(id, value);
	performEdit(id, value);
	endEdit(id);
}

void PluginVST3::SetBypass(bool bypass_) {
	if (bypass != bypass_) {
		bypass = bypass_;
		if (bypass_param_id != -1) {
			Steinberg::Vst::ParamValue value = static_cast<Steinberg::Vst::ParamValue>(bypass);
			SetParameter(bypass_param_id, value);
		}
	}
}

bool PluginVST3::BypassProcess() const {			// wywolanie process omijaj tylko wtedy, jak
	return bypass && bypass_param_id == -1;	// bypass == true i nie znaleziono parametru "bypass"
}

bool PluginVST3::HasEditor() const {
	return has_editor;
}

void PluginVST3::CreateEditor(HWND hWnd) {
	if (!gui && HasEditor()) {
		Steinberg::IPlugView* create_view = nullptr;
		if ((create_view = edit_controller->createView(Steinberg::Vst::ViewType::kEditor)) != nullptr) {
			gui = std::unique_ptr<PluginWindow>(new PluginVST3Window(*this, create_view));
			gui->Initialize(hWnd);
		}
	}
}

Steinberg::tresult PLUGIN_API PluginVST3::beginEdit(Steinberg::Vst::ParamID id) {
	current_queue = pd.inputParameterChanges->addParameterData(id, current_param_idx);
	offset = 0;
	return Steinberg::kResultTrue;
}

Steinberg::tresult PLUGIN_API PluginVST3::performEdit(Steinberg::Vst::ParamID id, Steinberg::Vst::ParamValue valueNormalized) {
	Steinberg::int32 index;
	if (!current_queue)
		current_queue = pd.inputParameterChanges->addParameterData(id, current_param_idx);
	current_queue->addPoint(offset++, valueNormalized, index);
	return Steinberg::kResultTrue;
}

Steinberg::tresult PLUGIN_API PluginVST3::endEdit(Steinberg::Vst::ParamID id) {
	return Steinberg::kResultTrue;
}

Steinberg::tresult PLUGIN_API PluginVST3::restartComponent(Steinberg::int32 flags) {
	return Steinberg::kResultFalse;
}

void PluginVST3::Resume() {
	processor_component->setActive(true);
	StartProcessing();
	active = true;
}

void PluginVST3::Suspend() {
	StopProcessing();
	processor_component->setActive(false);
	active = false;
}

void PluginVST3::StartProcessing() {
	audio->setProcessing(true);
}

void PluginVST3::StopProcessing() {
	audio->setProcessing(false);
}

void PluginVST3::ProcessOutputParameterChanges() {
	for (unsigned i = 0; i < static_cast<unsigned>(pd.outputParameterChanges->getParameterCount()); ++i) {
		auto q = pd.outputParameterChanges->getParameterData(i);
		Steinberg::Vst::ParamValue value;
		Steinberg::int32 offset;
		q->getPoint(q->getPointCount() - 1, offset, value);
		edit_controller->setParamNormalized(q->getParameterId(), value);
	}
}

Steinberg::FUnknown* PluginVST3::UnknownCast() {
	return static_cast<Steinberg::FObject *>(this);
}
} // namespace