#include "PresetVST3.h"

#include <fstream>

#include "PluginVST3.h"

namespace VSTHost {
	PresetVST3::PresetVST3(PluginVST3& p, std::basic_string<TCHAR> n) : plugin(p), name(n) {

}

PresetVST3::~PresetVST3() {

}

bool PresetVST3::SetState() {
	if (processor_stream.getSize() > 0) {
		plugin.processorComponent->setState(&processor_stream);
		processor_stream.seek(0, Steinberg::IBStream::kIBSeekSet, 0);
		if (edit_stream.getSize() > 0)
			plugin.editController->setState(&edit_stream);
		plugin.editController->setComponentState(&processor_stream);
	}
	edit_stream.seek(0, Steinberg::IBStream::kIBSeekSet, 0);
	processor_stream.seek(0, Steinberg::IBStream::kIBSeekSet, 0);
	return true;
}

void PresetVST3::LoadFromFile() {
	std::ifstream file(name + ".fxp", std::ifstream::binary | std::ifstream::in);
	if (file.is_open()) {
		decltype(processor_stream.getSize()) size;
		Steinberg::int32 read;
		//file.read(reinterpret_cast<char*>(&size), sizeof(size));
		file >> size;
		char* data = new char[size];
		file.read(data, size);
		processor_stream.setSize(0);
		processor_stream.write(static_cast<void*>(data), size, &read);
		delete[] data;
		//file.read(reinterpret_cast<char*>(&size), sizeof(size));
		file >> size;
		if (size > 0) {
			char* data = new char[size];
			file.read(data, size);
			edit_stream.setSize(0);
			edit_stream.write(static_cast<void*>(data), size, &read);
		}
		edit_stream.seek(0, Steinberg::IBStream::kIBSeekSet, 0);
		processor_stream.seek(0, Steinberg::IBStream::kIBSeekSet, 0);
		SetState();
		file.close();
	}
}

void PresetVST3::GetState() {
	if (plugin.processorComponent->getState(&processor_stream) != Steinberg::kResultTrue)
		processor_stream.setSize(0);
	if (plugin.editController->getState(&edit_stream) != Steinberg::kResultTrue)
		edit_stream.setSize(0);
	edit_stream.seek(0, Steinberg::IBStream::kIBSeekSet, 0);
	processor_stream.seek(0, Steinberg::IBStream::kIBSeekSet, 0);
}

void PresetVST3::SaveToFile() {
	std::ofstream file(name + ".fxp", std::ofstream::binary | std::ofstream::out | std::ofstream::trunc);
	if (file.is_open()) {
		GetState();
		file << processor_stream.getSize();
		file.write(reinterpret_cast<char*>(processor_stream.getData()), processor_stream.getSize());
		file << edit_stream.getSize();
		file.write(reinterpret_cast<char*>(edit_stream.getData()), edit_stream.getSize());
		file.close();
	}
}
} // namespace