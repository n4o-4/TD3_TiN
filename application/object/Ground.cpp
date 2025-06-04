#include "Ground.h"

void Ground::Initialize() {
	model_ = std::make_unique<Object3d>();
	model_->Initialize(Object3dCommon::GetInstance());

	ModelManager::GetInstance()->LoadModel("ground/ground.obj");
	ModelManager::GetInstance()->LoadModel("ground/Ground_01.obj");
	model_->SetModel("ground/Ground_01.obj");

	model_->SetLocalMatrix(MakeIdentity4x4());

	// depthの書き込みを無効化
	model_->SetWriteDepthTest(false);

	worldTransform_ = std::make_unique<WorldTransform>();
	worldTransform_->Initialize();
	worldTransform_->transform.scale = { 10.0f,10.0f,10.0f };
	worldTransform_->transform.rotate = { 0.0f,0.0f,0.0f };
}

void Ground::Update() {

#ifdef _DEBUG
	if (ImGui::TreeNode("ground")) {
		ImGui::DragFloat3("worldTransform_", &worldTransform_->transform.rotate.x, 0.01f);

		ImGui::TreePop(); // TreeNodeを閉じる
	}

#endif

	worldTransform_->UpdateMatrix();
	model_->SetLocalMatrix(MakeIdentity4x4());
	model_->Update();
}

void Ground::Draw(ViewProjection viewProjection, DirectionalLight directionalLight, PointLight pointLight, SpotLight spotLight) {
	model_->Draw(*worldTransform_.get(), viewProjection, directionalLight, pointLight, spotLight);
}
