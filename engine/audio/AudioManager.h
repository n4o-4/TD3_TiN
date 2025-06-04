#pragma once
#include <string>
#include <fstream>
#include <wrl.h>
#include <memory>
#include <unordered_map>
#include <xaudio2.h>

#pragma comment(lib,"xaudio2.lib")

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

#pragma comment(lib, "Mf.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "Mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")

class AudioManager
{
private:
	// チャンクヘッダ
	struct ChunkHeader
	{
		char id[4];   // チャンク毎のID
		int32_t size; // チャンクサイズ
	};

	// RIFFヘッダチャンク
	struct RiffHeader
	{
		ChunkHeader chunk; // "RIFF"
		char type[4];      // WAVE"
	};

	// FMTチャンク
	struct FormatChunk
	{
		ChunkHeader chunk; // "fmt"
		WAVEFORMATEX fmt;  // 波系フォーマット
	};

	// 音声データ
	struct SoundData
	{
		// 波系フォーマット
		WAVEFORMATEX wfex;

		// バッファの先頭アドレス
		BYTE* pBuffer;

		// バッファのサイズ
		unsigned int bufferSize;
	};

public:

	// シングルトンインスタンスの取得
	static AudioManager* GetInstance();

	void Initialize();

	void Finalize();

	void SoundLoadFile(const char* filename);

	void SoundUnload();

	std::unordered_map<std::string, SoundData> GetSoundData() { return soundDatas; }

private:

	void SoundLoadWave(const char* filename);

	void SoundLoadMP3(const char* filename);



private:


	static std::unique_ptr<AudioManager> instance;

	friend std::unique_ptr<AudioManager> std::make_unique<AudioManager>();
	friend std::default_delete<AudioManager>;

	AudioManager() = default;
	~AudioManager() = default;
	AudioManager(AudioManager&) = delete;
	AudioManager& operator=(AudioManager&) = delete;

	SoundData soundData{};

	std::unordered_map<std::string, SoundData> soundDatas;

};