
#ifndef PRESET_H
#define PRESET_H

class Preset {
public:
	Preset() {}
	virtual ~Preset() {}
	virtual bool Load() = 0;
	virtual void LoadFromFile() = 0;
	virtual void Save() = 0;
	virtual void SaveToFile() = 0;
};

#endif