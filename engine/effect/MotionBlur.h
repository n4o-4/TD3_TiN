#pragma once
#include "BaseEffect.h"

namespace MotionBlurShader
{
	struct Material
	{
		int numSamples;
		Vector2 center;
		float blurWidth;

		Vector3 diff;
		float padding[1];
	};
}


class MotionBlur : public BaseEffect
{
public:

	// 初期化
	void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager) override;

	// 更新
	void Update() override;

	// 描画
	void Draw(uint32_t renderTargetIndex, uint32_t renderResourceIndex) override;

	// カメラマネージャの設定
	void SetCameraManager(CameraManager* cameraManager) override { cameraManager_ = cameraManager; }

	// リソースの解放
	void Reset() override { resource_.Reset(); }

private:

	// パイプラインの生成
	void CreatePipeline();

	// ルートシグネチャの生成と設定
	void CreateRootSignature(Pipeline* pipeline);

	// パイプラインステートの生成と設定
	void CreatePipeLineState(Pipeline* pipeline);

	// マテリアルの生成
	void CreateMaterial();

private: // メンバ変数
	
	CameraManager* cameraManager_ = nullptr;

	// 
	Microsoft::WRL::ComPtr<ID3D12Resource> resource_;

	// 
	MotionBlurShader::Material* data_ = nullptr;
	
	Vector3 currentPos_{};

	Vector3 prePos_{};

	Vector3 diffPos_{};

public:

	float blurWidth_;
	int numSamples_;

};

