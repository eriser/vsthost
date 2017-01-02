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
PluginVST3::PluginVST3(HMODULE m, Steinberg::IPluginFactory* f) : Plugin(m), factory(f) {
	pd.inputs = nullptr;
	pd.outputs = nullptr;
	pd.inputParameterChanges = nullptr;
	pd.outputParameterChanges = nullptr;
	factory->addRef();
	Steinberg::PClassInfo ci;
	Steinberg::tresult result;
	bool initialized = false;
	for (decltype(factory->countClasses()) i = 0; i < factory->countClasses(); ++i) {
		factory->getClassInfo(i, &ci);
		result = factory->createInstance(ci.cid, FUnknown::iid, reinterpret_cast<void**>(&plugin));
		if (result == Steinberg::kResultOk && plugin) {
			result = plugin->queryInterface(Steinberg::Vst::IComponent::iid, reinterpret_cast<void**>(&processorComponent));
			if (result == Steinberg::kResultOk && processorComponent) {
				
				result = plugin->queryInterface(Steinberg::Vst::IEditController::iid, reinterpret_cast<void**>(&editController));
				if (result != Steinberg::kResultOk && processorComponent) {
					Steinberg::FUID controllerCID;
					if (processorComponent->getControllerClassId(controllerCID) == Steinberg::kResultTrue && controllerCID.isValid())
						result = factory->createInstance(controllerCID, Steinberg::Vst::IEditController::iid, (void**)&editController);
				}
				if (result == Steinberg::kResultOk)
					if (initialized = (processorComponent->initialize(UnknownCast()) == Steinberg::kResultOk)) {
						result = processorComponent->queryInterface(Steinberg::Vst::IAudioProcessor::iid, reinterpret_cast<void**>(&audio));
						processorComponent_initialized = true;
					}
			}
		}
		if (IsValid() && result == Steinberg::kResultOk)
			break;
		if (initialized) {
			processorComponent->terminate();
			processorComponent_initialized = false;
		}
		processorComponent = nullptr;
		editController = nullptr;
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
	if (processorComponent) {
		if (processorComponent_initialized)
			processorComponent->terminate();
		processorComponent->release();
	}
	if (editController) {
		if (editController_initialized)
			editController->terminate();
		editController->release();
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

bool PluginVST3::IsValid() {
	Steinberg::IPluginFactory2* factory2 = nullptr;
	factory->queryInterface(Steinberg::IPluginFactory2::iid, reinterpret_cast<void**>(&factory2));
	if (factory2) {
		Steinberg::PClassInfo2 ci2;
		factory2->getClassInfo2(0, &ci2);
		factory2->release();
		if (!std::strcmp(ci2.category, "Audio Module Class") && ci2.subCategories[0] == 'F' && ci2.subCategories[1] == 'x'
			&& editController && audio && processorComponent)
			return true;
	}
	return false;
}

void PluginVST3::Initialize() {
	// initialize edit controller (processor component is already initialized)
	editController->initialize(UnknownCast());
	editController_initialized = true;
	editController->setComponentHandler(this);

	// check if plugin has editor and remember it
	auto tmp = editController->createView(Steinberg::Vst::ViewType::kEditor);
	has_editor = tmp != nullptr;
	if (tmp)
		tmp->release();

	// check for bypass parameter (soft bypass) and for preset change parameter
	Steinberg::Vst::ParameterInfo pi;
	static Steinberg::Vst::ParamID kNoParamId = -1;
	for (Steinberg::int32 i = 0; i < editController->getParameterCount() && (bypass_param_id == -1 || program_change_param_id == -1); ++i) {
		editController->getParameterInfo(i, pi);
		if (pi.flags & Steinberg::Vst::ParameterInfo::ParameterFlags::kIsBypass)
			bypass_param_id = pi.id;
		else if (pi.flags & Steinberg::Vst::ParameterInfo::ParameterFlags::kIsProgramChange) {
			program_change_param_id = pi.id;
			program_change_param_idx = i;
		}
	}

	// establish program count
	editController->queryInterface(Steinberg::Vst::IUnitInfo::iid, reinterpret_cast<void**>(&unit_info));
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
	pd.inputs->numChannels = Steinberg::Vst::SpeakerArr::getChannelCount(speaker_arrangement);
	pd.outputs = new Steinberg::Vst::AudioBusBuffers;
	pd.outputs->numChannels = Steinberg::Vst::SpeakerArr::getChannelCount(speaker_arrangement);

	// create parameter changes
	if (editController) {
		auto param_count = editController->getParameterCount();
		pd.inputParameterChanges = new Steinberg::Vst::ParameterChanges(param_count);
		pd.outputParameterChanges = new Steinberg::Vst::ParameterChanges(param_count);
	}
	SetActive(true);

	// synchronize controller and processor
	processorComponent->queryInterface(Steinberg::Vst::IConnectionPoint::iid, (void**)&iConnectionPointComponent);
	editController->queryInterface(Steinberg::Vst::IConnectionPoint::iid, (void**)&iConnectionPointController);
	if (iConnectionPointComponent && iConnectionPointController) {
		iConnectionPointComponent->connect(iConnectionPointController);
		iConnectionPointController->connect(iConnectionPointComponent);
	}
	Steinberg::MemoryStream stream;
	if (processorComponent->getState(&stream) == Steinberg::kResultTrue) {
		stream.seek(0, Steinberg::IBStream::kIBSeekSet, 0);
		editController->setComponentState(&stream);
	}

	// create plugin state module
	state = std::unique_ptr<Preset>(new PresetVST3(*this, GetPluginName()));
}

std::string PluginVST3::GetPluginName() {
	Steinberg::PClassInfo ci;
	factory->getClassInfo(0, &ci);
	return std::string(ci.name);
}

void PluginVST3::Process(Steinberg::Vst::Sample32** input, Steinberg::Vst::Sample32** output) {
	if (IsActive()) {
		if (BypassProcess()) // hard bypass
			for (unsigned i = 0; i < GetChannelCount(); ++i)
				std::memcpy(static_cast<void*>(output[i]), static_cast<void*>(input[i]), sizeof(input[0][0]));
		else {
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
	}
}

void PluginVST3::UpdateBlockSize() {
	bool was_active;
	if (was_active = IsActive())
		SetActive(false);
	pd.numSamples = block_size;
	if (was_active)
		SetActive(true);
}

void PluginVST3::UpdateSampleRate() {
	bool was_active;
	if (was_active = IsActive())
		SetActive(false);
	Steinberg::Vst::ProcessSetup ps;
	ps.symbolicSampleSize = Steinberg::Vst::kSample32;
	ps.processMode = Steinberg::Vst::kRealtime;
	ps.sampleRate = sample_rate;
	audio->setupProcessing(ps);
	if (was_active)
		SetActive(true);
}

void PluginVST3::UpdateSpeakerArrangement() {
	bool was_active;
	if (was_active = IsActive())
		SetActive(false);
	audio->setBusArrangements(&speaker_arrangement, 1, &speaker_arrangement, 1);
	if (was_active)
		SetActive(true);
}

Steinberg::int32 PluginVST3::GetProgramCount() {
	return program_count;
}

void PluginVST3::SetProgram(Steinberg::int32 id) {
	if (id < program_count && program_change_param_id != -1) {
		Steinberg::Vst::ParameterInfo param_info{};
		if (editController->getParameterInfo(program_change_param_idx, param_info) == Steinberg::kResultTrue) {
			if (param_info.stepCount > 0 && id <= param_info.stepCount) {
				auto value = static_cast<Steinberg::Vst::ParamValue>(id) / static_cast<Steinberg::Vst::ParamValue>(param_info.stepCount);
				SetParameter(program_change_param_id, value);
			}
		}
	}
}

Steinberg::int32 PluginVST3::GetParameterCount() {
	return editController->getParameterCount();
}

Steinberg::Vst::ParamValue PluginVST3::GetParameter(Steinberg::Vst::ParamID id) {
	return 0;
}

void PluginVST3::SetParameter(Steinberg::Vst::ParamID id, Steinberg::Vst::ParamValue value) {
	beginEdit(id);
	editController->setParamNormalized(id, value);
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

bool PluginVST3::BypassProcess() {			// wywolanie process omijaj tylko wtedy, jak
	return bypass && bypass_param_id == -1;	// bypass == true i nie znaleziono parametru "bypass"
}

bool PluginVST3::HasEditor() {
	return has_editor;
}

void PluginVST3::CreateEditor(HWND hWnd) {
	if (!gui && HasEditor()) {
		Steinberg::IPlugView* create_view = nullptr;
		if ((create_view = editController->createView(Steinberg::Vst::ViewType::kEditor)) != nullptr) {
			gui = std::unique_ptr<PluginWindow>(new PluginVST3Window(*this, create_view));
			gui->Initialize(hWnd);
		}
	}
}

void PluginVST3::PrintFactory() {
	Steinberg::PFactoryInfo factoryInfo;
	factory->getFactoryInfo(&factoryInfo);
	std::cout << "  Factory Info:\n\tvendor = " << factoryInfo.vendor
		<< "\n\turl = " << factoryInfo.url
		<< "\n\temail = " << factoryInfo.email << std::endl;
	Steinberg::IPluginFactory2* factory2 = nullptr;
	factory->queryInterface(Steinberg::IPluginFactory2::iid, reinterpret_cast<void **>(&factory2));
	for (Steinberg::int32 i = 0; i < factory->countClasses(); i++) {
		Steinberg::PClassInfo ci;
		Steinberg::PClassInfo2 ci2;
		factory->getClassInfo(i, &ci);
		//PrintClass(ci, i);
		if (factory2) {
			factory2->getClassInfo2(i, &ci2);
			PrintClass2(ci2, i);
		}
	}
	if (factory2)
		factory2->release();
}

void PluginVST3::PrintClass(const Steinberg::PClassInfo &ci, int i) {
	std::cout << "  Class Info " << i << ":\n\tname = " << ci.name
		<< "\n\tcategory = " << ci.category << std::endl;
}

void PluginVST3::PrintClass2(const Steinberg::PClassInfo2 &ci, int i) {
	std::cout << "  Class Info " << i << ":\n\tname = " << ci.name
		<< "\n\tcategory = " << ci.category << std::endl
		<< "\tsubCategory = " << ci.subCategories << std::endl;
}

void PluginVST3::PrintBusInfo() {
	if (processorComponent) {
		std::cout << "-----------BUSES------------" << std::endl;
		Steinberg::Vst::BusInfo bi;
		for (Steinberg::int32 i = 0; i < processorComponent->getBusCount(Steinberg::Vst::kAudio, Steinberg::Vst::kInput); ++i) {
			processorComponent->getBusInfo(Steinberg::Vst::kAudio, Steinberg::Vst::kInput, i, bi);
			std::wcout << "Inputs:" << std::endl
				<< "Channel count: " << bi.channelCount << std::endl
				<< "Name: " << bi.name << std::endl
				<< "Bus type: " << bi.busType << std::endl
				<< "Flags: " << bi.flags << std::endl;
			//std::wcout << bi.name << std::endl;
		}
		std::cout << "---------------------------" << std::endl;
		for (Steinberg::int32 i = 0; i < processorComponent->getBusCount(Steinberg::Vst::kAudio, Steinberg::Vst::kOutput); ++i) {
			processorComponent->getBusInfo(Steinberg::Vst::kAudio, Steinberg::Vst::kOutput, i, bi);
			std::wcout << "Outputs:" << std::endl
				<< "Channel count: " << bi.channelCount << std::endl
				<< "Name: " << bi.name << std::endl
				<< "Bus type: " << bi.busType << std::endl
				<< "Flags: " << bi.flags << std::endl;
		}
	}
}

void PluginVST3::PrintParameters() {
	if (editController) {
		Steinberg::Vst::ParameterInfo pi;
		for (Steinberg::int32 i = 0; i < editController->getParameterCount() - 20; ++i) {
			editController->getParameterInfo(i, pi);
			std::wcout << "ParamID: " << pi.id << std::endl
				<< "Title: " << pi.title << std::endl
				<< "shortTitle: " << pi.shortTitle << std::endl
				<< "units: " << pi.units << std::endl
				<< "stepCount: " << pi.stepCount << std::endl
				<< "defaultNormalizedValue: " << pi.defaultNormalizedValue << std::endl
				<< "unitID: " << pi.unitId << std::endl
				<< "flags: " << pi.flags << "\n================" << std::endl;
		}
	}
}

void PluginVST3::PrintInfo() {

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
	processorComponent->setActive(true);
	StartProcessing();
	active = true;
}

void PluginVST3::Suspend() {
	StopProcessing();
	processorComponent->setActive(false);
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
		editController->setParamNormalized(q->getParameterId(), value);
	}
}

Steinberg::FUnknown* PluginVST3::UnknownCast() {
	return static_cast<Steinberg::FObject *>(this);
}
} // namespace