#pragma once
#include "DirectXCommon.h"
#include "Camera.h"

class Object3dCommon
{
public: // メンバ関数

	// シングルトンインスタンスの取得
	static Object3dCommon* GetInstance();

	// 初期化
	void Initialize(DirectXCommon* dxCommon);

	void Finalize();

	// 共通描画設定
	void SetView();

	DirectXCommon* GetDxCommon() const { return dxCommon_; }

	//void SetDefaultCamera(Camera* camera) { this->defaultCamera_ = camera; }

	Camera* GetDefaultCamera() const { return defaultCamera_.get(); }

	Microsoft::WRL::ComPtr<ID3D12PipelineState> GetGraphicsPipelineState() { return graphicsPipelineState; }
	Microsoft::WRL::ComPtr<ID3D12PipelineState> GetDisableDepthTestPipelineState() { return disableDepthTestPipelineState; }

private:

	static std::unique_ptr<Object3dCommon> instance;

	friend std::unique_ptr<Object3dCommon> std::make_unique<Object3dCommon>();
	friend std::default_delete<Object3dCommon>;

	Object3dCommon() = default;
	~Object3dCommon() = default;
	Object3dCommon(Object3dCommon&) = delete;
	Object3dCommon& operator=(Object3dCommon&) = delete;

	// グラフィックスパイプラインの生成
	void CreateGraphicsPipeline();

	// ルートシグネチャの生成
	void CreateRootSignature();

private: // メンバ変数
	DirectXCommon* dxCommon_;

    Microsoft::WRL::ComPtr< ID3D12RootSignature> rootSignature;

	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState;

	Microsoft::WRL::ComPtr<ID3D12PipelineState> disableDepthTestPipelineState;

	//ID3DBlob* signatureBlob = nullptr;
	Microsoft::WRL::ComPtr< ID3DBlob> signatureBlob = nullptr;
	//ID3DBlob* errorBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;

	std::unique_ptr<Camera> defaultCamera_ = nullptr;
};

