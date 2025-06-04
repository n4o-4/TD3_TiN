#pragma once
#include "DirectXCommon.h"
#include "SrvManager.h"
#include "WorldTransform.h"
#include "ViewProjection.h"
#include "ModelDatas.h"



class LineDrawerBase
{
public:

	enum class Type
	{
		AABB,
		Sphere,
		Grid,
		Skeleton,
	};

private:

	struct Sphere {
		Vector3 center;
		float radius;
		unsigned int color;
	};

	static const int kMaxLines = 2048;

	struct Pipeline
	{
		Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;
	};

	struct VertexData
	{
		Vector4 position;
	};

	struct LineForGPU
	{
		Matrix4x4 matWorld;
		Vector4 color;
	};

	struct LineObject
	{
		Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = nullptr;
		VertexData* vertexData = nullptr;
		D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
		
		Microsoft::WRL::ComPtr<ID3D12Resource> lineResource = nullptr;
		LineForGPU* lineData = nullptr;

		WorldTransform* transform;

		Type type;

		int32_t vertexIndex = 0;

		Skeleton skeleton;
	};

	
public: // メンバ関数

	// 初期化
	void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager);

	// 更新
	void Update();
	
	// スケルトンの更新
	void SkeletonUpdate(Skeleton skeleton);

	// 描画
	void Draw(ViewProjection viewProjection);

private: // メンバ関数

	void CreateRootSignature();

	void CreatePipellineState();

public:

	void CreateLineObject(Type type, WorldTransform* transform);

	void CreateSkeletonObject(Skeleton skeleton, WorldTransform* transform);

private:

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateVertexResource();

	void CreateVertexBufferView(LineObject* object);

	void WriteLineData(LineObject* object);

	void WriteSkeletonLineData(LineObject* object, Skeleton skeleton);

	void CreateLineResource(LineObject* object);

private: // メンバ変数

	DirectXCommon* dxCommon_ = nullptr;

	SrvManager* srvManager_ = nullptr;

	std::unique_ptr<Pipeline> pipeline_ = nullptr;

	std::list<std::unique_ptr<LineObject>> lineObjects_;

	std::unique_ptr<WorldTransform> worldTransform_ = nullptr;
};

