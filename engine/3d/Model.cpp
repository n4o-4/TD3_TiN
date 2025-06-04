#include "Model.h"
#include "TextureManager.h"
#include "MyMath.h"
#include <iostream>


void Model::Initialize(ModelCommon* modelCommon, const std::string& directoryPath, const std::string& filename)
{
	modelCommon_ = modelCommon;

	modelData = LoadModelFile(directoryPath, filename);

	TextureManager::GetInstance()->LoadTexture(modelData.material.textureFilePath);

	modelData.material.textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(modelData.material.textureFilePath);

	vertexResource = this->modelCommon_->GetDxCommon()->CreateBufferResource(sizeof(VertexData) * modelData.vertices.size());

	vertexBufferView.BufferLocation = vertexResource.Get()->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	vertexResource.Get()->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());

	//materialResource = this->modelCommon_->GetDxCommon()->CreateBufferResource(sizeof(Material));

	//// 書き込むためのアドレスを取得
	//materialResource.Get()->Map(0, nullptr, reinterpret_cast<void**>(&materialData));

	//materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	//materialData->enableLighting = true;
	//materialData->uvTransform = MakeIdentity4x4();

	//materialData->shininess = 48.3f;

	//materialData->specularColor = { 1.0f,1.0f,1.0f };

	/*TextureManager::GetInstance()->LoadTexture(modelData.material.textureFilePath);

	modelData.material.textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(modelData.material.textureFilePath);*/
}

void Model::Draw(WorldTransform worldTransform)
{
	//worldTransform.matWorld_ = Multiply(modelData.rootNode.localMatrix,worldTransform.matWorld_);

	//worldTransform.UpdateMatrix();

	modelCommon_->GetDxCommon()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);  // VBVを設定

	modelCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(modelData.material.textureFilePath));

	modelCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(4, worldTransform.GetTransformResource()->GetGPUVirtualAddress());

	modelCommon_->GetDxCommon()->GetCommandList()->DrawInstanced(UINT(modelData.vertices.size()), 1, 0, 0);
}

MaterialData Model::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename)
{
	MaterialData materialData;
	std::string line;
	std::ifstream file(directoryPath + "/" + filename);
	assert(file.is_open());
	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		// identifierに応じた処理
		if (identifier == "map_Kd") {
			std::string textureFilename;
			s >> textureFilename;

			// 連結してファイルパスにする
			materialData.textureFilePath = directoryPath + "/" + textureFilename;
		}
	}
	return materialData;
}

ModelData Model::LoadModelFile(const std::string& directoryPath, const std::string& filename)
{
	ModelData modelData;
	
	Assimp::Importer importer;

	std::string filePath = directoryPath + "/" + filename;

	const aiScene* scene = importer.ReadFile(filePath.c_str(), aiProcess_FlipWindingOrder | aiProcess_FlipUVs);

	if (!scene) {
		std::cerr << "Failed to load model: " << filePath << std::endl;
		std::cerr << "Assimp error: " << importer.GetErrorString() << std::endl;
		throw std::runtime_error("Model loading failed.");
	}

	assert(scene->HasMeshes()); // メッシュがないのは対応しない

	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex)
	{
		aiMesh* mesh = scene->mMeshes[meshIndex];
		assert(mesh->HasNormals()); // 法線がないMeshは今回は非対応
		assert(mesh->HasTextureCoords(0)); // TexcoordがないMesh
		// ここからMeshの中身(Face)の解析を行っていく

		for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex)
		{
			aiFace& face = mesh->mFaces[faceIndex];

			assert(face.mNumIndices == 3); // 三角形のみサポート
			// ここからFaceの中身(Vertex)の解析を行っていく

			for (uint32_t element = 0; element < face.mNumIndices; ++element)
			{
				uint32_t vertexIndex = face.mIndices[element];
				aiVector3D& position = mesh->mVertices[vertexIndex];
				aiVector3D& normal = mesh->mNormals[vertexIndex];
				aiVector3D& texcoord = mesh->mTextureCoords[0][vertexIndex];
				VertexData vertex;
				vertex.position = { position.x,position.y,position.z,1.0f };
				vertex.normal = { normal.x,normal.y,normal.z };
				vertex.texcoord = { texcoord.x,texcoord.y };
				// 
				vertex.position.x *= -1.0f;
				vertex.normal.x *= -1.0f;
				modelData.vertices.push_back(vertex);
			}

		}
	}

	for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex)
	{
		aiMaterial* material = scene->mMaterials[materialIndex];
		if (material->GetTextureCount(aiTextureType_DIFFUSE) != 0)
		{
			aiString textureFilePath;
			material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilePath);
			modelData.material.textureFilePath = directoryPath + "/" + textureFilePath.C_Str();
		}
	}

	modelData.rootNode = ReadNode(scene->mRootNode);

	return modelData;
}

Node Model::ReadNode(aiNode* node)
{
	Node result;
	
	aiVector3D scale, translate;
	aiQuaternion rotate;

	node->mTransformation.Decompose(scale, rotate, translate);
	result.transform.scale = { scale.x,scale.y,scale.z };
	result.transform.rotate = { rotate.x,-rotate.y,-rotate.z,rotate.w };
	result.transform.translate = { -translate.x,translate.y,translate.z };	
	
	result.localMatrix = MakeAffineMatrixforQuater(result.transform.scale, result.transform.rotate, result.transform.translate);

	result.name = node->mName.C_Str();
	result.children.resize(node->mNumChildren);

	for (uint32_t childIndex = 0; childIndex < node->mNumChildren; ++childIndex)
	{

		result.children[childIndex] = ReadNode(node->mChildren[childIndex]);
	}

	return result;
}

