#include <cstring>
#include <Mmreg.h>
#include <iostream>
using namespace std;

class WAVFile {
public:
	char * path;
	FILE * file;
	WAVEFORMATEX format;
	char * data;
	float ** dataFloat;
	int dataSize;

	WAVFile() {
		path = NULL;
	}

	WAVFile(char * path) {
		load(path);
	}

	~WAVFile() {}

	int load(char * path) {
		this->path = path;
		file = fopen(path, "rb");
		if (file) {
			BYTE temp[5];
			temp[4] = '\0';
			DWORD ckSize;
			fread(temp, sizeof(BYTE), 4, file);
			if (!strcmp((const char *)temp, "RIFF")) {
				fread(&ckSize, sizeof(DWORD), 1, file);
				fread(temp, sizeof(BYTE), 4, file);
				if (!strcmp((const char *)temp, "WAVE")) {
					fread(temp, sizeof(BYTE), 4, file);
					fread(&ckSize, sizeof(DWORD), 1, file);
					if (ckSize == 16) {
						fread(&format.wFormatTag, sizeof(WORD), 1, file);
						fread(&format.nChannels, sizeof(WORD), 1, file);
						fread(&format.nSamplesPerSec, sizeof(DWORD), 1, file);
						fread(&format.nAvgBytesPerSec, sizeof(DWORD), 1, file);
						fread(&format.nBlockAlign, sizeof(WORD), 1, file);
						fread(&format.wBitsPerSample, sizeof(WORD), 1, file);
						format.cbSize = 0; // poniewaz pcm

						fread(temp, sizeof(BYTE), 4, file);
						fread(&dataSize, sizeof(DWORD), 1, file);
						data = new char[dataSize];
						fread(data, sizeof(BYTE), dataSize, file);
						
						fclose(file);
						return 0;
					} else {
						fclose(file);
						return -4; // nie pcmowy format
					}
				} else {
					fclose(file);
					return -3; // nie ueiw
				}
			} else {
				fclose(file);
				return -2; // nie plik typu riff
			}
			//return 0;
		} else {
			return -1; // nie otwarto pliku
		}
	}

	void printFormat() {
		printf("wFormatTag: %d\nnChannels: %d\nnSamplesPerSec: %d\nnAvgBytesPerSec: %d\nnBlockAlign: %d\nwBitsPerSample: %d\n", 
		format.wFormatTag, format.nChannels, format.nSamplesPerSec, format.nAvgBytesPerSec, format.nBlockAlign, format.wBitsPerSample);
	}

	float **convert() {
		int newSize = dataSize / 2;
		newSize /= format.nChannels;
		cout << "Float size = " << newSize << endl;
		dataFloat = new float *[format.nChannels];
		for (int i = 0; i < format.nChannels; i++) {
			dataFloat[i] = new float [newSize];
		}
		//dataFloat = (float *)malloc(sizeof(float) * newSize);

		for (int i = 0; i < newSize; i++) {
			/*
			if (i < 10) {
				cout << (unsigned int)data[i + 1] << ' ' << (unsigned char)data[i]*256 << endl;
				cout << (unsigned int)data[i + 3] << ' ' << (unsigned char)data[i + 2]*256 << endl;
			}
			unsigned short s0 = data[i ] + data[i+1]*256;
			unsigned short s1 = data[i +2] + data[i + 3]*256;
			dataFloat[0][i] = s0/65535.0;
			dataFloat[1][i] = s1/65535.0;
			if (i < 10) cout << "in[" << 0 << "][" << i << "] = " << s0 << endl;
			if (i < 10) cout << "in[" << 1 << "][" << i << "] = " << s1 << endl;
			*/
			signed short s = data[4*i] + data[4*i + 1] * 256;
			dataFloat[0][i] = s / 32767.0;
			s = data[4*i + 2] + data[4*i + 3] * 256;
			dataFloat[1][i] = s / 32767.0;
			/*
			for (int j = 0; j < format.nChannels; j++) {
				signed short s = data[2 * (i + j)] + data[1 + 2 * (i + j)] * 256;
				if (i < 10) cout << "in[" << j << "][" << i << "] = " << s << endl;
				dataFloat[j][i] = s / 32767.0;
			}
			*/
			
		}
		//format.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
		return dataFloat;
	}

	float **getFloats() {
		return dataFloat;
	}
	void convertBack(float **out, int ii, int bufferSize) {
		//dataFloat += offset;
		for (int i = 0; i < bufferSize / 2; i++) {
			for (int j = 0; j < format.nChannels; j++) {
				short temp = (short)((out[j][i]) * 32767);
				data[2*bufferSize*ii+4*i + 2*j + 1] = (unsigned short)temp >> 8;
				data[2*bufferSize*ii+4*i + 2*j] = (char)temp;
			}
		}
	}
	void convertBack1(float **floaty, char *chary, int bufferSize) {
		for (int i = 0; i < bufferSize / 2; i++) {
			for (int j = 0; j < format.nChannels; j++) {
				float asdf = floaty[j][i] > 1. ? 1 : floaty[j][i];
				short temp = (short)(asdf * 32767);
				chary[4*i + 2*j + 1] = (unsigned short)temp >> 8;
				chary[4*i + 2*j] = (char)temp;
			}
		}
	}
	void convertBack2(float **floaty, char *chary, int bufferSize) {
		short *test = reinterpret_cast<short *>(chary);
		for (int i = 0; i < bufferSize / 2; i++) {
			for (int j = 0; j < format.nChannels; j++) {
				test[2 * i + j] = 32767 * (floaty[j][i] > 1. ? 1 : floaty[j][i] < -1. ? -1 : floaty[j][i]);
				//chary[4*i + 2*j + 1] = (unsigned short)temp >> 8;
				//chary[4*i + 2*j] = (char)temp;
			}
		}
	}
	void convertBack() {
		for (int i = 0; i < dataSize / (2 * format.nChannels) - 1; i++) {
			short temp = (short)((dataFloat[0][i]) * 32767);
			data[4*i] = (char)temp;
			data[4*i + 1] = (unsigned short)temp >> 8;
			temp = (short)((dataFloat[1][i]) * 32767);
			data[4*i + 2] = (char)temp;
			data[4*i + 3] = (unsigned short)temp >> 8;
			/*
			for (int j = 0; j < format.nChannels; j++) {
				short temp = (short)((dataFloat[j][i]) * 32767);
				data[4*i + 2*j + 1] = (unsigned short)temp >> 8;//2 * (i + j) + 1
				data[4*i + 2*j] = (char)temp;//
			}
			*/
		}
		for (int i = 0; i < 10; i++) cout << (int)data[i] << ' ';
	}
};