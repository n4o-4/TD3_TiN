#include "Dissolve.h"

void Dissolve::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager)
{
	// パイプラインの生成
	BaseEffect::Initialize(dxCommon, srvManager);

	// テクスチャの読み込み
	TextureManager::GetInstance()->LoadTexture("Resources/noise0.png");

	//パイプラインの初期化
	CreatePipeline();

	// マテリアルの生成
	CreateMaterial();


    edgeColor = (1.0f,0.3f,0.25f);

	threshold = 0.3f;

    thresholdWidth= 0.03f;

	data_->threshold = 0.3f;
	data_->thresholdWidth = 0.03f;
	data_->edgeColor = (1.0f, 0.3, 0.25f);
}

void Dissolve::Update()
{

	data_->threshold = threshold;
	data_->thresholdWidth = thresholdWidth;
	data_->edgeColor = edgeColor;

}

void Dissolve::Draw(uint32_t renderTargetIndex, uint32_t renderResourceIndex)
{
	// 描画先のRTVのインデックス
	uint32_t renderTextureIndex = 2 + renderTargetIndex;

	// 描画先のRTVを設定する
	dxCommon_->GetCommandList()->OMSetRenderTargets(1, &*dxCommon_->GetRTVHandle(renderTextureIndex), false, nullptr);

	// 指定した色で画面全体をクリアする
	float clearColor[] = { 1.0f,0.0f,0.0f,1.0f }; // 青っぽい色 RGBAの順
	dxCommon_->GetInstance()->GetCommandList()->ClearRenderTargetView(*DirectXCommon::GetInstance()->GetRTVHandle(renderTextureIndex), clearColor, 0, nullptr);

	// ルートシグネチャの設定	
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(pipeline_.get()->rootSignature.Get());

	// パイプラインステートの設定
	dxCommon_->GetCommandList()->SetPipelineState(pipeline_.get()->pipelineState.Get());

	// renderTextureのSrvHandleを取得
	D3D12_GPU_DESCRIPTOR_HANDLE srvHandle = (renderResourceIndex == 0) ? TextureManager::GetInstance()->GetSrvHandleGPU("RenderTexture0") : TextureManager::GetInstance()->GetSrvHandleGPU("RenderTexture1");

	// SRVを設定
	dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(0, srvHandle);

	// maskTextureのSrvHandleを取得
	dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(1, TextureManager::GetInstance()->GetSrvHandleGPU("Resources/noise0.png"));

	// Cbufferの設定
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(2, resource_.Get()->GetGPUVirtualAddress());
	
	// 描画
	dxCommon_->GetCommandList()->DrawInstanced(3, 1, 0, 0);


	///
}

void Dissolve::CreatePipeline()
{
	// ルートシグネチャの生成
	CreateRootSignature(pipeline_.get());

	// パイプラインステートの生成
	CreatePipeLineState(pipeline_.get());
}

void Dissolve::CreateRootSignature(Pipeline* pipeline)
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

	// SRV の Descriptor Range
	D3D12_DESCRIPTOR_RANGE descriptorRange1[1] = {};
	descriptorRange1[0].BaseShaderRegister = 1; // t0: Shader Register
	descriptorRange1[0].NumDescriptors = 1; // 1つのSRV
	descriptorRange1[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // SRV
	descriptorRange1[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // 自動計算


	// Root Parameter: SRV (gTexture)
	D3D12_ROOT_PARAMETER rootParameters[3] = {};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // Pixel Shaderで使用
	rootParameters[0].DescriptorTable.pDescriptorRanges = descriptorRange; // Tableの中身の配列を指定
	rootParameters[0].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange); // Tableで利用する数

	// Root Parameter: SRV (gMaskTexture)
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // Pixel Shaderで使用
	rootParameters[1].DescriptorTable.pDescriptorRanges = descriptorRange1; // Tableの中身の配列を指定
	rootParameters[1].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange1); // Tableで利用する数

	// Root Parameter: CBV
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[2].Descriptor.ShaderRegister = 0;

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

void Dissolve::CreatePipeLineState(Pipeline* pipeline)
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
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

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

	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = dxCommon_->CompileShader(L"Resources/shaders/Dissolve.PS.hlsl", L"ps_6_0");
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

void Dissolve::CreateMaterial()
{
	// bufferResourceの生成
	resource_ = dxCommon_->CreateBufferResource(sizeof(DissolveShader::Material));

	// データをマップ
	resource_.Get()->Map(0, nullptr, reinterpret_cast<void**>(&data_));
}
