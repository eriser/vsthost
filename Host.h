#ifndef HOST_H
#define HOST_H

#include <string>
#include <memory>

#define NOMINMAX // kolizja makra MAX z windows.h oraz std::numeric_limits<T>::max()

namespace VSTHost {
class Host {
public:
	Host(std::int64_t block_size, double sample_rate);
	~Host();
	void Process(float** input, float** output);
	void Process(char* input, char* output);
	void Process(std::int8_t* input, std::int8_t* output);
	void Process(std::int16_t* input, std::int16_t* output);
	void SetSampleRate(double sr);
	void SetBlockSize(std::int64_t bs);
	void CreateGUIThread();
	void CreateGUI();
	bool LoadPluginList(const std::string& path = kPluginList);
	bool SavePluginList(const std::string& path = kPluginList);
	const std::string& GetDefaultPluginListPath();
private:
	class HostImpl;
	const static std::string kPluginList;
	std::unique_ptr<HostImpl> impl;
};
} // namespace

#endif