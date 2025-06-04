#pragma once
#include "Vectors.h"
#include "Matrixs.h"
#include "ModelCommon.h"
#include <fstream>
#include <string>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp\postprocess.h>
#include "WorldTransform.h"
#include "ViewProjection.h"

#include "ModelDatas.h"

//struct Node
//	{
//		Matrix4x4 localMatrix;
//		std::string name;
//		std::vector<Node> children;
//	};

// 3Dモデル
class Model
{
public:
	// 初期化
	void Initialize(ModelCommon* modelCommon,const std::string& directoryPath,const std::string& filename);

	// 描画
	void Draw(WorldTransform worldTransform);	

private:

	// .mtlファイルの読み込み
	static MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);

	static ModelData LoadModelFile(const std::string& directoryPath, const std::string& filename);

public:

	static Node ReadNode(aiNode* node);

private:

private:
	ModelCommon* modelCommon_;

	ModelData modelData;

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;

	VertexData* vertexData = nullptr;

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

	/*Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;

	Material* materialData = nullptr;*/
};

