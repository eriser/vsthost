#ifndef HOST_H
#define HOST_H

#include <string>
#include <memory>
#include <cstdint>

namespace VSTHost {
class Host {
public:
	Host(std::int64_t max_num_samples, double sample_rate); // max_num_samples is maximum number of samples per channel
	~Host();
	void Process(float** input, float** output, std::int64_t num_samples); // num_samples - samples per channel
	void Process(char* input, char* output, std::int64_t num_samples);
	void Process(std::int16_t* input, std::int16_t* output, std::int64_t num_samples);
	void SetSampleRate(double sr);
	void SetBlockSize(std::int64_t bs);
	void CreateGUIThread();
	void CreateGUI();
	bool LoadPluginList(const std::string& path);
	bool SavePluginList(const std::string& path);
	bool LoadPluginList();
	bool SavePluginList();
private:
	class HostImpl;
	std::unique_ptr<HostImpl> impl;
};
} // namespace

#endif