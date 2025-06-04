#pragma once
#include "DirectXCommon.h"

class SpriteCommon
{
public: // メンバ関数

	// シングルトンインスタンスの取得
	static SpriteCommon* GetInstance();

	// 初期化
	void Initialize(DirectXCommon* dxCommon);

	void Finalize();

	void SetForegroundView();

	void SetBackgroundView();

	void DrawForeground();

	void DrawBackground();

	DirectXCommon* GetDxCommon() const { return dxCommon_; }

	bool GetIsDrawBackground() const { return isDrawBackground; }	
	bool GetIsDrawForeground() const { return isDrawForeground; }	

private:

	static std::unique_ptr<SpriteCommon> instance;

	friend std::unique_ptr<SpriteCommon> std::make_unique<SpriteCommon>();
	friend std::default_delete<SpriteCommon>;

	SpriteCommon() = default;
	~SpriteCommon() = default;
	SpriteCommon(SpriteCommon&) = delete;
	SpriteCommon& operator=(SpriteCommon&) = delete;

	// ルートシグネチャの生成
	void CreateRootSignature();

	// グラフィックスパイプラインの生成
	void CreateGraphicsPipelineForeground();

	// グラフィックスパイプラインの生成
	void CreateGraphicsPipelineBackground();

private:
	DirectXCommon* dxCommon_;

	Microsoft::WRL::ComPtr< ID3D12RootSignature> rootSignature;

	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineStateBackground;

	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineStateForeground;

	//ID3DBlob* signatureBlob = nullptr;
	Microsoft::WRL::ComPtr< ID3DBlob> signatureBlob = nullptr;
	//ID3DBlob* errorBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;

	bool isDrawBackground = false;	

	bool isDrawForeground = false;	
};