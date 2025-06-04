#pragma once
#include "BaseEffect.h"


namespace LinearFogShader
{
	struct Material
	{

		Matrix4x4 projectionInverse;

		Vector4 color = { 1.0f, 1.0f, 1.0f, 1.0f }; // フォグの色

		float strength = 0.1f; // フォグの強さ  

		float start = 10.0f; // フォグの開始位置  

		float end = 200.0f; // フォグの終了位置  

		float padding[1]; // パディング (16バイト境界のため)  
	};
}

class LinearFog : public BaseEffect
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

	// 
	Microsoft::WRL::ComPtr<ID3D12Resource> resource_;

	// 
	LinearFogShader::Material* data_ = nullptr;

	//
	CameraManager* cameraManager_ = nullptr;
};

