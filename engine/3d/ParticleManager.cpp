#include "ParticleManager.h"
#include "TextureManager.h"
#include "MyMath.h"

std::unique_ptr<ParticleManager> ParticleManager::instance = nullptr;

ParticleManager* ParticleManager::GetInstance()
{
	if (instance == nullptr) {
		instance = std::make_unique<ParticleManager>();
	}

	return instance.get();
}

void ParticleManager::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager)
{
	/*----------------------------------------------------------
	* 引数をメンバ変数に記録
	-------------------------------------------------------
	---*/

	dxCommon_ = dxCommon;
	srvManager_ = srvManager;

	accelerationField.acceleration = { 0.0f,0.0f,0.0f };
	accelerationField.area.min = { -1.0f,-1.0f,-1.0f };
	accelerationField.area.max = { 1.0f,1.0f,1.0f };

	/*----------------------------------------------------------
	* ランダムエンジンの初期化
	----------------------------------------------------------*/

	std::random_device seedGenerator;
	randomEngine.seed(seedGenerator());

	/*----------------------------------------------------------
	* Pipelineの生成関数呼び出し
	----------------------------------------------------------*/

	CreatePipeline();

	/*----------------------------------------------------------
	* 頂点データの初期化関数呼び出し
	----------------------------------------------------------*/

	InitializeVertexData();

	/*----------------------------------------------------------
	* 頂点リソースの生成関数の呼び出し
	----------------------------------------------------------*/

	CreateVertexResource();

	/*----------------------------------------------------------
	* 頂点バッファビューの生成関数の呼び出し
	----------------------------------------------------------*/

	CreateVertexBufferView();

	/*----------------------------------------------------------
	* 頂点リソースに頂点データを書き込む関数の呼び出し
	----------------------------------------------------------*/

	WriteDataInResource();
}

void ParticleManager::Update()
{

	/*----------------------------------------------------------
	* ビルボードマトリクスを計算する関数の呼び出し
	----------------------------------------------------------*/
	calculationBillboardMatrix();

	Matrix4x4 viewMatrix = cameraManager_->GetActiveCamera()->GetViewProjection().matView_;

	Matrix4x4 projectionMatrix = cameraManager_->GetActiveCamera()->GetViewProjection().matProjection_;

	for (std::unordered_map<std::string, ParticleGroup>::iterator particleGroupIterator = particleGroups.begin(); particleGroupIterator != particleGroups.end();) {

		ParticleGroup* particleGroup = &(particleGroupIterator->second);

		particleGroupIterator->second.kNumInstance = 0;

		for (std::list<Particle>::iterator particleIterator = particleGroup->particles.begin(); particleIterator != particleGroup->particles.end();) {

			if (particleGroupIterator->second.kNumInstance < kNumMaxInstance) {

				if ((*particleIterator).lifeTime <= (*particleIterator).currentTime) {
					particleIterator = particleGroup->particles.erase(particleIterator);
					continue;
				}

				if (IsCollision(accelerationField.area, (*particleIterator).transform.translate)) {
					(*particleIterator).velocity += accelerationField.acceleration * kDeltaTime;
				}

				float lifeRatio = (*particleIterator).currentTime / (*particleIterator).lifeTime;

				// scale
				Vector3 baseScale = Lerp((*particleIterator).startScale, (*particleIterator).finishScale, lifeRatio);
				
				if (particleGroup->enablePulse)
				{
					float t = (*particleIterator).currentTime / (*particleIterator).lifeTime;
					t = std::clamp(t, 0.0f, 1.0f);

					// 揺れを始めるのは寿命の80%以降
					float expandStartT = 0.65f;

					if (t >= expandStartT) {
						float localT = (t - expandStartT) / (1.0f - expandStartT);  // 0〜1

						float frequency = 6.0f;  // 揺れの回数
						float amplitude = 1.0f;  // 最大振幅（scaleが0まで下がる）

						// sin波（0〜±1）で揺れ
						float wave = sinf(localT * frequency * 2.0f * 3.14159f);  // -1〜1

						// 0〜1で上下するようにする（絶対値を取る）
						float scaleFactor = 1.0f - fabsf(wave) * amplitude;

						// baseScale に掛ける
						(*particleIterator).transform.scale = baseScale * scaleFactor;
					}
					else {
						// 通常スケール
						(*particleIterator).transform.scale = baseScale;
					}
				}
				else
				{
					// 通常スケール
					(*particleIterator).transform.scale = baseScale;
				}


				// translate
				(*particleIterator).transform.translate += (*particleIterator).velocity * kDeltaTime;
				(*particleIterator).currentTime += kDeltaTime;

				// 追加

				Matrix4x4 scaleMatrix;

				scaleMatrix.m[0][0] = (*particleIterator).transform.scale.x;
				scaleMatrix.m[0][1] = 0;
				scaleMatrix.m[0][2] = 0;
				scaleMatrix.m[1][0] = 0;
				scaleMatrix.m[1][1] = (*particleIterator).transform.scale.y;
				scaleMatrix.m[1][2] = 0;
				scaleMatrix.m[2][0] = 0;
				scaleMatrix.m[2][1] = 0;
				scaleMatrix.m[2][2] = (*particleIterator).transform.scale.z;
				scaleMatrix.m[3][3] = 1;

				Matrix4x4 worldMatrix = MakeIdentity4x4();

				if (particleGroup->enableBillboard)
				{
					// スケーリング行列
					Matrix4x4 scaleMatrix = MakeScaleMatrix(particleIterator->transform.scale);
					
					//
					Matrix4x4 translateMatrix = MakeTranslateMatrix(particleIterator->transform.translate);

					// 合成
					worldMatrix = Multiply(Multiply(scaleMatrix,billboardMatrix),translateMatrix);
				}
				else
				{
					worldMatrix = MakeAffineMatrix((*particleIterator).transform.scale, (*particleIterator).transform.rotate, (*particleIterator).transform.translate);

				}

				Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));

				particleGroup->instancingData[particleGroupIterator->second.kNumInstance].World = worldMatrix;
				particleGroup->instancingData[particleGroupIterator->second.kNumInstance].WVP = worldViewProjectionMatrix;

				

				Vector4 color = Vect4::Lerp((*particleIterator).startColor, (*particleIterator).finishColor, lifeRatio);

				(*particleIterator).color = color;
				particleGroup->instancingData[particleGroupIterator->second.kNumInstance].color = (*particleIterator).color;

				if (particleGroup->enableDeceleration)
				{
					DecelerationUpdate(*particleIterator);
				}

				
				
				++particleGroupIterator->second.kNumInstance;
			}

			++particleIterator;
		}

		++particleGroupIterator;
	}
}

void ParticleManager::Draw(std::string filePath)
{
	// ルートシグネチャを設定
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(rootSignature.Get());

	// VBVを設定
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 
	for (std::unordered_map<std::string, ParticleGroup>::iterator particleGroupIterator = particleGroups.begin(); particleGroupIterator != particleGroups.end();) {
		
		// PSOを設定
		dxCommon_->GetCommandList()->SetPipelineState(sPipeLineStates_[static_cast<int>(particleGroupIterator->second.blendMode)].Get());

		// 三角色
		particleGroupIterator->second.material->color = { 1.0f, 1.0f, 1.0f, 1.0f, };
		particleGroupIterator->second.material->enableLighting = false;
		particleGroupIterator->second.material->uvTransform = MakeIdentity4x4();

		dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(0, particleGroupIterator->second.materialResource.Get()->GetGPUVirtualAddress());

		dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(1, srvManager_->GetGPUDescriptorHandle(particleGroupIterator->second.srvIndex));

		dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(2,srvManager_->GetGPUDescriptorHandle(particleGroupIterator->second.materialData.textureIndex));

		if (particleGroupIterator->second.type == ParticleType::Normal) {
			// プリミティブトポロジーを設定
			dxCommon_->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
			dxCommon_->GetCommandList()->DrawInstanced(UINT(modelData.vertices.size()), particleGroupIterator->second.kNumInstance, 0, 0);
		}
		else if (particleGroupIterator->second.type == ParticleType::Ring)
		{
			// プリミティブトポロジーを設定
			dxCommon_->GetCommandList()->IASetVertexBuffers(0, 1, &ringVertexBufferView);
			dxCommon_->GetCommandList()->DrawInstanced(UINT(ringModelData.vertices.size()), particleGroupIterator->second.kNumInstance, 0, 0);
		}
		else if (particleGroupIterator->second.type == ParticleType::Cylinder)
		{

			for (size_t i = 0; i < cylinderModelData.vertices.size(); ++i) {
				VertexData& v = cylinderModelData.vertices[i]; // ← 参照で取得
				v.texcoord.x -= 0.01f; // ← これで元のデータが更新される
			}

			VertexData* cylinderVertexData;
			cylinderVertexResource.Get()->Map(0, nullptr, reinterpret_cast<void**>(&cylinderVertexData));
			std::memcpy(cylinderVertexData, cylinderModelData.vertices.data(), sizeof(VertexData) * cylinderModelData.vertices.size());

			// プリミティブトポロジーを設定
			dxCommon_->GetCommandList()->IASetVertexBuffers(0, 1, &cylinderVertexBufferView);
			dxCommon_->GetCommandList()->DrawInstanced(UINT(cylinderModelData.vertices.size()), particleGroupIterator->second.kNumInstance, 0, 0);
		}

		++particleGroupIterator;
	}

}

void ParticleManager::CreateRootSignature()
{
	HRESULT hr;

	/*----------------------------------------------------------
	* RootSiganatureの生成
    ----------------------------------------------------------*/
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;
	descriptorRange[0].NumDescriptors = 1;
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_DESCRIPTOR_RANGE descriptorRangeForInstancing[1] = {};
	descriptorRangeForInstancing[0].BaseShaderRegister = 0;
	descriptorRangeForInstancing[0].NumDescriptors = 1;
	descriptorRangeForInstancing[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRangeForInstancing[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	/*----------------------------------------------------------
	* RootParameterの生成
	----------------------------------------------------------*/

	D3D12_ROOT_PARAMETER rootParameters[3] = {};

	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[0].Descriptor.ShaderRegister = 0;

	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[1].DescriptorTable.pDescriptorRanges = descriptorRangeForInstancing;
	rootParameters[1].DescriptorTable.NumDescriptorRanges = _countof(descriptorRangeForInstancing);

	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);

	descriptionRootSignature.pParameters = rootParameters;
	descriptionRootSignature.NumParameters = _countof(rootParameters);

	/*----------------------------------------------------------
	* Samplerの生成
	----------------------------------------------------------*/

	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR; // バイナリフィルタ
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // 0~1の範囲外をリピート
	//staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP; // X座標
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	//staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // Y座標
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER; // 比較しない
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX; // 多くのMipMapを使う
	staticSamplers[0].ShaderRegister = 0;
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

	/*----------------------------------------------------------
	* シリアライズしてバイナリにする
	----------------------------------------------------------*/

	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;

	hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);

	if (FAILED(hr)) {
		Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
	}

	/*----------------------------------------------------------
	* バイナリを元RootSignatureを生成
	----------------------------------------------------------*/

	hr = dxCommon_->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));
}

void ParticleManager::CreatePipeline()
{
	HRESULT hr;

	/*----------------------------------------------------------
	* RootSignature生成関数呼び出し
	----------------------------------------------------------*/

	CreateRootSignature();

	/*----------------------------------------------------------
	* InputElementの設定
	----------------------------------------------------------*/

	D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[2].SemanticName = "NORMAL";
	inputElementDescs[2].SemanticIndex = 0;
	inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	/*----------------------------------------------------------
	* RasterizerStateの設定
	----------------------------------------------------------*/

	D3D12_RASTERIZER_DESC rasterizerDesc{};

	// 裏面(時計回り)を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;

	// 三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	/*----------------------------------------------------------
	* Shaderのコンパイル
	----------------------------------------------------------*/

	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = dxCommon_->CompileShader(L"Resources/shaders/Particle.VS.hlsl", L"vs_6_0");

	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = dxCommon_->CompileShader(L"Resources/shaders/Particle.PS.hlsl", L"ps_6_0");

	/*----------------------------------------------------------
	* DepthStencilStateの設定
	----------------------------------------------------------*/

	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};

	// Depthの機能を有効化する
	depthStencilDesc.DepthEnable = true;

	// 書き込みします
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;

	// 比較関数はLessEqual。近ければ描画される
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;


	/*----------------------------------------------------------
	* BlendStateの設定
	----------------------------------------------------------*/

	D3D12_BLEND_DESC blendDesc{};

	// 全ての要素数を書き込む
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	

	for (int i = 0; i < static_cast<int>(BlendMode::kCountOfBlendMode); ++i)
	{
		BlendMode currentBlendMode = static_cast<BlendMode>(i);

		switch (currentBlendMode)
		{
		case BlendMode::kNone:

			blendDesc.RenderTarget[0].BlendEnable = FALSE;


			break;

		case BlendMode::kNormal:

			blendDesc.RenderTarget[0].BlendEnable = TRUE;


			blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_SRC_ALPHA;
			blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;

			blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
			blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;


			break;

		case BlendMode::kAdd:

			blendDesc.RenderTarget[0].BlendEnable = TRUE;


			blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_SRC_ALPHA;
			blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;
			//blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;

			blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
			blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
			//blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;


			///ChatGPT
			//// RGBチャンネルのブレンド（加算）
			//blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;  // ソース色にアルファを乗算
			//blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;       // 背景色をそのまま
			//blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;      // 加算合成

			//// アルファチャンネルのブレンド（通常あまり重要ではないが、同様に設定）
			//blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ZERO;
			//blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;
			//blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;


			/// Dest = 背景

			break;

		case BlendMode::kSubtract:

			blendDesc.RenderTarget[0].BlendEnable = TRUE;


			blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_SRC_ALPHA;
			blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_SUBTRACT;
			blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;

			blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
			blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;
			blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;

			break;

		case BlendMode::kMultiply:

			blendDesc.RenderTarget[0].BlendEnable = TRUE;


			// カラーの乗算設定
			blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_DEST_COLOR;  // デスティネーションカラーを掛け合わせ
			blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;  // 加算
			blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;  // デスティネーションカラーはそのまま

			// アルファの設定（アルファを無視する場合）
			blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
			blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;

			break;

		case BlendMode::kScreen:

			blendDesc.RenderTarget[0].BlendEnable = TRUE;


			blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
			blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;

			blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_INV_SRC_COLOR;
			blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;

			break;

		case BlendMode::kAlpla:

			blendDesc.RenderTarget[0].BlendEnable = TRUE;

			blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
			blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
			blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;

			blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
			blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
			blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;

			break;
		}

		/*----------------------------------------------------------
		* GraphicsPipelineの設定
		----------------------------------------------------------*/
		D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
		graphicsPipelineStateDesc.pRootSignature = rootSignature.Get();
		graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;
		graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),vertexShaderBlob->GetBufferSize() };
		graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),pixelShaderBlob->GetBufferSize() };
		graphicsPipelineStateDesc.BlendState = blendDesc;
		graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;

		// 書き込むRTVの情報
		graphicsPipelineStateDesc.NumRenderTargets = 1;
		graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

		// 利用するトポロジー(形状)のタイプ。三角形
		graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

		// どのように画面に色をつけるか
		graphicsPipelineStateDesc.SampleDesc.Count = 1;
		graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

		// DepthStencilの設定
		graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
		graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
		 
		// 実際に生成
		sPipeLineStates_[i] = nullptr;
		hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&sPipeLineStates_[i]));
		assert(SUCCEEDED(hr));
	}

}

void ParticleManager::InitializeVertexData()
{
	// 汎用
	modelData.vertices.push_back({ {1.0f,1.0f,0.0f,1.0f},{0.0f,0.0f},{0.0f,0.0f,1.0f} }); // 左上
	modelData.vertices.push_back({ {-1.0f,1.0f,0.0f,1.0f},{1.0f,0.0f},{0.0f,0.0f,1.0f} }); // 右上
	modelData.vertices.push_back({ {1.0f,-1.0f,0.0f,1.0f},{0.0f,1.0f},{0.0f,0.0f,0.0f} }); // 左下
	modelData.vertices.push_back({ {1.0f,-1.0f,0.0f,1.0f},{0.0f,1.0f},{0.0f,0.0f,1.0f} }); // 左下
	modelData.vertices.push_back({ {-1.0f,1.0f,0.0f,1.0f},{1.0f,0.0f},{0.0f,0.0f,1.0f} });// 右上
	modelData.vertices.push_back({ {-1.0f,-1.0f,0.0f,1.0f},{1.0f,1.0f},{0.0f,0.0f,1.0f} }); // 右下

	// リング

	const uint32_t kRingDivide = 32;
	const float kOuterRadius = 1.0f;
	const float kInnerRadius = 0.1f;
	const float radianPerDivide = 2.0f * std::numbers::pi_v<float> / float(kRingDivide);

	for (uint32_t index = 0; index < kRingDivide; ++index) {
		float angle = index * radianPerDivide;
		float angleNext = (index + 1) * radianPerDivide;

		float sin = std::sin(angle);
		float cos = std::cos(angle);
		float sinNext = std::sin(angleNext);
		float cosNext = std::cos(angleNext);

		float u = float(index) / float(kRingDivide);
		float uNext = float(index + 1) / float(kRingDivide);

		// 三角形1
		ringModelData.vertices.push_back({ { -sin * kOuterRadius, cos * kOuterRadius, 0.0f, 1.0f }, { u, 0.0f }, { 0.0f, 0.0f, 1.0f } });
		ringModelData.vertices.push_back({ { -sinNext * kOuterRadius, cosNext * kOuterRadius, 0.0f, 1.0f }, { uNext, 0.0f }, { 0.0f, 0.0f, 1.0f } });
		ringModelData.vertices.push_back({ { -sin * kInnerRadius, cos * kInnerRadius, 0.0f, 1.0f }, { u, 1.0f }, { 0.0f, 0.0f, 1.0f } });

		// 三角形2
		ringModelData.vertices.push_back({ { -sin * kInnerRadius, cos * kInnerRadius, 0.0f, 1.0f }, { u, 1.0f }, { 0.0f, 0.0f, 1.0f } });
		ringModelData.vertices.push_back({ { -sinNext * kOuterRadius, cosNext * kOuterRadius, 0.0f, 1.0f }, { uNext, 0.0f }, { 0.0f, 0.0f, 1.0f } });
		ringModelData.vertices.push_back({ { -sinNext * kInnerRadius, cosNext * kInnerRadius, 0.0f, 1.0f }, { uNext, 1.0f }, { 0.0f, 0.0f, 1.0f } });
	}

	const float kCylinderHeight = 1.0f; // 円柱の高さ
	for (uint32_t index = 0; index < kRingDivide; ++index) {
		float sin = std::sin(index * radianPerDivide);
		float cos = std::cos(index * radianPerDivide);
		float sinNext = std::sin((index + 1) * radianPerDivide);
		float cosNext = std::cos((index + 1) * radianPerDivide);

		float u = float(index) / float(kRingDivide);
		float uNext = float(index + 1) / float(kRingDivide);

		// 下部の2点
		Vector4 bottomLeft = { -sin * kOuterRadius * 0.1f,      0.0f, cos * kOuterRadius * 0.1f,      1.0f };
		Vector4 bottomRight = { -sinNext * kOuterRadius * 0.1f,  0.0f, cosNext * kOuterRadius * 0.1f,  1.0f };

		// 上部の2点
		Vector4 topLeft = { -sin * kOuterRadius * 0.1f,      kCylinderHeight, cos * kOuterRadius * 0.1f,      1.0f };
		Vector4 topRight = { -sinNext * kOuterRadius * 0.1f,  kCylinderHeight, cosNext * kOuterRadius * 0.1f,  1.0f };

		Vector3 normal = { 0.0f, 0.0f, 1.0f }; // 任意で調整

		// --- 三角形① (下左 → 下右 → 上左)
		cylinderModelData.vertices.push_back({ bottomLeft,  { u, 0.0f }, normal });
		cylinderModelData.vertices.push_back({ bottomRight, { uNext, 0.0f }, normal });
		cylinderModelData.vertices.push_back({ topLeft,     { u, 1.0f }, normal });

		// --- 三角形② (上左 → 下右 → 上右)
		cylinderModelData.vertices.push_back({ topLeft,     { u, 1.0f }, normal });
		cylinderModelData.vertices.push_back({ bottomRight, { uNext, 0.0f }, normal });
		cylinderModelData.vertices.push_back({ topRight,    { uNext, 1.0f }, normal });
	}
}

void ParticleManager::CreateVertexResource()
{
	// 頂点リソースを作る
	vertexResource = dxCommon_->CreateBufferResource(sizeof(VertexData) * modelData.vertices.size());

	ringVertexResource = dxCommon_->CreateBufferResource(sizeof(VertexData) * ringModelData.vertices.size());

	cylinderVertexResource = dxCommon_->CreateBufferResource(sizeof(VertexData) * cylinderModelData.vertices.size());
}

void ParticleManager::CreateVertexBufferView()
{
	// 汎用
	// リソースの先頭アドレス
	vertexBufferView.BufferLocation = vertexResource.Get()->GetGPUVirtualAddress();

	// 使用するリソースのサイズ
	vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());

	// 1頂点あたりのサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	// ring
	ringVertexBufferView.BufferLocation = ringVertexResource.Get()->GetGPUVirtualAddress();
	ringVertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * ringModelData.vertices.size());
	ringVertexBufferView.StrideInBytes = sizeof(VertexData);

	// cylinder
	cylinderVertexBufferView.BufferLocation = cylinderVertexResource.Get()->GetGPUVirtualAddress();
	cylinderVertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * cylinderModelData.vertices.size());
	cylinderVertexBufferView.StrideInBytes = sizeof(VertexData);

}

void ParticleManager::WriteDataInResource()
{
	VertexData* vertexData;
	vertexResource.Get()->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());

	VertexData* ringVertexData;
	ringVertexResource.Get()->Map(0, nullptr, reinterpret_cast<void**>(&ringVertexData));
	std::memcpy(ringVertexData, ringModelData.vertices.data(), sizeof(VertexData) * ringModelData.vertices.size());

	VertexData* cylinderVertexData;
	cylinderVertexResource.Get()->Map(0, nullptr, reinterpret_cast<void**>(&cylinderVertexData));
	std::memcpy(cylinderVertexData, cylinderModelData.vertices.data(), sizeof(VertexData) * cylinderModelData.vertices.size());
}

void ParticleManager::CreateParticleGroup(const std::string name, const std::string textureFilePath,ParticleType type)
{
	// パーティクルグループの名前を検索
	// 無かった場合新しく作る

	if (particleGroups.find(name) == particleGroups.end()) {

		ParticleGroup newParticleGroup;

		newParticleGroup.materialData.textureFilePath = textureFilePath;

		// テクスチャ読み込み
		TextureManager::GetInstance()->LoadTexture(textureFilePath);

		// マテリアルデータにテクスチャのSRVインデックスを記録
		newParticleGroup.materialData.textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(newParticleGroup.materialData.textureFilePath);

		// インスタンシング用リソースの生成
		newParticleGroup.instancingResource = dxCommon_->CreateBufferResource(sizeof(ParticleForGPU) * kNumMaxInstance);
		
		newParticleGroup.instancingResource->Map(0, nullptr, reinterpret_cast<void**>(&newParticleGroup.instancingData));

		newParticleGroup.srvIndex = srvManager_->Allocate();

		srvManager_->CreateSRVforStructuredBuffer(newParticleGroup.srvIndex, newParticleGroup.instancingResource.Get(), kNumMaxInstance, sizeof(ParticleForGPU));

		newParticleGroup.materialResource = dxCommon_->CreateBufferResource(sizeof(Material));

		newParticleGroup.materialResource->Map(0, nullptr, reinterpret_cast<void**>(&newParticleGroup.material));

		newParticleGroup.type = type;

		newParticleGroup.kNumInstance = 0;

		newParticleGroup.blendMode = BlendMode::kAdd;

		newParticleGroup.kNumInstance = 0;

		particleGroups[name] = newParticleGroup;
	}
}

// ビルボードマトリクスの計算
void ParticleManager::calculationBillboardMatrix()
{
	Matrix4x4 backToFrontMatrix = MakeRotateYMatrix(std::numbers::pi_v<float>);

	billboardMatrix = Multiply(backToFrontMatrix, cameraManager_->GetActiveCamera()->GetViewProjection().matWorld_);

	billboardMatrix.m[3][0] = 0.0f;
	billboardMatrix.m[3][1] = 0.0f;
	billboardMatrix.m[3][2] = 0.0f;
}

ParticleManager::Particle ParticleManager::MakeNewParticle(const Vector3& translate, ParticleStates states,bool sameScale,float speed)
{
		// 新しいパーティクルの生成
		std::unique_ptr<Particle> newParticle;

		newParticle = std::make_unique<Particle>();



		// positionの設定
		std::uniform_real_distribution<float> distTranslateX(states.translateRange.min.x, states.translateRange.max.x);
		std::uniform_real_distribution<float> distTranslateY(states.translateRange.min.y, states.translateRange.max.y);
		std::uniform_real_distribution<float> distTranslateZ(states.translateRange.min.z, states.translateRange.max.z);

		Vector3 randomTranslate = { distTranslateX(randomEngine),distTranslateY(randomEngine) ,distTranslateZ(randomEngine) };

		std::uniform_real_distribution<float> distRotateX(states.rotateRange.min.x, states.rotateRange.max.x);
		std::uniform_real_distribution<float> distRotateY(states.rotateRange.min.y, states.rotateRange.max.y);
		std::uniform_real_distribution<float> distRotateZ(states.rotateRange.min.z, states.rotateRange.max.z);

		Vector3 randomRotate = { distRotateX(randomEngine),distRotateY(randomEngine),distRotateZ(randomEngine) };



		// scaleの設定

		// 同じスケールにするかどうか
		if (sameScale)
		{
			// 初期
			std::uniform_real_distribution<float> distStartScale(states.startScaleRange.min.x, states.startScaleRange.max.x);
			float scale = distStartScale(randomEngine);
			newParticle->startScale = { scale,scale,scale };

			// 終了
			std::uniform_real_distribution<float> distFinishScale(states.finishScaleRange.min.x, states.finishScaleRange.max.x);
			scale = distFinishScale(randomEngine);
			newParticle->finishScale = { scale,scale,scale };

		}
		else
		{
			std::uniform_real_distribution<float> distStartScaleX(states.startScaleRange.min.x, states.startScaleRange.max.x);
			std::uniform_real_distribution<float> distStartScaleY(states.startScaleRange.min.y, states.startScaleRange.max.y);
			std::uniform_real_distribution<float> distStartScaleZ(states.startScaleRange.min.z, states.startScaleRange.max.z);

			newParticle->startScale = { distStartScaleX(randomEngine),distStartScaleY(randomEngine),distStartScaleZ(randomEngine) };

			std::uniform_real_distribution<float> distFinishScaleX(states.finishScaleRange.min.x, states.finishScaleRange.max.x);
			std::uniform_real_distribution<float> distFinishScaleY(states.finishScaleRange.min.y, states.finishScaleRange.max.y);
			std::uniform_real_distribution<float> distFinishScaleZ(states.finishScaleRange.min.z, states.finishScaleRange.max.z);

			newParticle->finishScale = { distFinishScaleX(randomEngine),distFinishScaleY(randomEngine),distFinishScaleZ(randomEngine) };
		}

		// transformの設定
		
		newParticle->transform.rotate = randomRotate;
		newParticle->transform.translate = translate + randomTranslate;


		std::uniform_real_distribution<float> distVelocityX(states.velocityRange.min.x, states.velocityRange.max.x);
		std::uniform_real_distribution<float> distVelocityY(states.velocityRange.min.y, states.velocityRange.max.y);
		std::uniform_real_distribution<float> distVelocityZ(states.velocityRange.min.z, states.velocityRange.max.z);

		// velocityの設定
		newParticle->velocity = { distVelocityX(randomEngine),distVelocityY(randomEngine),distVelocityZ(randomEngine) };

		float length = std::sqrt(newParticle->velocity.x * newParticle->velocity.x + newParticle->velocity.y * newParticle->velocity.y + newParticle->velocity.z * newParticle->velocity.z);

		if (length > speed) {
			newParticle->velocity = Normalize(newParticle->velocity) * speed;
		}
		else if (length < std::numeric_limits<float>::epsilon()) {
			// ゼロベクトルだった場合にデフォルトの速度を与えるなど
			newParticle->velocity = { 0.0f, 0.0f, 0.0f }; // 例
		}

		std::uniform_real_distribution<float> distTime(states.lifeTimeRange.min, states.lifeTimeRange.max);

		// ライフタイムの設定
		newParticle->lifeTime = distTime(randomEngine);
		newParticle->currentTime = 0.0f;

		// 発生時の色の設定
		std::uniform_real_distribution<float> startColorR(states.startColorRange.min.x, states.startColorRange.max.x);
		std::uniform_real_distribution<float> startColorG(states.startColorRange.min.y, states.startColorRange.max.y);
		std::uniform_real_distribution<float> startColorB(states.startColorRange.min.z, states.startColorRange.max.z);
		std::uniform_real_distribution<float> startColorA(states.startColorRange.min.w, states.startColorRange.max.w);

		Vector4 startColor = { startColorR(randomEngine),startColorG(randomEngine),startColorB(randomEngine),startColorA(randomEngine) };

		// 終了時の色の設定
		std::uniform_real_distribution<float> finishColorR(states.finishColorRange.min.x, states.finishColorRange.max.x);
		std::uniform_real_distribution<float> finishColorG(states.finishColorRange.min.y, states.finishColorRange.max.y);
		std::uniform_real_distribution<float> finishColorB(states.finishColorRange.min.z, states.finishColorRange.max.z);
		std::uniform_real_distribution<float> finishColorA(states.finishColorRange.min.w, states.finishColorRange.max.w);

		Vector4 finishColor = { finishColorR(randomEngine),finishColorG(randomEngine),finishColorB(randomEngine),finishColorA(randomEngine) };

		newParticle->startColor = startColor;
		newParticle->finishColor = finishColor;

		return *newParticle;
}

ParticleManager::Particle ParticleManager::MakeNewRadialParticle(const Vector3& translate, ParticleStates states, bool sameScale, float speed)
{
	// 新しいパーティクルの生成
	std::unique_ptr<Particle> newParticle = std::make_unique<Particle>();

	// 中心位置（エミッターの位置）
	Vector3 center = translate;

	// ランダムな位置オフセット（中心からの距離）
	std::uniform_real_distribution<float> distTranslateX(states.translateRange.min.x, states.translateRange.max.x);
	std::uniform_real_distribution<float> distTranslateY(states.translateRange.min.y, states.translateRange.max.y);
	std::uniform_real_distribution<float> distTranslateZ(states.translateRange.min.z, states.translateRange.max.z);

	Vector3 offset = {
		distTranslateX(randomEngine),
		distTranslateY(randomEngine),
		distTranslateZ(randomEngine)
	};

	// パーティクルの位置（中心 + オフセット）
	newParticle->transform.translate = center + offset;

	// 中心 → パーティクル方向ベクトル（XY平面を仮定）
	Vector3 direction = Normalize(offset);

	// 回転（Z軸）を方向に合わせて設定（XY平面の場合）
	float angleZ = std::atan2(direction.y, direction.x);
	newParticle->transform.rotate = { 0.0f, 0.0f, angleZ };

	// スケール設定（細長くする）
	if (sameScale)
	{
		std::uniform_real_distribution<float> distStartScale(states.startScaleRange.min.x, states.startScaleRange.max.x);
		float scale = distStartScale(randomEngine);
		newParticle->startScale = { scale, scale * 0.1f, scale };

		std::uniform_real_distribution<float> distFinishScale(states.finishScaleRange.min.x, states.finishScaleRange.max.x);
		scale = distFinishScale(randomEngine);
		newParticle->finishScale = { scale, scale * 0.1f, scale };
	}
	else
	{
		std::uniform_real_distribution<float> distStartScaleX(states.startScaleRange.min.x, states.startScaleRange.max.x);
		std::uniform_real_distribution<float> distStartScaleY(states.startScaleRange.min.y, states.startScaleRange.max.y);
		std::uniform_real_distribution<float> distStartScaleZ(states.startScaleRange.min.z, states.startScaleRange.max.z);

		newParticle->startScale = {
			distStartScaleX(randomEngine),
			distStartScaleY(randomEngine),
			distStartScaleZ(randomEngine)
		};

		std::uniform_real_distribution<float> distFinishScaleX(states.finishScaleRange.min.x, states.finishScaleRange.max.x);
		std::uniform_real_distribution<float> distFinishScaleY(states.finishScaleRange.min.y, states.finishScaleRange.max.y);
		std::uniform_real_distribution<float> distFinishScaleZ(states.finishScaleRange.min.z, states.finishScaleRange.max.z);

		newParticle->finishScale = {
			distFinishScaleX(randomEngine),
			distFinishScaleY(randomEngine),
			distFinishScaleZ(randomEngine)
		};
	}

	// velocity（速度）設定：中心→外方向に speed 倍のベクトル
	newParticle->velocity = direction * 1.8f;

	// 念のため、速度がゼロでないことを保証
	float length = std::sqrt(newParticle->velocity.x * newParticle->velocity.x +
		newParticle->velocity.y * newParticle->velocity.y +
		newParticle->velocity.z * newParticle->velocity.z);

	if (length < std::numeric_limits<float>::epsilon()) {
		// ゼロベクトルだった場合にデフォルトの速度を与えるなど
		newParticle->velocity = { 1.0f, 0.0f, 0.0f }; // 例
		}

	// ライフタイム設定
	std::uniform_real_distribution<float> distTime(states.lifeTimeRange.min, states.lifeTimeRange.max);
	newParticle->lifeTime = distTime(randomEngine);
	newParticle->currentTime = 0.0f;

	// カラー（開始時）
	std::uniform_real_distribution<float> startColorR(states.startColorRange.min.x, states.startColorRange.max.x);
	std::uniform_real_distribution<float> startColorG(states.startColorRange.min.y, states.startColorRange.max.y);
	std::uniform_real_distribution<float> startColorB(states.startColorRange.min.z, states.startColorRange.max.z);
	std::uniform_real_distribution<float> startColorA(states.startColorRange.min.w, states.startColorRange.max.w);

	newParticle->startColor = {
		startColorR(randomEngine),
		startColorG(randomEngine),
		startColorB(randomEngine),
		startColorA(randomEngine)
	};

	// カラー（終了時）
	std::uniform_real_distribution<float> finishColorR(states.finishColorRange.min.x, states.finishColorRange.max.x);
	std::uniform_real_distribution<float> finishColorG(states.finishColorRange.min.y, states.finishColorRange.max.y);
	std::uniform_real_distribution<float> finishColorB(states.finishColorRange.min.z, states.finishColorRange.max.z);
	std::uniform_real_distribution<float> finishColorA(states.finishColorRange.min.w, states.finishColorRange.max.w);

	newParticle->finishColor = {
		finishColorR(randomEngine),
		finishColorG(randomEngine),
		finishColorB(randomEngine),
		finishColorA(randomEngine)
	};

	return *newParticle;
}

void ParticleManager::DecelerationUpdate(Particle& particle)
{
	float t = particle.currentTime / particle.lifeTime - 0.1f;
	t = std::clamp(t, 0.0f, 1.0f);

	// easeInCirc
	float ease = 1.0f - sqrtf(1.0f - t * t);

	// 減速の最大割合（たとえば0.9なら最小で10%は残る）
	float maxDeceleration = 0.51f;

	// イージングで変化する減速率（0.0〜0.9）
	float decelerationFactor = ease * maxDeceleration;

	// 減速分の速度を差し引く
	Vector3 deceleratedVelocity = particle.velocity * decelerationFactor;
	particle.velocity -= deceleratedVelocity;
}

void ParticleManager::Emit(const std::string name, const Vector3& position, uint32_t count, ParticleStates states)
{
	if (particleGroups.find(name) != particleGroups.end()) {
		for (uint32_t currentCount = 0; currentCount < count;) {
			particleGroups.find(name)->second.particles.push_back
			(
				MakeNewParticle(position,states,particleGroups.find(name)->second.sameScale, particleGroups.find(name)->second.speed)
			);
			++currentCount;
		}
	}
}

void ParticleManager::RadialEmit(const std::string name, const Vector3& position, uint32_t count, ParticleStates states)
{
	if (particleGroups.find(name) != particleGroups.end()) {
		for (uint32_t currentCount = 0; currentCount < count;) {
			particleGroups.find(name)->second.particles.push_back
			(
				MakeNewRadialParticle(position, states, particleGroups.find(name)->second.sameScale, particleGroups.find(name)->second.speed)
			);
			++currentCount;
		}
	}
}

void ParticleManager::Finalize()
{

	instance.reset();

}
