#pragma once
#include "BaseEffect.h"
class BoxFilter : public BaseEffect
{
public:

	// 初期化
	void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager) override;

	// 更新
	void Update() override;

	// 描画
	void Draw(uint32_t renderTargetIndex, uint32_t renderResourceIndex) override;

private:

	void CreatePipeline();


	void CreateRootSignature(Pipeline* pipeline);


	void CreatePipeLineState(Pipeline* pipeline);

};

