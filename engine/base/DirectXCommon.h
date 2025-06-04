#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>
#include <wrl.h>

#include "Logger.h"
#include "StringUtility.h"
#include "WinApp.h"
#include <dxcapi.h>
#include <format>
#include <chrono>
#include <thread>
#include <memory>
#include "externals/DirectXTex/DirectXTex.h"
#include <dxgidebug.h>

#include "Structs.h"
#include "OffScreenRendring.h"

class DirectXCommon
{
public:
	// シングルトンインスタンスの取得
	static DirectXCommon* GetInstance();

	void Initialize(WinApp* winApp);

	void Finalize();

	void RenderTexturePreDraw();
	void RenderTexturePostDraw();

	// 描画前処理
	void PreDraw();

	// 描画後処理
	void PostDraw();

	Microsoft::WRL::ComPtr<ID3D12Device> GetDevice() { return device; }

	DXGI_SWAP_CHAIN_DESC1 GetSeapChainDesc() { return swapChainDesc; }

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> GetCommandQueue() { return commandQueue; }

	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> GetCommandAllocator() { return commandAllocator; }

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> GetCommandList() { return commandList; }

	size_t GetBackBufferCount() const { return backBufferCount_; }

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);


	Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, int32_t width, int32_t height);


	Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(
		// CompilerするShaderファイルへのパス
		const std::wstring& filePath,

		// Compilerに使用するProfile
		const wchar_t* profile);

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(size_t sizeInBytes);

	// 読み込んだTexture情報をもとにTextureResourceを作る関数
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(const DirectX::TexMetadata& metadata);

	// データを転送するUploadTextureData関数
	void UploadTextureData(Microsoft::WRL::ComPtr<ID3D12Resource> texture, const DirectX::ScratchImage& mipImages);

	// Textureデータを読み込む関数
	DirectX::ScratchImage LoadTexture(const std::string& filePath);

	static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
		handleCPU.ptr += (descriptorSize * index);
		return handleCPU;
	}

	static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index) {
		D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap.Get()->GetGPUDescriptorHandleForHeapStart();
		handleGPU.ptr += (descriptorSize * index);
		return handleGPU;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE* GetRTVHandle(int index) { return &rtvHandles[index]; }
	void SetRTVHandle(int index) { rtvHandles[index].ptr = rtvHandles[index - 1].ptr + device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV); }

	D3D12_RENDER_TARGET_VIEW_DESC* GetRtvDesc() { return &rtvDesc; }

	UINT GetBufferIndex() { UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex(); return backBufferIndex; }

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetDSVHeap() { return dsvDescriptorHeap; }

	D3D12_VIEWPORT* GetViewPort() { return &viewport; }

	D3D12_RECT* GetRect() { return &scissorRect; }

	Microsoft::WRL::ComPtr<ID3D12Resource>* GetRenderTextureResources() { return renderTextureResources; }

	D3D12_CPU_DESCRIPTOR_HANDLE GetDsvHandle() { return dsvHandle; }

	IDXGISwapChain4* GetSwapChain() { return swapChain.Get(); }

	Microsoft::WRL::ComPtr<ID3D12Resource>* GetSwapChainResources() { return swapChainResources; }

	uint32_t GetRenderResourceIndex() { return renderResourceIndex_; }
	uint32_t GetRenderTargetIndex() { return renderTargetIndex_; }

	void SetRenderResourceIndex(uint32_t renderResourceIndex) { renderResourceIndex_ = renderResourceIndex; }
	void SetRenderTargetIndex(uint32_t renderTargetIndex) { renderTargetIndex_ = renderTargetIndex; }

	//========================================================
	// 深度リソースのsrvIndex
	void SetDepthSrvIndex(uint32_t srvIndex) { depthSrvIndex_ = srvIndex; }

	void SetDepthHandle(D3D12_CPU_DESCRIPTOR_HANDLE depthHandle) { depthHandle_ = depthHandle;}

	D3D12_CPU_DESCRIPTOR_HANDLE GetDsvHandles() { return dsvHandle; }

	uint32_t GetDepthSrvIndex() { return depthSrvIndex_; }

	Microsoft::WRL::ComPtr<ID3D12Resource> GetDepthStencilResource() { return depthStencilResource; }

	D3D12_CPU_DESCRIPTOR_HANDLE GetDepthSrvHandle() { return depthHandle_; }

private:  
	//デバイス初期化
	void InitializeDevice();

	// コマンド関連の初期化
	void InitializeCommands();

	// スワップチェーンの生成
	void CreateSwapChain();

	// 深度バッファの生成
	void CreateDepthBuffer();

	// 各種出スクリプターヒープの生成
	void CreateDescriptorHeaps();

	// レンダーターゲットビューの初期化
	void InitializeRenderTergetView();

	// 深度ステンシルビューの初期化
	void InitializeDepthStencilView();

	// フェンスの生成
	void InitializeFence();

	// ビューポート矩形の初期化
	void InitializeViewPort();

	// シザリング矩形
	void Scissor();

	// DXCコンパイラーの生成
	void CreateDXCCompiler();

	// 
	// 
	// の初期化
	void InitializeImGui();

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateRenderTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device,uint32_t width,uint32_t height, DXGI_FORMAT format,const Vector4& clearColor,D3D12_RESOURCE_STATES resourceStates);

	void CreateRenderTextureRTV();


	// FPS固定初期化
	void InitializeFixFPS();

	// FPS固定更新
	void UpdateFixFPS();

	//========================================================
	// オフスクリーン用

	void CreateOffScreenRootSignature();

	void CreateOffScreenPipeLine();
	
	
	
private:

	/*---------------------
	* メンバー変数
	---------------------*/

	static std::unique_ptr<DirectXCommon> instance;

	friend std::unique_ptr<DirectXCommon> std::make_unique<DirectXCommon>();
	friend std::default_delete<DirectXCommon>;

	DirectXCommon() = default;
	~DirectXCommon() = default;
	DirectXCommon(DirectXCommon&) = delete;
	DirectXCommon& operator=(DirectXCommon&) = delete;

	// WindowAPI
	WinApp* winApp = nullptr;

	// DirectX12デバイス
	Microsoft::WRL::ComPtr<ID3D12Device> device = nullptr;

	// DXGIファクトリ
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory = nullptr;

	Microsoft::WRL::ComPtr<IDXGISwapChain4>  swapChain = nullptr;
		
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};

	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResources[2] = { nullptr };

	Microsoft::WRL::ComPtr< ID3D12Fence> fence = nullptr;

	D3D12_VIEWPORT viewport{};

	D3D12_RECT scissorRect{};

	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils = nullptr;

	Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler = nullptr;

	Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler = nullptr;

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[4];

	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle;

	//D3D12_RESOURCE_BARRIER barrier{};

	HANDLE fenceEvent = nullptr;

	uint32_t fenceValue;

	uint32_t backBufferCount_ = 2;

	// 記録時間(FPS固定用)
	std::chrono::steady_clock::time_point reference_;

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue = nullptr;

	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;

	//uint32_t descriptorSizeSRV = 0;

	uint32_t descriptorSizeRTV = 0;

	uint32_t descriptorSizeDSV = 0;

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap = nullptr;

	//Microsoft::WRL::ComPtr< ID3D12DescriptorHeap> srvDescriptorHeap = nullptr;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Debug1> debugController = nullptr;

	Microsoft::WRL::ComPtr< ID3D12RootSignature> rootSignature;

	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState;


	Microsoft::WRL::ComPtr< ID3DBlob> signatureBlob = nullptr;
	
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;

	//========================================================
	// 深度リソース用
	uint32_t depthSrvIndex_ = 0;

	D3D12_CPU_DESCRIPTOR_HANDLE depthHandle_;

	//========================================================
	// ポストエフェクト用

	Microsoft::WRL::ComPtr<ID3D12Resource> renderTextureResources[2] = { nullptr };

	uint32_t renderResourceIndex_ = 0;
	uint32_t renderTargetIndex_ = 1;
};