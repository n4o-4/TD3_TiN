/*********************************************************************
* \file   LineSetup.cpp
* \brie
* 
* \author Harukichimaru
* \date   January 2025
* \note   
*********************************************************************/
#include "LineSetup.h"

///=============================================================================
///						初期化
void LineSetup::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager) {
	//========================================
	// DirectXCommonの取得
	dxCommon_ = dxCommon;

	//========================================
	// SrvSetupの取得
	srvManager_ = srvManager;

	//========================================
	// ルートシグネチャの作成
	CreateGraphicsPipeline();
}

///=============================================================================
///						共通化処理
void LineSetup::CommonDrawSetup() {
	//コマンドリストの取得
	// NOTE:Getを複数回呼び出すのは非効率的なので、変数に保持しておく
	auto commandList = dxCommon_->GetCommandList();
	//ルートシグネイチャのセット
	commandList->SetGraphicsRootSignature(rootSignature_.Get());
	//グラフィックスパイプラインステートをセット
	commandList->SetPipelineState(graphicsPipelineState_.Get());
	//プリミティブトポロジーをセットする(Line用にLINELISTに変更)
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
}

///=============================================================================
///						ルートシグネチャーの作成
void LineSetup::CreateRootSignature() {
	// ルートパラメータの設定
	D3D12_ROOT_PARAMETER rootParameters[1] = {};

	// 定数バッファ（TransformationMatrix）の設定（b0、頂点シェーダーで使用）
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[0].Descriptor.ShaderRegister = 0; // b0
	rootParameters[0].Descriptor.RegisterSpace = 0;

	// ルートシグネチャの設定
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.NumParameters = _countof(rootParameters);
	rootSignatureDesc.pParameters = rootParameters;
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// シリアライズとルートシグネチャの作成
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		throw std::runtime_error(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
	}

	hr = dxCommon_->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to create root signature");
	}
}


///=============================================================================
///						グラフィックスパイプラインの作成
void LineSetup::CreateGraphicsPipeline() {
	//========================================
	// RoorSignatureの作成
	CreateRootSignature();

	//========================================
	// InputElementの設定
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[2] = {};
	// 位置データ
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[0].InputSlot = 0;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[0].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
	inputElementDescs[0].InstanceDataStepRate = 0;
	// カラーデータ
	inputElementDescs[1].SemanticName = "COLOR";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[1].InputSlot = 0;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[1].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
	inputElementDescs[1].InstanceDataStepRate = 0;

	//========================================
	// InputLayoutの設定を行う
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	//========================================
	// BlendStateの設定を行う
	D3D12_BLEND_DESC blendDesc{};
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	//========================================
	// RasterizerStateの設定を行う
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

    rasterizerDesc.DepthBias = 100;             // 深度バイアスを設定
    rasterizerDesc.DepthBiasClamp = 0.0f;       // バイアスの最大値
    rasterizerDesc.SlopeScaledDepthBias = 1.0f; // 傾斜によるバイアスの倍率

	//========================================
	// VertexShaderをコンパイルする
	Microsoft::WRL::ComPtr <IDxcBlob> vertexShaderBlob = dxCommon_->CompileShader(L"externals/MagEngine/Line/Line.VS.hlsl", L"vs_6_0");
	if(!vertexShaderBlob) {
		throw std::runtime_error("Particle Failed to compile vertex shader :(");
	}
	//========================================
	// PixelShaderをコンパイルする
	Microsoft::WRL::ComPtr <IDxcBlob> pixelShaderBlob = dxCommon_->CompileShader(L"externals/MagEngine/Line/Line.PS.hlsl", L"ps_6_0");
	if(!pixelShaderBlob) {
		throw std::runtime_error("Particle Failed to compile pixel shader :(");
	}

	//========================================
	// PSOを生成する
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature_.Get();
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };
	graphicsPipelineStateDesc.BlendState = blendDesc;
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	//========================================
    // DepthStencilStateの設定を行う
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = false;  // デプスステンシルを有効
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL; 
    graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	//========================================
	// 実際に生成
	HRESULT hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc,
		IID_PPV_ARGS(&graphicsPipelineState_));
	if(FAILED(hr)) {
		throw std::runtime_error("Particle Failed to create graphics pipeline state :(");
	}
}
