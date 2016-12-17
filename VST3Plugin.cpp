#include "VST3Plugin.h"

#include "pluginterfaces/vst/ivstmessage.h"

#include "public.sdk/source/common/memorystream.h"

#include "pluginterfaces/gui/iplugview.h"

#include "VST3Preset.h"
#include <cstring>
#include "base\source\fstring.h"

VST3Plugin::VST3Plugin(HMODULE m, Steinberg::IPluginFactory* f) : Plugin(m), factory(f) {
	Steinberg::tresult res;
	Steinberg::PClassInfo ci;
	factory->getClassInfo(0, &ci);
	if (factory->createInstance(ci.cid, FUnknown::iid, reinterpret_cast<void**>(&plugin)) == Steinberg::kResultOk && plugin) {
		res = plugin->queryInterface(Steinberg::Vst::IComponent::iid, reinterpret_cast<void**>(&processorComponent));
		if (res == Steinberg::kResultOk && processorComponent)
			res = processorComponent->initialize(UnknownCast());
		res = (plugin->queryInterface(Steinberg::Vst::IEditController::iid, reinterpret_cast<void**>(&editController)));
		if (res != Steinberg::kResultOk) {
			Steinberg::FUID controllerCID;
			if (processorComponent->getControllerClassId(controllerCID) == Steinberg::kResultTrue && controllerCID.isValid())
				res = factory->createInstance(controllerCID, Steinberg::Vst::IEditController::iid, (void**)&editController);
		}
		if (res == Steinberg::kResultOk && editController) {
			editController->initialize(UnknownCast());
			editController->setComponentHandler(this);

			// check if plugin has editor and remember it
			auto tmp = editController->createView(Steinberg::Vst::ViewType::kEditor);
			has_editor = tmp != nullptr;
			if (tmp)
				tmp->release();

			// check for bypass parameter (soft bypass) and for preset change parameter
			Steinberg::Vst::UnitID program_change_unitid;
			Steinberg::Vst::ParameterInfo pi;
			static Steinberg::Vst::ParamID kNoParamId = -1;
			for (Steinberg::int32 i = 0; i < editController->getParameterCount() && (bypass_param_id == -1 || program_change_param_id == -1); ++i) {
				editController->getParameterInfo(i, pi);
				if (pi.flags & Steinberg::Vst::ParameterInfo::ParameterFlags::kIsBypass) {
					bypass_param_id = pi.id;
				}
				else if (pi.flags & Steinberg::Vst::ParameterInfo::ParameterFlags::kIsProgramChange) {
					program_change_param_id = pi.id;
					program_change_param_idx = i;
					program_change_unitid = pi.unitId;
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

			Steinberg::Vst::IConnectionPoint* iConnectionPointComponent = nullptr;
			Steinberg::Vst::IConnectionPoint* iConnectionPointController = nullptr;
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
		}
		
		SetupAudio();
		//PrintFactory();
	}
	state = new VST3Preset(processorComponent, editController, GetPluginName());
}

VST3Plugin::~VST3Plugin() {
	if (audio)
		audio->release();
	if (processorComponent)
		processorComponent->release();
	if (editController)
		editController->release();
	if (plugin) 
		plugin->release();
	if (factory)
		factory->release();
	void* exitProc = nullptr;
	if (module)
		exitProc = GetProcAddress(module, "ExitDll");
		if (exitProc)
			static_cast<ExitModuleProc>(exitProc)();
		FreeLibrary(module);
}

void VST3Plugin::SetupAudio() {
	if (processorComponent) {
		Steinberg::tresult res = processorComponent->queryInterface(Steinberg::Vst::IAudioProcessor::iid, reinterpret_cast<void**>(&audio));
		if (res == Steinberg::kResultOk) {
			SetActive(false);
			// PROCESS SETUP
			Steinberg::Vst::ProcessSetup ps;
			if (audio->canProcessSampleSize(Steinberg::Vst::kSample32) == Steinberg::kResultOk)
				ps.symbolicSampleSize = Steinberg::Vst::kSample32;
			else
				ps.symbolicSampleSize = Steinberg::Vst::kSample64; // zamienic
			ps.processMode = Steinberg::Vst::kRealtime;
			ps.sampleRate = sample_rate;
			audio->setupProcessing(ps);

			// PROCESS DATA
			if (audio->canProcessSampleSize(Steinberg::Vst::kSample32) == Steinberg::kResultOk)
				pd.symbolicSampleSize = Steinberg::Vst::kSample32;
			else
				pd.symbolicSampleSize = Steinberg::Vst::kSample64; // zamienic
			pd.numSamples = block_size;
			pd.processMode = Steinberg::Vst::kRealtime;
			pd.numInputs = 1;
			pd.numOutputs = 1;
			pd.inputs = new Steinberg::Vst::AudioBusBuffers;
			pd.inputs->numChannels = Steinberg::Vst::SpeakerArr::getChannelCount(speaker_arrangement);
			pd.outputs = new Steinberg::Vst::AudioBusBuffers;
			pd.outputs->numChannels = Steinberg::Vst::SpeakerArr::getChannelCount(speaker_arrangement);

			if (editController) {
				auto param_count = editController->getParameterCount();
				pd.inputParameterChanges = new Steinberg::Vst::ParameterChanges(param_count);
				pd.outputParameterChanges = new Steinberg::Vst::ParameterChanges(param_count);
			}


			SetActive(true);
		}
	}
	else
		std::cout << "processor component error" << std::endl;
}

void VST3Plugin::Process(Steinberg::Vst::Sample32** input, Steinberg::Vst::Sample32** output) {
	if (IsActive()) {
		if (BypassProcess()) // hard bypass
			for (unsigned i = 0; i < GetChannelCount(); ++i)
				std::memcpy(static_cast<void*>(output[i]), static_cast<void*>(input[i]), sizeof(input[0][0]));
		else {
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

void VST3Plugin::ProcessOutputParameterChanges() {
	for (unsigned i = 0; i < static_cast<unsigned>(pd.outputParameterChanges->getParameterCount()); ++i) {
		auto q = pd.outputParameterChanges->getParameterData(i);
		Steinberg::Vst::ParamValue value;
		Steinberg::int32 offset;
		q->getPoint(q->getPointCount() - 1, offset, value);
		editController->setParamNormalized(q->getParameterId(), value);
	}
}

bool VST3Plugin::IsValid() {
	Steinberg::IPluginFactory2* factory2 = nullptr;
	factory->queryInterface(Steinberg::IPluginFactory2::iid, reinterpret_cast<void**>(&factory2));
	if (factory2) {
		Steinberg::PClassInfo2 ci2;
		factory2->getClassInfo2(0, &ci2);
		factory2->release();
		if (!std::strcmp(ci2.category, "Audio Module Class") && ci2.subCategories[0] == 'F' && ci2.subCategories[1] == 'x')
			return true;
	}
	return false;
}

void VST3Plugin::PrintInfo() {

}

void VST3Plugin::PrintFactory() {
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

void VST3Plugin::PrintClass(const Steinberg::PClassInfo &ci, int i) {
	std::cout << "  Class Info " << i << ":\n\tname = " << ci.name
		<< "\n\tcategory = " << ci.category << std::endl;
}

void VST3Plugin::PrintClass2(const Steinberg::PClassInfo2 &ci, int i) {
	std::cout << "  Class Info " << i << ":\n\tname = " << ci.name
		<< "\n\tcategory = " << ci.category << std::endl
		<< "\tsubCategory = " << ci.subCategories << std::endl;
}

void VST3Plugin::PrintBusInfo() {
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

void VST3Plugin::PrintParameters() {
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

std::string VST3Plugin::GetPluginName() {
	Steinberg::PClassInfo ci;
	factory->getClassInfo(0, &ci);
	return std::string(ci.name);
}

Steinberg::FUnknown* VST3Plugin::UnknownCast() {
	return static_cast<Steinberg::FObject *>(this); 
}

Steinberg::tresult PLUGIN_API VST3Plugin::beginEdit(Steinberg::Vst::ParamID id) {
	current_queue = pd.inputParameterChanges->addParameterData(id, current_param_idx);
	offset = 0;
	return Steinberg::kResultTrue;
}

Steinberg::tresult PLUGIN_API VST3Plugin::performEdit(Steinberg::Vst::ParamID id, Steinberg::Vst::ParamValue valueNormalized) {
	Steinberg::int32 index;
	if (!current_queue)
		current_queue = pd.inputParameterChanges->addParameterData(id, current_param_idx);
	current_queue->addPoint(offset++, valueNormalized, index);
	return Steinberg::kResultTrue;
}

Steinberg::tresult PLUGIN_API VST3Plugin::endEdit(Steinberg::Vst::ParamID id) {
	return Steinberg::kResultTrue;
}

Steinberg::tresult PLUGIN_API VST3Plugin::restartComponent(Steinberg::int32 flags) {
	return Steinberg::kResultFalse;
}

Steinberg::IPlugView* VST3Plugin::CreateView() {
	return editController->createView("editor");
}

std::vector<std::string> VST3Plugin::GetPresets() {
	std::vector<std::string> ret;
	Steinberg::Vst::ProgramListInfo list_info{};
	for (Steinberg::uint32 i = 0; i < program_count; ++i) {
		if (unit_info->getProgramListInfo(0, list_info) == Steinberg::kResultTrue) {
			Steinberg::Vst::String128 tmp = { 0 };
			if (unit_info->getProgramName(list_info.id, i, tmp) == Steinberg::kResultTrue)
				//	ret.emplace_back(tmp);
				std::wcout << tmp << std::endl;
		}
	}
	return ret;
}

bool VST3Plugin::HasEditor() {
	return has_editor;
}

Steinberg::uint32 VST3Plugin::GetProgramCount() {
	return program_count;
}

void VST3Plugin::SetProgram(Steinberg::int32 id) {
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

Steinberg::uint32 VST3Plugin::GetParameterCount() {
	return editController->getParameterCount();
}

Steinberg::Vst::ParamValue VST3Plugin::GetParameter(Steinberg::Vst::ParamID id) {
	return 0;
}

void VST3Plugin::SetParameter(Steinberg::Vst::ParamID id, Steinberg::Vst::ParamValue value) {
	beginEdit(id);
	editController->setParamNormalized(id, value);
	performEdit(id, value);
	endEdit(id);
}

void VST3Plugin::SaveState() {
	Plugin::SaveState();
}

void VST3Plugin::LoadState() {
	Plugin::LoadState();
}

void VST3Plugin::UpdateBlockSize() {
	bool was_active;
	if (was_active = IsActive())
		SetActive(false);
	pd.numSamples = block_size;
	if (was_active)
		SetActive(true);
}

void VST3Plugin::UpdateSampleRate() {
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

void VST3Plugin::UpdateSpeakerArrangement() {
	bool was_active;
	if (was_active = IsActive())
		SetActive(false);
	audio->setBusArrangements(&speaker_arrangement, 1, &speaker_arrangement, 1);
	if (was_active)
		SetActive(true);
}

void VST3Plugin::SetBypass(bool bypass_) {
	if (bypass != bypass_) {
		bypass = bypass_;
		if (bypass_param_id != -1) {
			Steinberg::Vst::ParamValue value = static_cast<Steinberg::Vst::ParamValue>(bypass);
			SetParameter(bypass_param_id, value);
		}

	}
}

void VST3Plugin::Resume() {
	processorComponent->setActive(true);
	StartProcessing();
	active = true;
}

void VST3Plugin::Suspend() {
	StopProcessing();
	processorComponent->setActive(false);
	active = false;
}

void VST3Plugin::StartProcessing() {
	audio->setProcessing(true);
}

void VST3Plugin::StopProcessing() {
	audio->setProcessing(false);
}

bool VST3Plugin::BypassProcess() {			// wywolanie process omijaj tylko wtedy, jak
	return bypass && bypass_param_id == -1;	// bypass == true i nie znaleziono parametru "bypass"
}