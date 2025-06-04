#include "ModelManager.h"

std::unique_ptr<ModelManager> ModelManager::instance = nullptr;

ModelManager* ModelManager::GetInstance()
{
	if (instance == nullptr) {
		instance = std::make_unique<ModelManager>();
	}

	return instance.get();
}

void ModelManager::Initialize(DirectXCommon* dxCommon)
{
	/*modelCommon = new ModelCommon;
	modelCommon->Initialize(dxCommon);*/

	modelCommon = std::make_unique<ModelCommon>();
	modelCommon->Initialize(dxCommon);
}

void ModelManager::Finalize()
{
	instance.reset();
}

void ModelManager::LoadModel(const std::string& filePath)
{
	// 読み込み済みモデル検索
	if (models.contains(filePath)) {
		return;
	}

	// モデルの生成とファイル読み込み、初期化
	std::unique_ptr<Model> model = std::make_unique<Model>();
	model->Initialize(modelCommon.get(), "Resources", filePath);

	// モデルをmapコンテナに格納する
	models.insert(std::make_pair(filePath, std::move(model)));
}

Model* ModelManager::FindModel(const std::string& filePath)
{
	// 読み込み済みモデルを検索
	if (models.contains(filePath)) {
		// 読み込みモデルを戻り値としてreturn
		return models.at(filePath).get();
	}

	// ファイル名一致なし
	return nullptr;
}
