#include "ViewProjection.h"
#include "WinApp.h"

void ViewProjection::Initialize()
{
	viewProjectionResource_ = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(ConstBufferDataViewProjection));

	viewProjectionResource_.Get()->Map(0, nullptr, reinterpret_cast<void**>(&viewProjectionData_));

	transform.scale = { 1.0f,1.0f,1.0f };

	transform.rotate = { 0.0f,0.0f,0.0f };

	transform.translate = { 0.0f,0.0f,-15.0f };

	fovY = 0.85f;
	aspectRation = static_cast<float>(WinApp::kClientWidth) / static_cast<float>(WinApp::kClientHeight);
	nearClip = 0.1f;
	farClip = 100.0f;

	viewProjectionData_->matView = MakeIdentity4x4();

	viewProjectionData_->matProjection = MakeIdentity4x4();
}

void ViewProjection::Update()
{
	worldPosition_ = transform.translate;

	matWorld_ = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);

	matView_ = Inverse(matWorld_);

	matProjection_ = MakePerspectiveFovMatrix(fovY, aspectRation, nearClip, farClip);

	viewProjectionData_->worldPosition = Vector3(matWorld_.m[3][0], matWorld_.m[3][1], matWorld_.m[3][2]);

	viewProjectionData_->matView = matView_;

	viewProjectionData_->matProjection = matProjection_;
}
