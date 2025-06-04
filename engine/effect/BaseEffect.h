#pragma once
#include "DirectXCommon.h"
#include "TextureManager.h"
#include "CameraManager.h"

#include "Vectors.h"

struct Pipeline
{
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;
};


class BaseEffect
{
public:

	// 初期化
	virtual void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager);

	// 更新
	virtual void Update() = 0;

	// 描画
	virtual void Draw(uint32_t renderTargetIndex, uint32_t renderResourceIndex) = 0;

	// カメラマネージャの設定
	virtual void SetCameraManager(CameraManager* cameraManager);

	virtual void Reset();

	bool GetIsActive() { return isActive_; }

protected:

	DirectXCommon* dxCommon_ = nullptr;

	SrvManager* srvManager_ = nullptr;

	std::unique_ptr<Pipeline> pipeline_ = nullptr;

	bool isActive_ = true;
};

