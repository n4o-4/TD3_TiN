#pragma once
#include "DirectXCommon.h"

class SrvManager
{
public:

	// 最大SRV数(テクスチャ枚数)
	static const uint32_t kMaxSRVCount;

private:

    DirectXCommon* dxCommon_ = nullptr;

	// SRV用のデスクリプタサイズ
	uint32_t descriptorSize;

	// SRV用デスクリプタヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap;

	// 次に使用するSRVインデックス
	uint32_t useIndex = 0;

public:

	// 初期化
	void Initialize(DirectXCommon* dxCommon);

	// 
	uint32_t Allocate();

	// 
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(uint32_t index);

	// SRV生成(テクスチャ用)
	void CreateSRVforTexture2D(uint32_t srvIndex, ID3D12Resource* pResource, DXGI_FORMAT Format, UINT MipLevels);

	// offScreen、renderTexture用
	void CreateOffScreenTexture(uint32_t srvIndex,uint32_t rtvIndex);

	// SRV生成(Structure Buffer用)
	void CreateSRVforStructuredBuffer(uint32_t srvIndex, ID3D12Resource* pResource, UINT numElements, UINT structureByteStride);

	void PreDraw();

	// SRVセットコマンド
	void SetGraphicsRootDescriptorTable(UINT RootParameterIndex, uint32_t srvIndex);

	bool CheckSrvCount();

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetDescriptorHeap() { return descriptorHeap; }

	uint32_t GetDescriptorSize() { return descriptorSize; }
private:

	void CreateSrvForDepth();


};

