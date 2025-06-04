#include "BaseCamera.h"

void BaseCamera::Initialize()
{
	// viewProjectionの初期化
	viewProjection_ = std::make_unique<ViewProjection>();
	viewProjection_->Initialize();

	viewProjection_->transform.translate = { 0.0f,0.0f,-15.0f };

	viewProjection_->Update();

	worldMatrix = MakeAffineMatrix(viewProjection_->transform.scale, viewProjection_->transform.rotate, viewProjection_->transform.translate);
}

void BaseCamera::Update()
{
	worldMatrix = MakeAffineMatrix(viewProjection_->transform.scale, viewProjection_->transform.rotate, viewProjection_->transform.translate);

	viewProjection_->Update();
}
