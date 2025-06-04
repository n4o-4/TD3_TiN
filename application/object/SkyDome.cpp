#include "SkyDome.h"

///=============================================================================
///						初期化
void SkyDome::Initialize() {
	//========================================
	// モデルの読み込み
	ModelManager::GetInstance()->LoadModel("skyDome/skyDome.obj");
	ModelManager::GetInstance()->LoadModel("skyDome/skyDomeBg.obj");
	ModelManager::GetInstance()->LoadModel("skyDome/skyDomeCloud.obj");
	ModelManager::GetInstance()->LoadModel("skyDome/cube.obj");
	//========================================
	// モデルの初期化
	// 基本スカイドーム
	model_ = std::make_unique<Object3d>();
	model_->Initialize(Object3dCommon::GetInstance());
	model_->SetModel("skyDome/skyDome.obj");
	// スカイドームの背景
	modelSkydomeBg_ = std::make_unique<Object3d>();
	modelSkydomeBg_->Initialize(Object3dCommon::GetInstance());
	modelSkydomeBg_->SetModel("skyDome/skyDomeBg.obj");
	// スカイドームの雲
	modelSkydomeCloud_ = std::make_unique<Object3d>();
	modelSkydomeCloud_->Initialize(Object3dCommon::GetInstance());
	modelSkydomeCloud_->SetModel("skyDome/skyDomeCloud.obj");
	// cube
	cube_ = std::make_unique<Object3d>();
	cube_->Initialize(Object3dCommon::GetInstance());
	cube_->SetModel("skyDome/cube.obj");
	// depthの書き込みを無効化
	cube_->SetWriteDepthTest(false);


	//========================================
	// ワールド変換の初期化
	// 基本スカイドーム
	worldTransform_ = std::make_unique<WorldTransform>();
	worldTransform_->Initialize();
	worldTransform_->transform.scale = { 1.0f,1.0f,1.0f };
	// スカイドームの背景
	worldTransformSkydomeBg_ = std::make_unique<WorldTransform>();
	worldTransformSkydomeBg_->Initialize();
	worldTransformSkydomeBg_->transform.scale = { 1.0f,1.0f,1.0f };
	// スカイドームの雲
	worldTransformSkydomeCloud_ = std::make_unique<WorldTransform>();
	worldTransformSkydomeCloud_->Initialize();
	worldTransformSkydomeCloud_->transform.scale = { 1.0f,1.0f,1.0f };
	// cube
	worldTransformCube_ = std::make_unique<WorldTransform>();
	worldTransformCube_->Initialize();
	worldTransformCube_->transform.scale = { 20.0f,20.0f,20.0f };
	worldTransformCube_->transform.translate = { 0.0f,50.0f,0.0f };
}
///=============================================================================
///						更新
void SkyDome::Update() {
	//========================================
	// ワールド変換の更新
#ifdef _DEBUG
	if (ImGui::TreeNode("skyDome")) {
		ImGui::DragFloat3("worldTransform_", &worldTransform_->transform.scale.x, 0.01f);
		ImGui::TreePop(); // TreeNodeを閉じる
	}

#endif
	//========================================
	// 基本スカイドーム
	worldTransform_->UpdateMatrix();
	model_->SetLocalMatrix(MakeIdentity4x4());
	model_->Update();
	// スカイドームの背景
	worldTransformSkydomeBg_->UpdateMatrix();
	modelSkydomeBg_->SetLocalMatrix(MakeIdentity4x4());
	modelSkydomeBg_->Update();
	// スカイドームの雲
	// 少し回転させる
	worldTransformSkydomeCloud_->transform.rotate.y += 0.0001f;
	worldTransformSkydomeCloud_->UpdateMatrix();
	modelSkydomeCloud_->SetLocalMatrix(MakeIdentity4x4());
	modelSkydomeCloud_->Update();

	// cube
	worldTransformCube_->UpdateMatrix();	
	cube_->SetLocalMatrix(MakeIdentity4x4());
	cube_->Update();
}
///=============================================================================
///						描画
void SkyDome::Draw(ViewProjection viewProjection, DirectionalLight directionalLight, PointLight pointLight, SpotLight spotLight) {
	//model_->Draw(*worldTransform_.get(), viewProjection, directionalLight, pointLight, spotLight);
	//modelSkydomeBg_->Draw(*worldTransformSkydomeBg_.get(), viewProjection, directionalLight, pointLight, spotLight);
	//modelSkydomeCloud_->Draw(*worldTransformSkydomeCloud_.get(), viewProjection, directionalLight, pointLight, spotLight);
	cube_->Draw(*worldTransformCube_.get(), viewProjection, directionalLight, pointLight, spotLight);
}
