#pragma once
#include "wrl.h"
#include "d3d12.h"
#include "Vectors.h"
#include "MyMath.h"
#include <cstdint>
#include <string>

class SpriteCommon;

class Sprite
{
public: // メンバ関数
	// 初期化
	void Initialize(SpriteCommon* spriteCommon,std::string textureFilePath);

	void Update();

	void Draw();

	const Vector2& GetPosition() const { return position;}

	void SetPosition(const Vector2& position) { this->position = position; }
	
	float GetRotation() const { return rotation; }
	
	void SetRotation(float rotation) { this->rotation = rotation; }

	const Vector4 GetColor() const { return materialData->color; }

	void SetColor(const Vector4& color) { materialData->color = color; }
	
	const Vector2& GetSize() const { return size; }

	void SetSize(const Vector2& size) { this->size = size; }

	// getter
	const Vector2& GetAnchorPoint() const { return anchorPoint; }

	// setter
	void SetAnchorPoint(const Vector2& anchorPoint) { this->anchorPoint = anchorPoint; }

	// getter
	const bool GetIsFlipX() { return isFlipX_; }
	const bool GetIsFlipY() { return isFlipY_; }

	// setter
	void SetIsFlipX(bool isFlipX) { isFlipX_ = isFlipX; }
	void SetIsFlipY(bool isFlipY) { isFlipY_ = isFlipY; }

	// getter
	const Vector2& GetTexLeftTop() { return textureLeftTop; }
	const Vector2& GetTexSize() { return textureSize; }

	// setter
	void SetTexLeftTop(const Vector2& leftTop) { textureLeftTop = leftTop; }
	void SetTexSize(const Vector2& TexSize) { textureSize = TexSize; }

private: 

	void CreateVertexData();

	void CreateIndexData();

	void CreateMaterialData();

	void CreateTransformationMatrixData();

	// 
	void AdjustTextureSize();

private:

	struct VertexData {
		Vector4 position;
		Vector2 texcoord;
		Vector3 normal;
	};

	struct Material {
		Vector4 color;
		int enableLighting;
		float padding[3];
		Matrix4x4 uvTransform;
	};

	struct TransformationMatrix {
		Matrix4x4 WVP;
		Matrix4x4 World;
	};

	struct Transform
	{
		Vector3 scale;
		Vector3 rotate;
		Vector3 translate;
	};

	SpriteCommon* spriteCommon = nullptr;

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource = nullptr;

	// バッファリソース内のデータを指すポインタ
	VertexData* vertexData;
	uint32_t* indexData;

	// バッファリソースの使い道を補足するバッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;

	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	Material* materialData = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> transformationResource = nullptr;

	TransformationMatrix* transformationMatrixData;

	Transform cameraTransform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-15.0f} };

	Transform transform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	Transform uvTransform{
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f},
	};

	std::string textureFilePath;

	Vector2 position = { 0.0f,0.0f };

	float rotation = 0.0f;

	Vector2 size = { 640.0f,360.0f };

	// テクスチャ番号
	uint32_t textureIndex = 0;

	Vector2 anchorPoint = { 0.0f,0.0f };

	// 左右フリップ
	bool isFlipX_ = false;

	// 上下フリップ
	bool isFlipY_ = false;

	// テクスチャ左上座標
	Vector2 textureLeftTop = { 0.0f,0.0f };

	// テクスチャ切り出しサイズ
	Vector2 textureSize = { 100.0f,100.0f };

};

