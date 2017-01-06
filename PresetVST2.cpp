#include "PresetVST2.h"

#include <fstream>

#include "PluginVST2.h"

namespace VSTHost {
const size_t PresetVST2::kProgramUnionSize = 8; // in bytes
const std::string PresetVST2::kExtension{ "fxp" };

PresetVST2::PresetVST2(PluginVST2& p) : plugin(p), program(nullptr), fxprogram_size(0) {
	// is program data handled in formatless chunks
	program_chunks = 0 != (plugin.GetFlags() & VstAEffectFlags::effFlagsProgramChunks);
	// set preset path
	preset_file_path = Plugin::kPluginDirectory + plugin.GetPluginFileName();
	std::string::size_type pos = 0;
	if ((pos = preset_file_path.find_last_of('.')) != std::string::npos)
		preset_file_path = preset_file_path.substr(0, pos);
	preset_file_path += "." + kExtension;
	// fxProgram struct
	if (ProgramChunks()) {
		char* chunk = nullptr;
		auto chunk_size = plugin.Dispatcher(AEffectOpcodes::effGetChunk, 1, 0, &chunk); // plugin allocates the data
		fxprogram_size = sizeof(fxProgram) - kProgramUnionSize + sizeof(VstInt32) + chunk_size;
		program = reinterpret_cast<fxProgram*>(new char[fxprogram_size]{});
		program->content.data.size = chunk_size;
		if (chunk)
			free(chunk);
	}	// size of the union is 8 bytes
	else {
		fxprogram_size = sizeof(fxProgram) - kProgramUnionSize + plugin.GetParameterCount() * sizeof(float);
		program = reinterpret_cast<fxProgram*>(new char[fxprogram_size]{});
	}
	program->version = 1;
	program->fxID = plugin.plugin->uniqueID;
	program->fxVersion = plugin.GetVSTVersion();
	program->numParams = plugin.GetParameterCount();
	program->fxMagic = ProgramChunks() ? chunkPresetMagic : fMagic;
	program->byteSize = fxprogram_size - sizeof(program->byteSize) - sizeof(program->chunkMagic);
	program->chunkMagic = cMagic;
	strcpy(program->prgName, "VSTHost Preset");
	GetState();
}

PresetVST2::~PresetVST2() {
	if (program)
		delete[] reinterpret_cast<char*>(program);
}

void PresetVST2::SetState() {
	if (ProgramChunks())
		plugin.Dispatcher(AEffectOpcodes::effSetChunk, 1, program->content.data.size, &program->content.data.chunk);
	else
		for (Steinberg::int32 i = 0; i < plugin.GetParameterCount(); i++)
			plugin.SetParameter(i, program->content.params[i]);
}

void PresetVST2::LoadFromFile() {
	std::ifstream file(preset_file_path, std::ifstream::binary | std::ifstream::in);
	if (file.is_open()) {
		fxProgram in{};
		auto const head_size = sizeof(in.chunkMagic) + sizeof(in.byteSize);
		file.read(reinterpret_cast<char*>(&in), head_size);
		if (SwapNeeded()) {
			SWAP_32(in.chunkMagic);
			SWAP_32(in.byteSize);
		}
		if (file.good() && in.chunkMagic == cMagic && in.byteSize == program->byteSize) {
			file.read(reinterpret_cast<char*>(&in) + head_size, sizeof(fxProgram) - head_size - sizeof(in.content));
			if (SwapNeeded()) {
				SWAP_32(in.fxID);
				SWAP_32(in.fxMagic);
				SWAP_32(in.fxVersion);
				SWAP_32(in.numParams);
				SWAP_32(in.version);
			}
			if (file.good() && in.fxID == program->fxID /*&& in.fxVersion == program->fxVersion*/
				&& in.numParams == program->numParams && in.version == program->version) {
				if (in.fxMagic == chunkPresetMagic) {
					std::unique_ptr<char[]> in_chunk(new char[program->content.data.size]);
					Steinberg::int32 in_chunk_size;
					file.read(reinterpret_cast<char*>(&in_chunk_size), sizeof(in_chunk_size));
					if (file.good() && in_chunk_size == program->content.data.size) {
						file.read(in_chunk.get(), program->content.data.size);
						if (file.eof()) { // preset file is valid, can be loaded
							memcpy(program->content.data.chunk, in_chunk.get(), program->content.data.size);
							memcpy(program->prgName, in.prgName, sizeof(program->prgName));
							SetState();
						}
					}
				}
				else if (in.fxMagic == fMagic) {
					std::unique_ptr<float[]> params(new float[program->numParams]);
					Steinberg::int32 i = 0;
					while (i < program->numParams && file.good()) {
						file.read(reinterpret_cast<char*>(params.get() + i), sizeof(params[0]));
						if (SwapNeeded())
							SWAP_32(params[i]);
						++i;
					}
					file.get();
					if (file.eof() && i == program->numParams) { // preset is valid, params can be loaded
						for (i = 0; i < program->numParams; ++i)
							program->content.params[i] = params[i];
						SetState();
					}
				}
			}
		}
		file.close();
	}
}

void PresetVST2::GetState() {
	if (ProgramChunks()) {
		char* chunk = nullptr;
		plugin.Dispatcher(AEffectOpcodes::effGetChunk, 1, 0, &chunk);
		memcpy(program->content.data.chunk, chunk, program->content.data.size);
		if (chunk)
			free(chunk);
	}
	else
		for (Steinberg::int32 i = 0; i < plugin.GetParameterCount(); i++)
			program->content.params[i] = plugin.GetParameter(i);
}

void PresetVST2::SaveToFile() {
	GetState();
	std::ofstream file(preset_file_path, std::ofstream::binary | std::ofstream::out | std::ofstream::trunc);
	if (file.is_open()) {
		if (SwapNeeded())
			SwapProgram();
		file.write(reinterpret_cast<char*>(program), fxprogram_size);
		if (SwapNeeded())
			SwapProgram();
		file.close();
	}
}

void PresetVST2::SwapProgram() {
	SWAP_32(program->chunkMagic);
	SWAP_32(program->byteSize);
	SWAP_32(program->fxMagic);
	SWAP_32(program->version);
	SWAP_32(program->fxID);
	SWAP_32(program->fxVersion);
	SWAP_32(program->numParams);
	if (ProgramChunks()) {
		SWAP_32(program->content.data.size);
	}
	else {
		for (Steinberg::int32 i = 0; i < plugin.GetParameterCount(); ++i) {
			SWAP_32(program->content.params[i]);
		}
	}
}

bool PresetVST2::ProgramChunks() {
	return program_chunks;
}

#ifndef __cpp_constexpr
#define constexpr
#endif

constexpr bool PresetVST2::SwapNeeded() {
	static constexpr Steinberg::int32 magic = cMagic;
	static constexpr char str1[] = "CcnK";
	static constexpr char str2[] = { magic, magic >> 8, magic >> 16, magic >> 24 };
	static constexpr bool swap_needed = str1[0] == str2[3] && str1[1] == str2[2]
										&& str1[2] == str2[1] && str1[3] == str2[0];
	return swap_needed;
}

} // namespace