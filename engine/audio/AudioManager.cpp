#include "AudioManager.h"

#include "StringUtility.h"

#include "assert.h"


std::unique_ptr<AudioManager> AudioManager::instance = nullptr;

AudioManager* AudioManager::GetInstance()
{

	if (instance == nullptr) {
		instance = std::make_unique<AudioManager>();
	}

	return instance.get();
}

void AudioManager::Initialize()
{

	CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);

}

void AudioManager::Finalize()
{
	instance.reset();
}

void AudioManager::SoundLoadFile(const char* filename)
{
	// filenameをstringに変換
	std::string fileStr = filename;

	if (fileStr.size() >= 4 && fileStr.substr(fileStr.size() - 4) == ".wav")
	{
		SoundLoadWave(filename);
	}
	else if (fileStr.size() >= 4 && fileStr.substr(fileStr.size() - 4) == ".mp3")
	{
		SoundLoadMP3(filename);
	}
}

void AudioManager::SoundUnload()
{

	for (auto& [key, soundData] : soundDatas)
	{
		if (soundData.pBuffer)
		{
			delete[] soundData.pBuffer;
			soundData.pBuffer = nullptr;
			soundData.bufferSize = 0;
			soundData.wfex = {};
		}
	}
	// 必要であればマップをクリア
	soundDatas.clear();

}

void AudioManager::SoundLoadWave(const char* filename)
{
	if (soundDatas.find(filename) != soundDatas.end())
	{
		return;
	}

	HRESULT result;

	/*-----------------------------
	* ファイルをオープン
	-----------------------------*/

	// ファイル入力ストリームのインスタンス
	std::ifstream file;

	// .wavファイルをバイナリモードで開く
	file.open(filename, std::ios_base::binary);

	// ファイルオープン失敗を検出する
	assert(file.is_open());

	/*-----------------------------
	* .wavデータの読み込み
	-----------------------------*/

	// RIFFヘッダーの読み込み
	RiffHeader riff;
	file.read((char*)&riff, sizeof(riff));

	// ファイルがRIFFかチェック
	if (strncmp(riff.chunk.id, "RIFF", 4) != 0)
	{
		assert(0);
	}

	// タイプがWAVEかチェック
	if (strncmp(riff.type, "WAVE", 4) != 0)
	{
		assert(0);
	}

	// Formatチャンクの読み込み
	FormatChunk format = {};

	// チャンクヘッダーの確認
	file.read((char*)&format, sizeof(ChunkHeader));
	if (strncmp(format.chunk.id, "fmt ", 4) != 0)
	{
		assert(0);
	}

	// チャンク本体の読み込み
	assert(format.chunk.size <= sizeof(format.fmt));
	file.read((char*)&format.fmt, format.chunk.size);

	// Dataチャンクの読み込み
	ChunkHeader data;

	//// JUNKチャンクを検出した場合
	while (true) {
		// チャンクヘッダーを読み込む
		file.read((char*)&data, sizeof(data));

		// 読み込みエラーが発生した場合、ファイルを閉じて終了
		if (file.eof() || !file) {
			assert(0);  // データチャンクが見つからなかった
			break;
		}

		// Dataチャンクを検出した場合
		if (strncmp(data.id, "data", 4) == 0) {
			break;  // Dataチャンクが見つかったのでループを抜ける
		}

		// JUNKチャンクやその他のチャンクの場合、読み飛ばす
		file.seekg(data.size, std::ios_base::cur);
	}

	// Dataチャンクのデータ部(波系データ)の読み
	char* pBuffer = new char[data.size];
	file.read(pBuffer, data.size);

	// waveファイルを閉じる
	file.close();

	/*-----------------------------
	* 読み込んだ音声データを返す
	-----------------------------*/

	// returnする為の音声データ

	soundData.wfex = format.fmt;
	soundData.pBuffer = reinterpret_cast<BYTE*>(pBuffer);
	soundData.bufferSize = data.size;

	soundDatas[filename] = soundData;
}

void AudioManager::SoundLoadMP3(const char* filename)
{
	std::string fileStr = filename;

	std::wstring wFileStr = StringUtility::ConvertString(fileStr.c_str());

	// ソースリーダーの生成
	IMFSourceReader* pSourceReader = nullptr;
	MFCreateSourceReaderFromURL(wFileStr.c_str(), NULL, &pSourceReader);

	IMFMediaType* pMediaType = nullptr;
	MFCreateMediaType(&pMediaType);
	pMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
	pMediaType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
	pSourceReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, pMediaType);

	pMediaType->Release();
	pMediaType = nullptr;
	pSourceReader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, &pMediaType);

	// メディアタイプからWaveFormatexを生成
	WAVEFORMATEX* tempWfex = nullptr;
	MFCreateWaveFormatExFromMFMediaType(pMediaType, &tempWfex, nullptr);

	soundData.wfex = *tempWfex;
	std::vector<BYTE> mediaData; // 一時的なバッファ

	while (true)
	{
		IMFSample* pMFSample{ nullptr };
		DWORD dwStreamFlags{ 0 };
		HRESULT hr = pSourceReader->ReadSample(
			MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, nullptr, &dwStreamFlags, nullptr, &pMFSample);

		// ストリーム終了チェック
		if (dwStreamFlags & MF_SOURCE_READERF_ENDOFSTREAM)
		{
			break;
		}

		IMFMediaBuffer* pMFMediaBuffer{ nullptr };
		if (pMFSample)
		{
			hr = pMFSample->ConvertToContiguousBuffer(&pMFMediaBuffer);
			if (FAILED(hr))
			{
				pMFSample->Release();
			}
		}

		BYTE* pBuffer{ nullptr };
		DWORD cbCurrentLength{ 0 };
		hr = pMFMediaBuffer->Lock(&pBuffer, nullptr, &cbCurrentLength);
		if (FAILED(hr))
		{
			pMFMediaBuffer->Release();
			pMFSample->Release();
		}

		// データを追加
		mediaData.resize(mediaData.size() + cbCurrentLength);
		memcpy(mediaData.data() + mediaData.size() - cbCurrentLength, pBuffer, cbCurrentLength);

		// アンロックと解放
		pMFMediaBuffer->Unlock();
		pMFMediaBuffer->Release();
		pMFSample->Release();
	}

	// 結果をSoundDataに格納
	soundData.bufferSize = static_cast<unsigned int>(mediaData.size());
	soundData.pBuffer = new BYTE[soundData.bufferSize];
	memcpy(soundData.pBuffer, mediaData.data(), soundData.bufferSize);

	soundDatas[filename] = soundData;
}
