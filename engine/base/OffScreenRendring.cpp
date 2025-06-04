#include "OffScreenRendring.h"

#include "DirectXCommon.h"

void OffScreenRendring::Initialzie()
{
}

void OffScreenRendring::PreDraw()
{
	


}

void OffScreenRendring::PostDraw()
{
	


}

Microsoft::WRL::ComPtr<ID3D12Resource> OffScreenRendring::CreateRenderTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, uint32_t width, uint32_t height, DXGI_FORMAT format, const Vector4& clearColor)
{
	Microsoft::WRL::ComPtr<ID3D12Resource> resource;

	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = UINT(width); // Textureの幅
	resourceDesc.Height = UINT(height); // Textureの高さ
	resourceDesc.MipLevels = 1; // mipmapの数
	resourceDesc.DepthOrArraySize = 1; // 奥行 or 配列Textureの配列数
	resourceDesc.Format = format; // TextureのFormat
	resourceDesc.SampleDesc.Count = 1; // サンプルカウント。1固定
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; // Textureの次元数
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	// 利用するHeapの設定。
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_CLEAR_VALUE clearValue;
	clearValue.Format = format;
	clearValue.Color[0] = clearColor.x;
	clearValue.Color[1] = clearColor.y;
	clearValue.Color[2] = clearColor.z;
	clearValue.Color[3] = clearColor.w;

	device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		&clearValue,
		IID_PPV_ARGS(&resource));

	return resource;
}


