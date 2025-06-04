#include "DirectXCommon.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"dxcompiler.lib")

#include "Logger.h"

#include "TextureManager.h"

std::unique_ptr<DirectXCommon> DirectXCommon::instance = nullptr;

DirectXCommon* DirectXCommon::GetInstance()
{
	if (instance == nullptr) {
		instance = std::make_unique<DirectXCommon>();
	}

	return instance.get();
}


void DirectXCommon::Initialize(WinApp* winApp)
{
	InitializeFixFPS();
	assert(winApp);

	this->winApp = winApp;

	InitializeDevice();
	InitializeCommands();
	CreateSwapChain();
	CreateDepthBuffer();
	CreateDescriptorHeaps();
	InitializeRenderTergetView();
	
	/// 追加↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓

	CreateRenderTextureRTV();

	/// 追加↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑

	InitializeDepthStencilView();
	InitializeFence();
	InitializeViewPort();
	Scissor();
	CreateDXCCompiler();
	InitializeImGui();

	dsvHandle = dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	CreateOffScreenPipeLine();
}

void DirectXCommon::Finalize()
{
	instance.reset();
}

void DirectXCommon::InitializeDevice()
{
	HRESULT hr;

	// デバッグレイヤーを有効化
#ifdef _DEBUG
	
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
	{
		// デバッグレイヤーを有効化する
		debugController->EnableDebugLayer();

		// さらにGPU側でもチェックを行うようにする
		debugController->SetEnableGPUBasedValidation(TRUE);
	}
#endif

	// DXGIFactoryの生成
	hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	assert(SUCCEEDED(hr));

	// アダプターの列挙
	//使用するアダプタ用の変数。最初にnullptrを入れておく
	Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter = nullptr;

	//良い順にアダプタを頼む
	for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter)) != DXGI_ERROR_NOT_FOUND; i++)
	{
		//アダプターの情報を取得する
		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr = useAdapter->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr));
		//ソフトウェアアダプタでなければ採用!
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE))
		{
			//採用したアダプタの情報をログに出力。wstringの方なので注意
			Logger::Log(StringUtility::ConvertString(adapterDesc.Description));
			break;
		}
		useAdapter = nullptr; // ソフトウェアの場合は見なかったことにする
	}

	// 適切なアダプタが見つからなかったので起動できない
	assert(useAdapter != nullptr);

	//機能レベルとログ出力用の文字列
	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_12_2,D3D_FEATURE_LEVEL_12_1,D3D_FEATURE_LEVEL_12_0
	};

	const char* featureLevelStrings[] = { "12.2","12.1","12.0" };

	// デバイス生成
	//高い順に生成できるか試していく
	for (size_t i = 0; i < _countof(featureLevels); i++)
	{
		//採用したアダプターでデバイスを生成
		hr = D3D12CreateDevice(useAdapter.Get(), featureLevels[i], IID_PPV_ARGS(&device));

		// 指定した機能レベルでデバイスを生成出来たかを確認
		if (SUCCEEDED(hr))
		{
			//生成できたのでログ出力を行ってループを抜ける
			Logger::Log(std::format("FeatureLevel :{}\n", featureLevelStrings[i]));
			break;
		}
	}

	//デバイスの生成がうなくいかなかったので起動できない
	assert(device != nullptr);
	Logger::Log("Complete create D3D12Device!!!\n");// 初期化完了のログを出す

	// エラー時に止める
#ifdef _DEBUG
	//ID3D12InfoQueue* infoQueue = nullptr;
	Microsoft::WRL::ComPtr< ID3D12InfoQueue> infoQueue = nullptr;
	//Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue = nullptr;
	if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue))))
	{
		// ヤバいエラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);

		// エラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);

		// 警告時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

		// 抑制するメッセージのID
		D3D12_MESSAGE_ID denyIds[] =
		{
			// Windows11でのDXGIデバッグレイヤーとDX12デバッグレイヤーの相互作用によるエラー

			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
		};

		// 抑制するレベル
		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;

		// 指定したメッセージの表示を抑制する
		infoQueue->PushStorageFilter(&filter);
	}
#endif
}

void DirectXCommon::InitializeCommands()
{
	HRESULT hr;

	//コマンドキューを生成する
    D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
	assert(SUCCEEDED(hr));

	//コマンドアロケータを生成
	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	assert(SUCCEEDED(hr));

	//コマンドリストを生成する
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));
	assert(SUCCEEDED(hr));
}

void DirectXCommon::CreateSwapChain()
{
	HRESULT hr;

	// スワップチェーンを生成する

	swapChainDesc.Width = WinApp::kClientWidth;     // 画面の幅。ウィンドウのクライアント領域を同じものにしておく
	swapChainDesc.Height = WinApp::kClientHeight;   // 画面の高さ。ウィンドウのクライアント領域を同じものにしておく
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;  //色の形成
	swapChainDesc.SampleDesc.Count = 1; //マルチサンプルしない
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // 画面のターゲットとして利用する
	swapChainDesc.BufferCount = 2;  //ダブルバッファ
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;  //モニタにうつしたら、中身を破棄

	// コマンドキュー、ウィンドウハンドル、設定を渡して生成する
	hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue.Get(), winApp->GetHWND(), &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain.GetAddressOf()));
	assert(SUCCEEDED(hr));
}

void DirectXCommon::CreateDepthBuffer()
{
	// DepthStencilTextureをウィンドウのサイズで作成
    depthStencilResource = CreateDepthStencilTextureResource(device.Get(), WinApp::kClientWidth, WinApp::kClientHeight);
}

void DirectXCommon::CreateDescriptorHeaps()
{
   // descriptorSizeSRV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	descriptorSizeRTV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	descriptorSizeDSV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	// ディスクリプタヒープの生成
	rtvDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 5, false);

	// SRV用のヒープでディスクリプタの数は１２８。SRVはShader内でさわるものなので、ShaderVisibleはfalse
	//srvDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, kMaxSRVCount, true);

	// DSV用のヒープでディスクリプタの数は1。DSVはShader内で触るものではないので、ShaderVisibleはfalse
	dsvDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 2, false);

}

// レンダーターゲットビューの初期化

inline void DirectXCommon::InitializeRenderTergetView()
{
	HRESULT hr;

	// SwapChainからResourceを引っ張ってくる
	hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResources[0]));

	// うまく取得出来なければ起動出来ない
	assert(SUCCEEDED(hr));

	hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1]));

	// うまく取得出来なければ起動出来ない
	assert(SUCCEEDED(hr));

	// RTVの設定
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;  //出力結果をSRGBに変換して書き込む
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D; // 2dテクスチャーとして書き込む
	// ディスクリプタの先頭を取得する
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	// RTVを２つ作るのでディスクリプタを２つ用意
	rtvHandles[2];///////////////

	// まず１つ目を作る
	rtvHandles[0] = rtvStartHandle;
	device->CreateRenderTargetView(swapChainResources[0].Get(), &rtvDesc, rtvHandles[0]);

	swapChainResources[0]->SetName(L"swapChainresource0");

	rtvHandles[1].ptr = rtvHandles[0].ptr + device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	device->CreateRenderTargetView(swapChainResources[1].Get(), &rtvDesc, rtvHandles[1]);
	swapChainResources[1]->SetName(L"swapChainresource1");
}

void DirectXCommon::InitializeDepthStencilView()
{
	// DSVの設定
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // Format。基本的にはResourceに合わせる
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D; // 2dTexture
	// DSVHeapの先頭にDSVを作る
	device->CreateDepthStencilView(depthStencilResource.Get(), &dsvDesc, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	//=====================
	///追加
	

}

void DirectXCommon::InitializeFence()
{
	HRESULT hr;

	// 初期値0でFenceを作る
	fenceValue = 0;
	hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	assert(SUCCEEDED(hr));

	// FenceのSignalを持つためのイベントを作成する
	fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(fenceEvent != nullptr);
}

void DirectXCommon::InitializeViewPort()
{
	// ビューポート
	// クライアント領域のサイズと一緒にして画面全体に表示
	viewport.Width = WinApp::kClientWidth;
	viewport.Height = WinApp::kClientHeight;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
}

void DirectXCommon::Scissor()
{
	// シーザー矩形
	// 基本的にビューポートと同じ矩形が構成されるようにする
	scissorRect.left = 0;
	scissorRect.right = WinApp::kClientWidth;
	scissorRect.top = 0;
	scissorRect.bottom = WinApp::kClientHeight;
}

void DirectXCommon::CreateDXCCompiler()
{
	HRESULT hr;

	// dxCompilerを初期化
	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
	assert(SUCCEEDED(hr));

	// 現時点でincludeはしていないが、includeに対応するための設定をしておく
	hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
	assert(SUCCEEDED(hr));
}

void DirectXCommon::InitializeImGui()
{
	//// ImGuiの初期化
	//IMGUI_CHECKVERSION();
	//ImGui::CreateContext();
	//ImGui::StyleColorsDark();
	//ImGui_ImplWin32_Init(winApp->GetHWND());
	//ImGui_ImplDX12_Init(device.Get(), swapChainDesc.BufferCount,
	//	rtvDesc.Format,
	//	srvDescriptorHeap.Get(),
	//	srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
	//	srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
}

Microsoft::WRL::ComPtr<ID3D12Resource> DirectXCommon::CreateRenderTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, uint32_t width, uint32_t height, DXGI_FORMAT format, const Vector4& clearColor, D3D12_RESOURCE_STATES resourceStates)
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
		resourceStates,
		&clearValue,
		IID_PPV_ARGS(&resource));

	return resource;
}

void DirectXCommon::CreateRenderTextureRTV()
{
	const Vector4 kRenderTargetClearValue{ 1.0f,0.0f,0.0f,1.0f };
	renderTextureResources[0] = CreateRenderTextureResource(
		device,
		WinApp::kClientWidth,
		WinApp::kClientHeight,
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
		kRenderTargetClearValue,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	rtvHandles[2].ptr = rtvHandles[1].ptr + device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	device->CreateRenderTargetView(renderTextureResources[0].Get(), &rtvDesc, rtvHandles[2]);
	assert(renderTextureResources[0]);
	renderTextureResources[0]->SetName(L"renderTexture0");

	renderTextureResources[1] = CreateRenderTextureResource(
		device,
		WinApp::kClientWidth,
		WinApp::kClientHeight,
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
		kRenderTargetClearValue,
		D3D12_RESOURCE_STATE_RENDER_TARGET);

	rtvHandles[3].ptr = rtvHandles[2].ptr + device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	device->CreateRenderTargetView(renderTextureResources[1].Get(), &rtvDesc, rtvHandles[3]);
	assert(renderTextureResources[1]);
	renderTextureResources[1]->SetName(L"renderTexture1");

	renderResourceIndex_ = 0;
	renderTargetIndex_ = 1;
}

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DirectXCommon::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible)
{
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = heapType;
	descriptorHeapDesc.NumDescriptors = numDescriptors;
	descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
	assert(SUCCEEDED(hr));
	return descriptorHeap;
}

Microsoft::WRL::ComPtr<ID3D12Resource> DirectXCommon::CreateDepthStencilTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, int32_t width, int32_t height)
{
	{
		D3D12_RESOURCE_DESC resourceDesc{};
		resourceDesc.Width = width;        // textureの幅
		resourceDesc.Height = height;      // textureの高さ
		resourceDesc.MipLevels = 1;        // mipmapの数
		resourceDesc.DepthOrArraySize = 1; // 奥行 or 配列Textureの配列数
		//resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // DepthStencilとして利用可能なフォーマット
		resourceDesc.Format = DXGI_FORMAT_R24G8_TYPELESS; // SRVとDSVで使えるフォーマット
		resourceDesc.SampleDesc.Count = 1; // サンプリングカウント。1固定。
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; // 2次元
		//resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL; // DepthStrncilとして使う通知
		// 修正前
		// resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE; // SRVとしても使う

		// 修正後
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_NONE; // SRVとしても使う

		// 利用するHeapの設定
		D3D12_HEAP_PROPERTIES heapProperties{};
		heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; // VRAM上に作る

		// 深度値のクリア設定
		D3D12_CLEAR_VALUE depthClearValue{};
		depthClearValue.DepthStencil.Depth = 1.0f; // 1.0f(最大値)でクリア
		depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // フォーマット。Resourceと合わせる

		// Resourceの生成
		//ID3D12Resource* resource = nullptr;
		Microsoft::WRL::ComPtr<ID3D12Resource> resource;
		HRESULT hr = device->CreateCommittedResource(
			&heapProperties, // Heapの設定
			D3D12_HEAP_FLAG_NONE, // Heapの特殊な設定。特になし
			&resourceDesc, // Resourceの設定
			D3D12_RESOURCE_STATE_DEPTH_WRITE, // 深度値を書き込む状態にしておく
			&depthClearValue, // Clear最適
			IID_PPV_ARGS(&resource));
		assert(SUCCEEDED(hr));

        
		return resource;
	}
}

void DirectXCommon::RenderTexturePreDraw()
{
	D3D12_RESOURCE_BARRIER barrier{};

	// TransitionBarrierの設定
	// 今回のバリアはTransition
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;

	// NONEにしておく
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

	uint32_t renderTargetIndex = 2 + renderResourceIndex_;

	// バリアを張る対象のリソース。現在のバックバッファに対して行う
	barrier.Transition.pResource = renderTextureResources[renderResourceIndex_].Get();

	// 還移前(現在)のResourceState
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

	// 還移後のResourceState
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

	// TransitionBarrierを張る
	DirectXCommon::GetInstance()->GetCommandList()->ResourceBarrier(1, &barrier);

	// DSV設定
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = GetDsvHandle();

	DirectXCommon::GetInstance()->GetCommandList()->OMSetRenderTargets(1, DirectXCommon::GetInstance()->GetRTVHandle(renderTargetIndex), false, &dsvHandle);

	// 指定した色で画面全体をクリアする
	float clearColor[] = { 1.0f,0.0f,0.0f,1.0f }; // 青っぽい色 RGBAの順
	DirectXCommon::GetInstance()->GetCommandList()->ClearRenderTargetView(*DirectXCommon::GetInstance()->GetRTVHandle(renderTargetIndex), clearColor, 0, nullptr);

	// 画面全体の深度をクリア
	DirectXCommon::GetInstance()->GetCommandList()->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);


	DirectXCommon::GetInstance()->GetCommandList()->RSSetViewports(1, &*DirectXCommon::GetInstance()->GetViewPort()); // Viewportを設定
	DirectXCommon::GetInstance()->GetCommandList()->RSSetScissorRects(1, &*DirectXCommon::GetInstance()->GetRect()); // Scissorを設定
}

void DirectXCommon::RenderTexturePostDraw()
{
	D3D12_RESOURCE_BARRIER barrier{};

	// TransitionBarrierの設定
	// 今回のバリアはTransition
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;

	// NONEにしておく
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

	// バリアを張る対象のリソース。現在のバックバッファに対して行う
	barrier.Transition.pResource = renderTextureResources[renderResourceIndex_].Get();

	// 還移前(現在)のResourceState
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;

	// 還移後のResourceState
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

	// TransitionBarrierを張る
	DirectXCommon::GetInstance()->GetCommandList()->ResourceBarrier(1, &barrier);
}

void DirectXCommon::PreDraw()
{
	// これから書き込むバックバッファのインデックスを取得
	UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();

	D3D12_RESOURCE_BARRIER barrier{};

	// TransitionBarrierの設定
	// 今回のバリアはTransition
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;

	// NONEにしておく
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

	// バリアを張る対象のリソース。現在のバックバッファに対して行う
	barrier.Transition.pResource = swapChainResources[backBufferIndex].Get();

	// 還移前(現在)のResourceState
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;

	// 還移後のResourceState
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

	// TransitionBarrierを張る
	commandList->ResourceBarrier(1, &barrier);

	// 描画先のRTVを設定する
	commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, nullptr);

	//// DSV設定
	//commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, &dsvHandle);

	//// 指定した色で画面全体をクリアする
	//float clearColor[] = { 0.1f,0.25,0.5f,1.0f }; // 青っぽい色 RGBAの順
	//commandList->ClearRenderTargetView(rtvHandles[backBufferIndex], clearColor, 0, nullptr);

	//// 画面全体の深度をクリア
	//commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// SRV用のデスクリプタヒープを指定
	//Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>  descriptorHeaps[] = { srvDescriptorHeap.Get() };
	//commandList->SetDescriptorHeaps(1, descriptorHeaps->GetAddressOf());

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	commandList->RSSetViewports(1, &viewport); // Viewportを設定
	commandList->RSSetScissorRects(1, &scissorRect); // Scissorを設定

	commandList->SetGraphicsRootSignature(rootSignature.Get());
	commandList->SetPipelineState(graphicsPipelineState.Get());

	D3D12_GPU_DESCRIPTOR_HANDLE srvHandle;

	if (renderResourceIndex_ == 0)
	{
		srvHandle = TextureManager::GetInstance()->GetSrvHandleGPU("RenderTexture0");
	}
	else
	{
		srvHandle = TextureManager::GetInstance()->GetSrvHandleGPU("RenderTexture1");
	}
	
	// srvHandle.ptr が 0 または異常な値でないか確認
	assert(srvHandle.ptr != 0);

	commandList->SetGraphicsRootDescriptorTable(0, srvHandle);

	commandList->DrawInstanced(3, 1, 0, 0);
}

void DirectXCommon::PostDraw()
{
	HRESULT hr;

	// これから書き込むバックバッファのインデックスを取得
	UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();

	D3D12_RESOURCE_BARRIER barrier{};

	// TransitionBarrierの設定
	// 今回のバリアはTransition
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;

	// NONEにしておく
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

	// バリアを張る対象のリソース。現在のバックバッファに対して行う
	barrier.Transition.pResource = swapChainResources[backBufferIndex].Get();

	// 画面に描く処理は終わり、画面に映すので、状態を還移
	// 今回はRenderTargetからPresentにする
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

	// TransitionBarrierを張る
	commandList->ResourceBarrier(1, &barrier);


	//// 現在書き込んだ方を読み込み用に変換
	//D3D12_RESOURCE_BARRIER barrier1{};
	//barrier1.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	//barrier1.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	//barrier1.Transition.pResource = renderTextureResources[renderTargetIndex_].Get();
	//barrier1.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	//barrier1.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

	//commandList->ResourceBarrier(1, &barrier1);

	//// 今読み込んだ方を描画用に変換
	//D3D12_RESOURCE_BARRIER barrier2{};
	//barrier2.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	//barrier2.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	//barrier2.Transition.pResource = renderTextureResources[renderResourceIndex_].Get();
	//barrier2.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	//barrier2.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

	//commandList->ResourceBarrier(1, &barrier2);

	//std::swap(renderTargetIndex_, renderResourceIndex_);

	// コマンドリストの内容を確定させる。すべてのコマンドを積んでからCloseすること
	hr = commandList->Close();
	assert(SUCCEEDED(hr));

	// GPUにコマンドリストの実行を行わせる
	Microsoft::WRL::ComPtr<ID3D12CommandList> commandLists[] = { commandList.Get() };
	commandQueue->ExecuteCommandLists(1, commandLists->GetAddressOf());

	// GPUがここまでたどり着いたときに、Fenceの値を指定した値に代入するようにSignalを送る
	commandQueue->Signal(fence.Get(), ++fenceValue);

	// GPUの作業が完了するのを待つ
	if (fence->GetCompletedValue() < fenceValue)
	{
		HANDLE event = CreateEvent(nullptr, false, false, nullptr);
		// 指定したSignalにたどり着いてないので、たどり着くまで待つようにイベントを設定する
		fence->SetEventOnCompletion(fenceValue, event);

		// イベントを待つ
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);
	}

	// GPUとOSに画面の交換を行うように通知する
	swapChain->Present(1, 0);

	// 次のフレーム用のコマンドリストを準備
	hr = commandAllocator->Reset();
	assert(SUCCEEDED(hr));
	hr = commandList->Reset(commandAllocator.Get(), nullptr);
	assert(SUCCEEDED(hr));

	UpdateFixFPS();
}

Microsoft::WRL::ComPtr<IDxcBlob> DirectXCommon::CompileShader(const std::wstring& filePath, const wchar_t* profile)
{
	// これからシェーダーをコンパイル
	//Log(ConvertString(std::format(L"Begin CompileShader, path:{}, profile:{}\n", filePath, profile)));

	// hlslファイルを読み込む
	Microsoft::WRL::ComPtr<IDxcBlobEncoding> shaderSource = nullptr;
	HRESULT hr = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);

	// 読めなかったら止める
	assert(SUCCEEDED(hr));

	// 読み込んだファイルの内容を設定する
	DxcBuffer shaderSourceBuffer;
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8; // UTFの文字コードであることを通知

	LPCWSTR arguments[] =
	{
		filePath.c_str(), // コンパイル対象のhlslファイル名
		L"-E",L"main", // エントリーポイントの指定。基本的にmain以外にはしない
		L"-T", profile, // ShaderProfileの設定
		L"-Zi", L"-Qembed_debug", // デバッグ用の情報を埋め込む
		L"-Od",    // 最適化しておく
		L"-Zpr",   // メモリレイアウトは最優先
	};

	// 実際にShaderをコンパイルする
	Microsoft::WRL::ComPtr<IDxcResult> shaderResult = nullptr;
	hr = dxcCompiler.Get()->Compile(
		&shaderSourceBuffer,
		arguments,
		_countof(arguments),
		includeHandler.Get(),
		IID_PPV_ARGS(&shaderResult)
	);

	assert(shaderResult != nullptr);

	// コンパイルエラーでなくdxcが起動出来ないなど致命的な状況
	assert(SUCCEEDED(hr));

	// 警告・エラーが出たらログを出して止める
	Microsoft::WRL::ComPtr<IDxcBlobUtf8> shaderError = nullptr;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0)
	{
		Logger::Log(shaderError->GetStringPointer());

		// 警告・エラー
		assert(false);
	}

	// コンパイル結果から実行のバイナリ部分を取得
	Microsoft::WRL::ComPtr<IDxcBlob> shaderBlob = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr));

	// 成功したログを出す
	//Log(ConvertString(std::format(L"Compile Succeeded, path:{},profile:{}\n", filePath, profile)));

	// もう使わないリソースを解放
	//shaderSource->Release(); /// エラーの原因の可能性
	//shaderResult->Release(); /// エラーの原因の可能性

	// 実行用のバイナリを返却
	return shaderBlob;

}

Microsoft::WRL::ComPtr<ID3D12Resource> DirectXCommon::CreateBufferResource(size_t sizeInBytes)
{
	//ID3D12Resource* resultResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> resultResource;
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	D3D12_RESOURCE_DESC vertexBufferDesc{};
	vertexBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vertexBufferDesc.Width = sizeInBytes;
	vertexBufferDesc.Height = 1;
	vertexBufferDesc.DepthOrArraySize = 1;
	vertexBufferDesc.MipLevels = 1;
	vertexBufferDesc.SampleDesc.Count = 1;
	vertexBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &vertexBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&resultResource));
	return resultResource;
}

Microsoft::WRL::ComPtr<ID3D12Resource> DirectXCommon::CreateTextureResource(const DirectX::TexMetadata& metadata)
{
	// metadataを基にResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = UINT(metadata.width); // Textureの幅
	resourceDesc.Height = UINT(metadata.height); // Textureの高さ
	resourceDesc.MipLevels = UINT16(metadata.mipLevels); // mipmapの数
	resourceDesc.DepthOrArraySize = UINT16(metadata.arraySize); // 奥行 or 配列Textureの配列数
	resourceDesc.Format = metadata.format; // TextureのFormat
	resourceDesc.SampleDesc.Count = 1; // サンプルカウント。1固定
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension); // Textureの次元数

	// 利用するHeapの設定。
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_CUSTOM; // 細かい設定を行う
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK; // WRITEBACKポリシーでCPUにアクセス
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0; // プロセッサの近くに配置

	// Resourceの生成
	//ID3D12Resource* resource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));
	return resource;
}

void DirectXCommon::UploadTextureData(Microsoft::WRL::ComPtr<ID3D12Resource> texture, const DirectX::ScratchImage& mipImages)
{
	// meta情報を取得
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();

	// MipMapについて
	for (size_t mipLevel = 0; mipLevel < metadata.mipLevels; ++mipLevel)
	{
		// mipMapLevelを指定して各Imageを取得
		const DirectX::Image* img = mipImages.GetImage(mipLevel, 0, 0);

		// Textureに転送
		HRESULT hr = texture->WriteToSubresource(
			UINT(mipLevel),
			nullptr,              // 全領域へコピー
			img->pixels,          // 元データアドレス
			UINT(img->rowPitch),  // 1ラインサイズ
			UINT(img->slicePitch) // 1枚サイズ
		);
		assert(SUCCEEDED(hr));
	}

}

DirectX::ScratchImage DirectXCommon::LoadTexture(const std::string& filePath)
{
	// テクスチャファイルを読んでプログラムで扱えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathW = StringUtility::ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));

	// ミニマップの生成
	DirectX::ScratchImage mipImages{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	assert(SUCCEEDED(hr));

	// ミニマップ付きデータを返す
	return mipImages;
}

void DirectXCommon::InitializeFixFPS()
{
	// 現在時間を記録
	reference_ = std::chrono::steady_clock::now();
}

void DirectXCommon::UpdateFixFPS()
{
	// 1/60秒ぴったりの時間
	const std::chrono::microseconds kMinTime(uint64_t(1000000.0f / 60.0f));

	const std::chrono::microseconds kMinCheckTime(uint64_t(1000000.0f / 65.0f));

	// 現在時間を取得する
	std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

	// 前回記録からの経過時間を取得する
	std::chrono::microseconds elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - reference_);

	// 1/60秒(よりわずかに短い時間)経っていない場合
	if (elapsed < kMinCheckTime) {
		// 1/60秒経過するまで微小なスリープを繰り返す
		while (std::chrono::steady_clock::now() - reference_ < kMinTime) {
			// 1マイクロ秒スリープ
			std::this_thread::sleep_for(std::chrono::microseconds(1));
		}
	}
	// 現在の時間を記録する
	reference_ = std::chrono::steady_clock::now();
}

void DirectXCommon::CreateOffScreenRootSignature()
{
	HRESULT hr;

	// ルートシグネチャの設定
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// SRV の Descriptor Range
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0; // t0: Shader Register
	descriptorRange[0].NumDescriptors = 1; // 1つのSRV
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // SRV
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // 自動計算

	// Root Parameter: SRV (gTexture)
	D3D12_ROOT_PARAMETER rootParameters[1] = {};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // Pixel Shaderで使用
	//rootParameters[0].DescriptorTable.NumDescriptorRanges = 1; // 1つの範囲
	//rootParameters[0].DescriptorTable.pDescriptorRanges = &descriptorRange; // SRV の範囲

	rootParameters[0].DescriptorTable.pDescriptorRanges = descriptorRange; // Tableの中身の配列を指定
	rootParameters[0].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange); // Tableで利用する数


	// Static Sampler
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR; // バイリニアフィルタ
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX; // 全MipMap使用
	staticSamplers[0].ShaderRegister = 0; // s0: Shader Register
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // Pixel Shaderで使用

	// ルートシグネチャの構築
	descriptionRootSignature.pParameters = rootParameters; // ルートパラメーター配列へのポインタ
	descriptionRootSignature.NumParameters = _countof(rootParameters); // 配列の長さ
	descriptionRootSignature.pStaticSamplers = staticSamplers; // サンプラー配列へのポインタ
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers); // サンプラーの数

	//シリアライズしてバイナリにする

	hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr))
	{
		Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}

	// バイナリを元に生成
	rootSignature = nullptr;
	hr = device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));
}

void DirectXCommon::CreateOffScreenPipeLine()
{
	CreateOffScreenRootSignature();

	D3D12_INPUT_ELEMENT_DESC inputElementDescs[2] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = nullptr;
	inputLayoutDesc.NumElements = 0;

	// BlendStateの設定
	D3D12_BLEND_DESC blendDesc{};

	// すべての要素数を書き込む
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	// RasiterzerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};

	// 裏面(時計回り)を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;

	// 三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	// Shaderをコンパイルする
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = CompileShader(L"Resources/shaders/Fullscreen.VS.hlsl", L"vs_6_0");
	assert(vertexShaderBlob != nullptr);

	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = CompileShader(L"Resources/shaders/CopyImage.PS.hlsl", L"ps_6_0");
	assert(pixelShaderBlob != nullptr);

	// DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};

	// Depthの機能を有効化する
	depthStencilDesc.DepthEnable = false;


	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature.Get(); // RootSignature
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;  // InputLayout
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),vertexShaderBlob->GetBufferSize() }; // VertexShader
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),pixelShaderBlob->GetBufferSize() };   // PixelShader
	graphicsPipelineStateDesc.BlendState = blendDesc; //BlendState
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc; // RasterizerState

	// 書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	// 利用するトポロジ(形状)のタイプ.。三角形
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	// どのように画面に色をつけるか
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	// DepthStencilの設定
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	// 実際に生成
	graphicsPipelineState = nullptr;
	device->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState));
}
