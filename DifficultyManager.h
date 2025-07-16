#pragma once
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>

enum class Difficulty {
	Easy,
	Normal,
	Hard
};

class DifficultyManager {
public:
	static DifficultyManager* GetInstance();

	void SetDifficulty(Difficulty difficulty);
	Difficulty GetDifficulty() const;

	// 呼び出し時にCSVファイルをメモリに読み込む
	void PreloadCSVFiles();

	// wave数
	size_t GetWaveCount() const;

	// 特定の波のデータをstringstream形式で返す
	std::istringstream GetWaveStream(int index) const;

private:
	DifficultyManager();

	Difficulty currentDifficulty_ = Difficulty::Easy;
	std::unordered_map<Difficulty, std::vector<std::string>> pathMap_;
	std::vector<std::string> cachedWaveData_;  // CSV 1 wave = 1 string(ライン全体)

	void LoadCSVFilesToMemory(const std::vector<std::string>& paths);
};

