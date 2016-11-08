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
				fmt.cbSize = 0; // read two bytes too much
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
		std::cout << "WAVEFORMATEX:" << std::endl
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
	WAVEFORMATEX fmt;
	char* data;
	unsigned size;
} wave;

HWAVEOUT out;
WAVEHDR hdr[2];
char *out_char[2];

void init_player(int block_size) {
	for (unsigned i = 0; i < 2; ++i) {
		out_char[i] = new char[block_size * 2]; // ka¿da próbka ma 2 bajty
		//hdr[i].dwBufferLength = block_size * 2; 
		//hdr[i].dwFlags = 0;
		//hdr[i].dwLoops = 0;
	}
	//waveOutOpen(&out, WAVE_MAPPER, &wave.fmt, 0, 0, CALLBACK_NULL);
}


int main() {
	wave.Load("./../feed/feed.wav");
	int block_size = 52;// 5504;
	int sample_rate = wave.GetSampleRate();
	Host host(block_size, sample_rate);
	///////////////////////////	odtwarzanie ///////////////////////////////
	wave.Print();
	//wave->convert();
	//int bf = 44100 / 8;	////////////////////// buffer size
	//bf += 16 - (bf % 16);
	int bf = 5504;
	int limit = wave.size / (2 * bf);
	//init_player(block_size);
	char *out_char[2];
	float **in[2], **out_float[2];
	for (int i = 0; i < 2; i++) {
		in[i] = new float *[2];
		out_char[i] = new char[bf * 2];
		out_float[i] = new float *[2];
		for (int j = 0; j < 2; j++) out_float[i][j] = new float[bf / 2];
	}
	HWAVEOUT hWaveOut;
	host.SetBlockSize(bf / 2);
	waveOutOpen(&hWaveOut, WAVE_MAPPER, &wave.fmt, 0, 0, CALLBACK_NULL);
	while (true) {
		//host.Process(wave->dataFloat, out_float[0]);
		//wave->convertBack1(out_float[0], out_char[0], bf);
		host.Process(wave.data, out_char[0]);
		WAVEHDR header[2];
		header[0].dwBufferLength = bf * 2;
		header[0].dwFlags = 0;
		header[0].dwLoops = 0;
		header[0].lpData = out_char[0];
		header[1].dwBufferLength = bf * 2;
		header[1].dwFlags = 0;
		header[1].dwLoops = 0;
		int i = 0, j = 0;
		waveOutPrepareHeader(hWaveOut, (wavehdr_tag*)&header[0], sizeof(WAVEHDR));
		waveOutWrite(hWaveOut, (wavehdr_tag*)&header[0], sizeof(WAVEHDR));
		while (j++ < limit - 2) {
			//int old_i = i;	// !i
			i = (i + 1) % 2;
			//int asdf = j * bf / 2;
			//in[i][0] = wave->dataFloat[0] + asdf;
			//in[i][1] = wave->dataFloat[1] + asdf;
			//host.Process(in[i], out_float[i]);
			//wave->convertBack1(out_float[i], out_char[i], bf);
			int asdf = j * bf * 2;
			host.Process(wave.data + asdf, out_char[i]);

			header[i].lpData = out_char[i];
			waveOutPrepareHeader(hWaveOut, (wavehdr_tag*)&header[i], sizeof(WAVEHDR));
			waveOutWrite(hWaveOut, (wavehdr_tag*)&header[i], sizeof(WAVEHDR));
			do { Sleep(10); } while (!(header[!i].dwFlags & WHDR_DONE));
			waveOutUnprepareHeader(hWaveOut, &header[!i], sizeof(WAVEHDR));
		}
		do { Sleep(10); } while (!(header[i].dwFlags & WHDR_DONE));
		waveOutUnprepareHeader(hWaveOut, &header[i], sizeof(WAVEHDR));
	}
	////////////////////////////////////////////////////////////////////

	std::cin.get();
}