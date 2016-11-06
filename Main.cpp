#include "HostHeader.h"
#include "wavplayer.h"

int main() {
	Host host(123, 44100.0);

	///////////////////////////	odtwarzanie ///////////////////////////////
	WAVFile *wave = new WAVFile("./../feed/feed.wav");
	wave->printFormat();
	wave->convert();
	int bf = 44100 / 8;	////////////////////// buffer size
	int limit = wave->dataSize / (2 * bf);
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
	waveOutOpen(&hWaveOut, WAVE_MAPPER, &wave->format, 0, 0, CALLBACK_NULL);
	while (true) {
		host.Process(wave->dataFloat, out_float[0]);
		wave->convertBack1(out_float[0], out_char[0], bf);
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
			int asdf = j * bf / 2;
			in[i][0] = wave->dataFloat[0] + asdf;
			in[i][1] = wave->dataFloat[1] + asdf;
			host.Process(in[i], out_float[i]);
			wave->convertBack1(out_float[i], out_char[i], bf);
			header[i].lpData = out_char[i];
			waveOutPrepareHeader(hWaveOut, (wavehdr_tag*)&header[i], sizeof(WAVEHDR));
			waveOutWrite(hWaveOut, (wavehdr_tag*)&header[i], sizeof(WAVEHDR));
			do {} while (!(header[!i].dwFlags & WHDR_DONE));
			waveOutUnprepareHeader(hWaveOut, &header[!i], sizeof(WAVEHDR));
		}
		do {} while (!(header[i].dwFlags & WHDR_DONE));
		waveOutUnprepareHeader(hWaveOut, &header[i], sizeof(WAVEHDR));
	}
	////////////////////////////////////////////////////////////////////

	std::cin.get();
}