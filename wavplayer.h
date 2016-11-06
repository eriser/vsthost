#include <MMSystem.h>

#include "wavfile.h"

class WAVPlayer {
public:
	WAVFile * wave;
	volatile WAVEHDR wh;
	HWAVEOUT hWaveOut;
	WAVPlayer(char * path) {
		wave = new WAVFile(path);
		
		wave->convert();
		//wave->convertBack();
		//wh.lpData = (LPSTR)wave->dataFloat;
		
		wh.lpData = wave->data;
		wh.dwBufferLength = 44100 / 4;//wave->dataSize;
		wh.dwFlags = 0;
		wh.dwLoops = 0;
		waveOutOpen(&hWaveOut, WAVE_MAPPER, &wave->format, 0, 0, CALLBACK_NULL);
		waveOutPrepareHeader(hWaveOut, (wavehdr_tag*)&wh, sizeof(wh));
	}
	~WAVPlayer() {
		delete wave;
	}

	void write(char *bufor, int sampli) {
		//waveOutWrite(hWaveOut, bufor, sampli);
	}
	void play(int i, int bf) {
		WAVEHDR asdf;
			asdf.dwBufferLength = bf * 2;
			asdf.dwFlags = 0;
			asdf.dwLoops = 0;
			asdf.lpData = wave->data + i * bf * 2;
			cout << "how many times: " << i << endl;
			waveOutPrepareHeader(hWaveOut, (wavehdr_tag*)&asdf, sizeof(asdf));
			waveOutWrite(hWaveOut, (wavehdr_tag*)&asdf, sizeof(asdf));
			Sleep(100);
			do {} while (!(asdf.dwFlags & WHDR_DONE));	//Sleep(10);
			//waveOutUnprepareHeader(hWaveOut,&asdf ,sizeof(asdf));
	}
	void play() {
			WAVEHDR asdf;
			asdf.dwBufferLength = wave->dataSize;
			asdf.dwFlags = 0;
			asdf.dwLoops = 0;
			asdf.lpData = wave->data;
			waveOutPrepareHeader(hWaveOut, (wavehdr_tag*)&asdf, sizeof(asdf));
			waveOutWrite(hWaveOut, (wavehdr_tag*)&asdf, sizeof(asdf));
			Sleep(8000);
			
			do {} while (!(asdf.dwFlags & WHDR_DONE));
	}
	void play1(int bf, int size) {
		char *ptr = wave->data, *stop = wave->data + size;
		WAVEHDR header[2];
		header[0].dwBufferLength = bf * 2;
		header[0].dwFlags = 0;
		header[0].dwLoops = 0;
		header[0].lpData = ptr;
		header[1].dwBufferLength = bf * 2;
		header[1].dwFlags = 0;
		header[1].dwLoops = 0;
		//ptr += bf * 2;
		//header[1].lpData = ptr;
		int i = 0;
		waveOutPrepareHeader(hWaveOut, (wavehdr_tag*)&header[0], sizeof(WAVEHDR));
		//waveOutPrepareHeader(hWaveOut, (wavehdr_tag*)&header[1], sizeof(WAVEHDR));
		waveOutWrite(hWaveOut, (wavehdr_tag*)&header[i], sizeof(WAVEHDR));
		
		while (ptr < stop) {
			int old_i = i;
			i = (i + 1) % 2;
			ptr += bf * 2;
			header[i].lpData = ptr;
			waveOutPrepareHeader(hWaveOut, (wavehdr_tag*)&header[i], sizeof(WAVEHDR));
			//Sleep(150);
			waveOutWrite(hWaveOut, (wavehdr_tag*)&header[i], sizeof(WAVEHDR));
			do {} while (!(header[old_i].dwFlags & WHDR_DONE));//Sleep(10);
			//waveOutWrite(hWaveOut, (wavehdr_tag*)&header[i], sizeof(WAVEHDR));
			waveOutUnprepareHeader(hWaveOut,&header[old_i] ,sizeof(WAVEHDR));
		}
	}
	void unplay() {
		waveOutUnprepareHeader(hWaveOut,(wavehdr_tag*)&wh,sizeof(wh));
		waveOutClose(hWaveOut);
	}

};