#include "SrvManager.h"

const uint32_t SrvManager::kMaxSRVCount = 512;

void SrvManager::Initialize(DirectXCommon* directXCommon)
{
	// メンバ変数に記録
	this->dxCommon_ = directXCommon;

	// デスクリプタヒープの生成
	descriptorHeap = directXCommon->CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, kMaxSRVCount, true);

	// デスクリプタ1個分のサイズを取得して記録
	descriptorSize = directXCommon->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	CreateSrvForDepth();
}

uint32_t SrvManager::Allocate()
{
	// return する番号を一旦記録しておく
	int index = useIndex;

	// 次回のために番号を1進める
	useIndex++;

	// 記録した番号をreturn
	return index;
}

D3D12_CPU_DESCRIPTOR_HANDLE SrvManager::GetCPUDescriptorHandle(uint32_t index)
{
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize * index);

	return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE SrvManager::GetGPUDescriptorHandle(uint32_t index)
{
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descriptorSize * index);

	return handleGPU;
}

void SrvManager::CreateSRVforTexture2D(uint32_t srvIndex, ID3D12Resource* pResource, DXGI_FORMAT Format, UINT MipLevels)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};

	srvDesc.Format = Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャ
	srvDesc.Texture2D.MipLevels = MipLevels;

	dxCommon_->GetDevice()->CreateShaderResourceView(pResource, &srvDesc, GetCPUDescriptorHandle(srvIndex));
}

void SrvManager::CreateOffScreenTexture(uint32_t srvIndex, uint32_t rtvIndex)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	assert(dxCommon_->GetRenderTextureResources()[rtvIndex].Get());

	dxCommon_->GetDevice()->CreateShaderResourceView(dxCommon_->GetRenderTextureResources()[rtvIndex].Get(), &srvDesc, GetCPUDescriptorHandle(srvIndex));
}

void SrvManager::CreateSRVforStructuredBuffer(uint32_t srvIndex, ID3D12Resource* pResource, UINT numElements, UINT structureByteStride)
{

	// SRV の記述子を設定する
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = numElements;
	srvDesc.Buffer.StructureByteStride = structureByteStride;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN; // 構造化バッファでは通常 UNKNOWN

	// デバイスを使って SRV を作成する
	dxCommon_->GetDevice()->CreateShaderResourceView(pResource, &srvDesc, GetCPUDescriptorHandle(srvIndex));
}

void SrvManager::PreDraw()
{
	// 描画用のDescriptorHeapの設定
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeaps[] = { descriptorHeap.Get() };
	dxCommon_->GetCommandList()->SetDescriptorHeaps(1, descriptorHeaps->GetAddressOf());
}

void SrvManager::SetGraphicsRootDescriptorTable(UINT RootParameterIndex, uint32_t srvIndex)
{
	dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(RootParameterIndex, GetGPUDescriptorHandle(srvIndex));
}

bool SrvManager::CheckSrvCount()
{
	if (useIndex < kMaxSRVCount) {
		return true;
	}
	else {
		return false;
	}
}

void SrvManager::CreateSrvForDepth()
{
	uint32_t index = Allocate();

    ID3D12Resource* depthStencilResource = dxCommon_->GetDepthStencilResource().Get();

	depthStencilResource->SetName(L"DepthTexture");

	D3D12_SHADER_RESOURCE_VIEW_DESC depthTextureSrvDesc{};
	depthTextureSrvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	depthTextureSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	depthTextureSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	depthTextureSrvDesc.Texture2D.MipLevels = 1;
	dxCommon_->GetDevice()->CreateShaderResourceView(depthStencilResource, &depthTextureSrvDesc, GetCPUDescriptorHandle(index));

	dxCommon_->SetDepthSrvIndex(index);
	dxCommon_->SetDepthHandle(GetCPUDescriptorHandle(index));
}
