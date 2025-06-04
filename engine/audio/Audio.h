#pragma once
#include <fstream>
#include <wrl.h>
#include <xaudio2.h>
#include <memory>

#include <unordered_map>

#pragma comment(lib,"xaudio2.lib")

#include <AudioManager.h>

class Audio {


public:

private:

public:

	void Initialize();

	void Finalize();
	
	// 音声再生

	void SoundPlay(const char* filename,int loopCount);

	void SoundStop(const char* filename);

	void SetVolume(float volume);

private:

	Microsoft::WRL::ComPtr<IXAudio2> xAudio2;

	IXAudio2MasteringVoice* masterVoice;

	IXAudio2SourceVoice* pSourceVoice = nullptr;

};