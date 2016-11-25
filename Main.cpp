#include "HostHeader.h"

#include <fstream>
#include <string>

#include <MMSystem.h>

class WAVFile {
public:
	//explicit WAVFile(std::string path) : data(nullptr), size(0) { Load(path); }
	WAVFile() : data(nullptr), size(0) {}
	~WAVFile() { 
		if (data) 
			delete[] data; 
	}
	void Load(std::string path) {
		std::ifstream file(path, std::ifstream::binary | std::ifstream::in);
		if (file.is_open()) {
			char tmp[4 * 5] = { 0 };
			file.read(tmp, 4 * 5);
			DWORD cbSize = *reinterpret_cast<DWORD*>(tmp + 4 * 4);
			if (!std::string(tmp, 4u).compare("RIFF") && !std::string(tmp + 8, 4u).compare("WAVE") && !std::string(tmp + 12, 4u).compare("fmt ") && cbSize == 16 && file.good()) {
				file.read(reinterpret_cast<char *>(&fmt), sizeof(fmt));
				fmt.cbSize = 0; // read two bytes too many
				file.unget(); 
				file.unget();
				if (file.good())
					file.read(tmp, 4 * 2);
				if (file.good() && !std::string(tmp, 4u).compare("data")) {
					size = *reinterpret_cast<unsigned*>(tmp + 4);
					if (data)
						delete data;
					data = new char[size];
					file.read(data, size);
				}
				else
					std::cout << "Error reading data" << std::endl;
			}
			else
				std::cout << "Unsupported file format." << std::endl;
		}
		else
			std::cout << "Could not open file." << std::endl;
	}
	void Print() {
		std::cout << "Samples: " << size / (GetChannelCount() * GetBitDepth() / 8) << std::endl
			<< "Bytes: " << size << std::endl
			<< "WAVEFORMATEX:" << std::endl
			<< "\twFormatTag: " << fmt.wFormatTag << std::endl
			<< "\tnChannels: " << fmt.nChannels << std::endl
			<< "\tnSamplesPerSec: " << fmt.nSamplesPerSec << std::endl
			<< "\tnAvgBytesPerSec: " << fmt.nAvgBytesPerSec << std::endl
			<< "\tnBlockAlign: " << fmt.nBlockAlign << std::endl
			<< "\twBitsPerSample: " << fmt.wBitsPerSample << std::endl
			<< "\tcbSize: " << fmt.cbSize << std::endl;
	}
	double GetSampleRate() { 
		return data ? static_cast<double>(fmt.nSamplesPerSec) : 0.0;
	}
	int GetChannelCount() {
		return fmt.nChannels;
	}
	int GetBitDepth() {
		return fmt.wBitsPerSample;
	}
	bool IsStereo() {
		return fmt.nChannels == 2;
	}
	WAVEFORMATEX fmt;
	char* data;
	unsigned size;
} wave;

HWAVEOUT out;
WAVEHDR hdr[2];
char *out_char[2];

void init_player(int block_size) {
	for (unsigned i = 0; i < 2; ++i) {
		out_char[i] = new char[block_size];
		hdr[i].dwBufferLength = block_size; 
		hdr[i].dwFlags = 0;
		hdr[i].dwLoops = 0;
	}
	waveOutOpen(&out, WAVE_MAPPER, &wave.fmt, 0, 0, CALLBACK_NULL);
}

void write_header(int i) {
	hdr[i].lpData = out_char[i];
	waveOutPrepareHeader(out, (wavehdr_tag*)&hdr[i], sizeof(WAVEHDR));
	waveOutWrite(out, (wavehdr_tag*)&hdr[i], sizeof(WAVEHDR));
}

void unprepare_header(int i) {
	do { Sleep(11); } while (!(hdr[i].dwFlags & WHDR_DONE));
	waveOutUnprepareHeader(out, &hdr[i], sizeof(WAVEHDR));
}


int main() {
	wave.Load("./../feed/feed0.wav");
	wave.Print();

	int block_size = 4096;	
	int bs_all_channels = block_size * wave.GetChannelCount();
	int bs_all_channels_bytes = bs_all_channels * (wave.GetBitDepth() / 8);
	double sample_rate = static_cast<double>(wave.GetSampleRate());
	bool stereo = wave.IsStereo();
	Host host(block_size, sample_rate, stereo);
	host.test();

	// this player is bad, because for small block_sizes it just doesn't work well
	init_player(bs_all_channels_bytes);
	unsigned i = 0, pos = 0;

	host.Process(wave.data, out_char[0]);
	write_header(0);
	while (true) {
		i = (i + 1) % 2;
		if (wave.size < pos + 2 * bs_all_channels_bytes)
			pos = 0;	// ignoring (wave.size - pos - bs_all_channels_bytes) bytes
		else
			pos += bs_all_channels_bytes;
		host.Process(wave.data + pos, out_char[i]);
		write_header(i);
		unprepare_header(!i);
	}
	unprepare_header(i);

	std::cin.get();
}