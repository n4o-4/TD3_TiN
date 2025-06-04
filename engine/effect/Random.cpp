#include "Random.h"

void Random::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager)
{
	// パイプラインの生成
	BaseEffect::Initialize(dxCommon, srvManager);

	//パイプラインの初期化
	CreatePipeline();

	// マテリアルの生成
	CreateMaterial();

	std::random_device seedGenerator;
	randomEngine.seed(seedGenerator());
}

void Random::Update()
{
	// ランダムな値を生成
	std::uniform_real_distribution<float> distribution(0.0f, 1.0f);

	// データの設定
	data_->time = distribution(randomEngine);
}

void Random::Draw(uint32_t renderTargetIndex, uint32_t renderResourceIndex)
{
	// 描画先のRTVのインデックス
	uint32_t renderTextureIndex = 2 + renderTargetIndex;

	// 描画先のRTVを設定する
	dxCommon_->GetCommandList()->OMSetRenderTargets(1, &*dxCommon_->GetRTVHandle(renderTextureIndex), false, nullptr);

	// ルートシグネチャの設定	
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(pipeline_.get()->rootSignature.Get());

	// パイプラインステートの設定
	dxCommon_->GetCommandList()->SetPipelineState(pipeline_.get()->pipelineState.Get());

	// 定数バッファの設定	
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(0, resource_.Get()->GetGPUVirtualAddress());

	// 描画
	dxCommon_->GetCommandList()->DrawInstanced(3, 1, 0, 0);
}

void Random::CreatePipeline()
{
	// ルートシグネチャの生成
	CreateRootSignature(pipeline_.get());

	// パイプラインステートの生成
	CreatePipeLineState(pipeline_.get());
}

void Random::CreateRootSignature(Pipeline* pipeline)
{
	HRESULT hr;

	// ルートシグネチャの設定
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// Root Parameter: CBuffer (Materia)
	D3D12_ROOT_PARAMETER rootParameters[1] = {};

	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;   //CBVを使う
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;  //PixelShaderで使う
	rootParameters[0].Descriptor.ShaderRegister = 0;    // レジスタ番号0とバインド
	// Static Sampler
	//D3D12_STATIC_SAMPLER_DESC staticSamplers[0] = {};

	// ルートシグネチャの構築
	descriptionRootSignature.pParameters = rootParameters; // ルートパラメーター配列へのポインタ
	descriptionRootSignature.NumParameters = _countof(rootParameters); // 配列の長さ
	descriptionRootSignature.pStaticSamplers = nullptr; // サンプラー配列へのポインタ
	descriptionRootSignature.NumStaticSamplers = 0; // サンプラーの数

	//シリアライズしてバイナリにする

	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;

	hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr))
	{
		Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}

	// バイナリを元に生成
	pipeline->rootSignature = nullptr;
	hr = dxCommon_->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&pipeline->rootSignature));
	assert(SUCCEEDED(hr));
}

void Random::CreatePipeLineState(Pipeline* pipeline)
{
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
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = dxCommon_->CompileShader(L"Resources/shaders/Fullscreen.VS.hlsl", L"vs_6_0");
	assert(vertexShaderBlob != nullptr);

	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = dxCommon_->CompileShader(L"Resources/shaders/Random.PS.hlsl", L"ps_6_0");
	assert(pixelShaderBlob != nullptr);

	// DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};

	// Depthの機能を有効化する
	depthStencilDesc.DepthEnable = false;


	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = pipeline->rootSignature.Get(); // RootSignature
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
	pipeline->pipelineState = nullptr;
	dxCommon_->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&pipeline->pipelineState));
}

void Random::CreateMaterial()
{
	// bufferResourceの生成
	resource_ = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(RandomShader::Material));

	// データをマップ
	resource_.Get()->Map(0, nullptr, reinterpret_cast<void**>(&data_));
}
