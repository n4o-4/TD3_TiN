#include "DifficultyManager.h"
#include <fstream>
#include <cassert>

DifficultyManager* DifficultyManager::GetInstance() {
	static DifficultyManager instance;
	return &instance;
}

DifficultyManager::DifficultyManager() {
	pathMap_[Difficulty::Easy] = {
		"./Resources/enemySpawn1.csv",
		"./Resources/enemySpawn2.csv",
		"./Resources/enemySpawn3.csv"
	};
	pathMap_[Difficulty::Normal] = {
		"./Resources/enemySpawn4.csv",
		"./Resources/enemySpawn5.csv",
		"./Resources/enemySpawn6.csv"
	};
	pathMap_[Difficulty::Hard] = {
		"./Resources/enemySpawn7.csv",
		"./Resources/enemySpawn8.csv",
		"./Resources/enemySpawn9.csv"
	};
}

void DifficultyManager::SetDifficulty(Difficulty difficulty) {
	currentDifficulty_ = difficulty;
}

Difficulty DifficultyManager::GetDifficulty() const {
	return currentDifficulty_;
}

void DifficultyManager::PreloadCSVFiles() {
	const auto& paths = pathMap_[currentDifficulty_];
	LoadCSVFilesToMemory(paths);
}

void DifficultyManager::LoadCSVFilesToMemory(const std::vector<std::string>& paths) {
	cachedWaveData_.clear();
	for (const auto& path : paths) {
		std::ifstream file(path);
		assert(file.is_open());
		std::stringstream buffer;
		buffer << file.rdbuf();
		cachedWaveData_.push_back(buffer.str());
		file.close();
	}
}

size_t DifficultyManager::GetWaveCount() const {
	return cachedWaveData_.size();
}

std::istringstream DifficultyManager::GetWaveStream(int index) const {
	assert(index >= 0 && index < cachedWaveData_.size());
	return std::istringstream(cachedWaveData_[index]);
}