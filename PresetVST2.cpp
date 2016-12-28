#include "PresetVST2.h"

#include "PluginVST2.h"

namespace VSTHost {
PresetVST2::PresetVST2(PluginVST2& p) : plugin(p), isSaved(false), chunk(NULL) {
	path[0] = '.';
	path[1] = '\\';
	plugin.Dispatcher(AEffectXOpcodes::effGetEffectName, 0, 0, (void *)(path + 2));
	AddExtension();

	info.version = 1;
	info.pluginUniqueID = plugin.plugin->uniqueID;
	info.pluginVersion = plugin.GetVSTVersion();
	info.numElements = plugin.GetParameterCount();
	if (ProgramChunks()) {
		size = sizeof(fxProgram);
		program = (fxProgram *)malloc(size);
		program->fxMagic = chunkPresetMagic;
	}
	else {
		size = sizeof(fxProgram)+(plugin.GetParameterCount() - 2) * sizeof(float); // 0 element juz jest wliczony w strukture
		program = (fxProgram *)malloc(size);	// union ma size 8 wiec params[1] tez wliczony.  ? wiec -2
		program->fxMagic = fMagic;
	}
	program->chunkMagic = cMagic;
	program->byteSize = -1;
	program->version = info.version;
	program->fxID = info.pluginUniqueID;
	program->fxVersion = info.pluginVersion;
	program->numParams = info.numElements;
	strcpy(program->prgName, "Custom");
}

PresetVST2::~PresetVST2() {
	if (ProgramChunks() && chunk) free(chunk);
	if (program) free(program);
}

bool PresetVST2::SetState() {
	if (isSaved) {	// wczytuje wszystkie parametry w zaleznosci czy w postaci chunk czy tablicy
		if (ProgramChunks()) {
			plugin.Dispatcher(effSetChunk, 1, chunkSize, &chunk);
		}
		else {
			for (int i = 0; i < plugin.GetParameterCount(); i++){
				plugin.SetParameter(i, program->content.params[i]);
			}
		}
		return true;
	}
	else {	// jezeli nie ma stanu zaladowanego jeszcze, to sprawdzam czy plik jest legit
		FILE *file = fopen(path, "r");
		if (file) {
			VstInt32 test;
			fread(&test, sizeof(VstInt32), 1, file);
			fclose(file);
			if (test == cMagic) {
				LoadFromFile();
				return true;
			}
			else return false;
		}
		else return false;
	}
}

void PresetVST2::LoadFromFile() {
	FILE *file = fopen(path, "r");
	fread(program, size, 1, file);
	if (ProgramChunks()) {
		chunkSize = program->content.data.size;
		chunk = (char *)malloc(chunkSize * sizeof(char));
		chunk[0] = program->content.data.chunk[0];
		fread(chunk + 1, chunkSize - 1, 1, file);
	}
	fclose(file);
	isSaved = true;
	SetState();
}

void PresetVST2::GetState() {
	if (ProgramChunks()) {
		if (chunk) free(chunk);
		chunkSize = plugin.Dispatcher(effGetChunk, 1, 0, &chunk);
		program->content.data.size = chunkSize;
		// program->content.data.chunk = chunk;	// todo: zapisac do struktury tablice a nie 1 element tylko
		program->content.data.chunk[0] = chunk[0];
		//chunk++;	// chunkmagic i byteSize nie liczac ponizej
		program->byteSize = sizeof(fxProgram) - 2 * sizeof(VstInt32) + chunkSize;
	}
	else {
		chunkSize = (plugin.GetParameterCount() - 1) * sizeof(float);
		program->byteSize += program->byteSize = sizeof(fxProgram) - 2 * sizeof(VstInt32) + chunkSize;
		for (int i = 0; i < plugin.GetParameterCount(); i++) {
			program->content.params[i] = plugin.GetParameter(i);
		}
	}
	isSaved = true;
	SaveToFile();
}

void PresetVST2::SaveToFile() {
	FILE *file;
	file = fopen(path, "w");
	fwrite(program, size, 1, file);
	if (ProgramChunks()) {
		fwrite(chunk + 1, chunkSize - 1, 1, file);
	}
	fclose(file);
}

void PresetVST2::AddExtension() {
	int pos = 0;
	while (path[pos] != '\0') pos++;
	path[pos] = '.';
	path[pos + 1] = 'f';
	path[pos + 2] = 'x';
	path[pos + 3] = 'p';
	path[pos + 4] = '\0';
}

bool PresetVST2::ProgramChunks() {
	return 0 != (plugin.GetFlags() & VstAEffectFlags::effFlagsProgramChunks);
}
} // namespace