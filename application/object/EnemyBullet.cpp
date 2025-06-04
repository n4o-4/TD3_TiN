#include "EnemyBullet.h"
#include "Player.h"

void EnemyBullet::Initialize(const WorldTransform worldTransform,Vector3 target)
{
	objectTransform_ = std::make_unique<WorldTransform>();
	objectTransform_->Initialize();

	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(Object3dCommon::GetInstance());
	ModelManager::GetInstance()->LoadModel("enemy/enemy.obj");
	object3d_->SetModel("enemy/enemy.obj");

	BaseObject::Initialize(objectTransform_->transform.translate, 1.0f);

	objectTransform_->transform = worldTransform.transform;

	objectTransform_->UpdateMatrix();

	targetPosition_ = target;

	velocity_ = targetPosition_ - objectTransform_->transform.translate;
	velocity_ = Normalize(velocity_);

	isActive_ = true;
}

void EnemyBullet::Update()
{
	objectTransform_->transform.translate += velocity_;

	object3d_->SetLocalMatrix(MakeIdentity4x4());

	objectTransform_->UpdateMatrix();

	activeTime_ += 1.0f / 60.0f;
	if (activeTime_ > kActiveTime_)
	{
		isActive_ = false;
	}

	BaseObject::Update(objectTransform_->transform.translate);
}

void EnemyBullet::Draw(ViewProjection viewProjection, DirectionalLight directionalLight, PointLight pointLight, SpotLight spotLight)
{
	if (!isActive_) return;

	object3d_->Draw(*objectTransform_, viewProjection, directionalLight, pointLight, spotLight);
}

///=============================================================================
///						当たり判定
///--------------------------------------------------------------
///						接触開始処理
void EnemyBullet::OnCollisionEnter(BaseObject *other) {
	//========================================
	// Player
	if(dynamic_cast<EnemyBullet *>( other )) {
		// 自分を消す
		isActive_ = false;
	}
}
///--------------------------------------------------------------
///						接触継続処理
void EnemyBullet::OnCollisionStay(BaseObject *other) {

}
///--------------------------------------------------------------
///						接触終了処理
void EnemyBullet::OnCollisionExit(BaseObject *other) {

}
