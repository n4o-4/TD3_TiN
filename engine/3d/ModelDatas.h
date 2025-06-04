#pragma once
#include <string>
#include <vector>
#include "Structs.h"
#include <map>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp\postprocess.h>
#include <iostream>
#include <cassert>
#include <optional>
struct Node
{
	QuaternionTransform transform;
	Matrix4x4 localMatrix;
	std::string name;
	std::vector<Node> children;

};

struct MaterialData
{
	std::string textureFilePath;
	uint32_t textureIndex;
};

struct Material {
	Vector4 color;
	int enableLighting;
	float padding[3];
	Matrix4x4 uvTransform;
	float shininess;
	Vector3 specularColor;
};

struct VertexData
{
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};

struct ModelData {
	std::vector<VertexData> vertices;
	MaterialData material;
	Node rootNode;
};


struct Joint
{
	QuaternionTransform transform; // transform情報
	Matrix4x4 localMatrix; // localMatrix
	Matrix4x4 skeletonSpaceMatrix; // skeletonSpaceでの変換行列
	std::string name; // 名前
	std::vector<int32_t> children; // 子JointのIndexのリスト。いなければ空
	int32_t index; // 自身のIndex
	std::optional<int32_t> parent; // 親JointのIndex。いなければnull
};

struct Skeleton
{
	int32_t root; //RootJointのIndex
	std::map<std::string, int32_t> jointMap; // Joint名とIndexとの辞書
	std::vector<Joint> joints; // 所属しているJoint
};

namespace ParticleModel
{
	struct ModelData
	{
		std::vector<VertexData> vertices;
		MaterialData material;
	};

	ParticleModel::ModelData LoadModelFile(const std::string& directoryPath, const std::string& filename);
}
