#include "WorldTransform.h"

void WorldTransform::Initialize()
{

	transformResource_ = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(ConstBufferDataWorldTransform));

	// データを書き込むためのアドレスを取得
	transformResource_.Get()->Map(0, nullptr, reinterpret_cast<void**>(&transformData_));

	transform.scale = { 1.0f,1.0f,1.0f };

	transform.rotate = { 0.0f,0.0f,0.0f };

	transform.translate = { 0.0f,0.0f,0.0f };

}

void WorldTransform::UpdateMatrix()
{
	if (parent_)
	{
		parent_->UpdateMatrix();

		matWorld_ = Multiply(MakeAffineMatrix(transform.scale, transform.rotate, transform.translate), parent_->matWorld_);
	}
	else
	{
		matWorld_ = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	}

	matWorldInverse_ = Inverse(matWorld_);

	transformData_->matWorld = matWorld_;

	transformData_->matWorldInverse = matWorldInverse_;

}

void WorldTransform::TransferMatrix()
{
	matWorldInverse_ = Inverse(matWorld_);

	transformData_->matWorld = matWorld_;

	transformData_->matWorldInverse = matWorldInverse_;
}

Vector3 WorldTransform::GetWorldPosition() const
{
	return {
		matWorld_.m[3][0],
		matWorld_.m[3][1],
		matWorld_.m[3][2]
	};
}
