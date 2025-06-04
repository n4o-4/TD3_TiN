#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <dxgi1_6.h>
#include <cassert>
#include <dxcapi.h>
#include <format>
#include <memory>
#include "externals/DirectXTex/DirectXTex.h"
#include <dxgidebug.h>
#include "Structs.h"

class OffScreenRendring
{
/// --------------------------------
/// メンバ関数
///---------------------------------
public:
	void Initialzie();

	void PreDraw();

	void PostDraw();

	void SetRenderTextureResource(ID3D12Resource* renderTextureResources) { *renderTextureResources_ = renderTextureResources; }

private:

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateRenderTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, uint32_t width, uint32_t height, DXGI_FORMAT format, const Vector4& clearColor);

/// --------------------------------
/// メンバ変数
///---------------------------------
public:

private:

	ID3D12Resource* renderTextureResources_[2] = {nullptr};

	D3D12_RESOURCE_BARRIER barrier{};
};