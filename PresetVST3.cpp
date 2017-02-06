#include "PresetVST3.h"

#include <fstream>
#include <iostream>
#include <vector>

#include "public.sdk/source/vst/vstpresetfile.h"

#include "PluginVST3.h"

namespace VSTHost {
const std::string PresetVST3::kExtension{ "vstpreset" };

PresetVST3::PresetVST3(PluginVST3& p) : plugin(p) {
	// preset file path
	//preset_file_path = Plugin::kPluginDirectory + plugin.GetPluginFileName();
	preset_file_path = plugin.GetPluginPath();
	std::string::size_type pos = 0;
	if ((pos = preset_file_path.find_last_of('.')) != std::string::npos)
		preset_file_path = preset_file_path.substr(0, pos);
	preset_file_path += "." + kExtension;
	// FUID of the component (processor) part
	Steinberg::PClassInfo ci;
	plugin.factory->getClassInfo(plugin.class_index, &ci);
	fuid = ci.cid;
}

PresetVST3::~PresetVST3() {}

void PresetVST3::SetState() {
	if (processor_stream.getSize() > 0) {
		plugin.processor_component->setState(&processor_stream);
		processor_stream.seek(0, Steinberg::IBStream::kIBSeekSet, 0);
		if (edit_stream.getSize() > 0)
			plugin.edit_controller->setState(&edit_stream);
		plugin.edit_controller->setComponentState(&processor_stream);
	}
	edit_stream.seek(0, Steinberg::IBStream::kIBSeekSet, 0);
	processor_stream.seek(0, Steinberg::IBStream::kIBSeekSet, 0);
}

void PresetVST3::LoadFromFile() {
	std::ifstream file(preset_file_path, std::ifstream::binary | std::ifstream::in);
	if (file.is_open()) {
		std::vector<char> v;
		while (file.good()) {
			char c;
			if (file.get(c))
				v.push_back(c);
		}
		Steinberg::MemoryStream in(&v[0], v.size());
		if (Steinberg::Vst::PresetFile::loadPreset(&in, fuid, plugin.processor_component, plugin.edit_controller))
			GetState(); // preset was loaded successfully, so i update the state of this object
		file.close();
	}
}

void PresetVST3::GetState() {
	if (plugin.processor_component->getState(&processor_stream) != Steinberg::kResultTrue)
		processor_stream.setSize(0);
	if (plugin.edit_controller->getState(&edit_stream) != Steinberg::kResultTrue)
		edit_stream.setSize(0);
	edit_stream.seek(0, Steinberg::IBStream::kIBSeekSet, 0);
	processor_stream.seek(0, Steinberg::IBStream::kIBSeekSet, 0);
}

void PresetVST3::SaveToFile() {
	GetState();
	Steinberg::MemoryStream out;
	if (Steinberg::Vst::PresetFile::savePreset(&out, fuid, plugin.processor_component, plugin.edit_controller)) {
		std::ofstream file(preset_file_path, std::ofstream::binary | std::ofstream::out | std::ofstream::trunc);
		if (file.is_open()) {
			file.write(out.getData(), out.getSize());
			file.close();
		}
	}
}
} // namespace