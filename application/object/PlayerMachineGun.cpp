#include "PlayerMachineGun.h"
#include "Enemy.h"

PlayerMachineGun::PlayerMachineGun(const Vector3 &position, const Vector3 &velocity) {
	model_ = std::make_unique<Object3d>();
	model_->Initialize(Object3dCommon::GetInstance());

	ModelManager::GetInstance()->LoadModel("mg.obj");
	model_->SetModel("mg.obj");

	worldTransform_ = std::make_unique<WorldTransform>();
	worldTransform_->Initialize();
	worldTransform_->transform.translate = position;
	worldTransform_->transform.scale = {0.8f, 0.8f, 0.8f}; // マシンガンの弾は小さめ
	velocity_ = velocity;
	BaseObject::Initialize(worldTransform_->transform.translate, 1.6f); // 当たり判定のサイズ

	ParticleManager::GetInstance()->CreateParticleGroup("bullet", "Resources/circle.png", ParticleManager::ParticleType::Normal);
	ParticleManager::GetInstance()->GetParticleGroup("bullet")->enableBillboard = true;

	particleEmitter_ = std::make_unique<ParticleEmitter>();
	particleEmitter_->Initialize("bullet");
	particleEmitter_->SetParticleCount(2);	// デフォルト発生数
	particleEmitter_->SetFrequency(0.001f); // より高頻度で発生させる
	particleEmitter_->SetLifeTimeRange(ParticleManager::Range({0.1f, 0.1f}));
	particleEmitter_->SetStartColorRange(ParticleManager::ColorRange({0.961f, 1.000f, 0.388f, 1.0f}, {0.961f, 1.000f, 0.388f, 1.0f}));
	particleEmitter_->SetFinishColorRange(ParticleManager::ColorRange({0.961f, 1.000f, 0.388f, 1.0f}, {0.961f, 1.000f, 0.388f, 1.0f}));

	particleEmitter_->SetVelocityRange(ParticleManager::Vec3Range({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}));

	particleEmitter_->SetStartScaleRange(ParticleManager::Vec3Range({ 0.4f, 0.4f,0.4f }, { 0.4f, 0.4f,0.4f }));

	particleEmitter_->SetFinishScaleRange(ParticleManager::Vec3Range({ 0.4f, 0.4f,0.4f }, { 0.4f, 0.4f,0.4f }));

	ParticleManager::GetInstance()->CreateParticleGroup("hitSprite", "Resources/gradationLine.png", ParticleManager::ParticleType::Ring);

	ParticleManager::GetInstance()->GetParticleGroup("hitSprite")->enableBillboard = true;

	hitSprite_ = std::make_unique<ParticleEmitter>();
	hitSprite_->Initialize("hitSprite");
	hitSprite_->SetParticleCount(1);  // デフォルト発生数

	hitSprite_->SetLifeTimeRange(ParticleManager::Range({ 0.5f, 0.5f }));
	hitSprite_->SetStartColorRange(ParticleManager::ColorRange({ 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }));
	hitSprite_->SetFinishColorRange(ParticleManager::ColorRange({ 1.0f, 1.0f, 1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 0.0f }));

	hitSprite_->SetVelocityRange(ParticleManager::Vec3Range({ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }));
	hitSprite_->SetStartScaleRange(ParticleManager::Vec3Range({ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }));
	hitSprite_->SetStartScaleRange(ParticleManager::Vec3Range({ 15.0f,15.0f,15.0f, }, { 15.0f,15.0f,15.0f }));
}

void PlayerMachineGun::Update() {
	lifeTime_++;

	// 一定時間で消滅
	if (lifeTime_ > 120) { // 約2秒
		isActive_ = false;
	}

	// 移動処理
	worldTransform_->transform.translate = worldTransform_->transform.translate + velocity_;

	// 進行方向に向かせる
	if (Length(velocity_) > 0.0001f) {
		Vector3 dir = Normalize(velocity_);

		// Yaw（Y軸回転）を求める
		float yaw = std::atan2(dir.x, dir.z);

		// Pitch（X軸回転）を求める（XZ平面で真横に撃たない場合のみ有効）
		float pitch = -std::asin(dir.y); // マイナスを付けて上方向を正に

		worldTransform_->transform.rotate = {pitch, yaw, 0.0f};
	}

	model_->SetLocalMatrix(MakeIdentity4x4()); // ローカル行列を単位行列に
	worldTransform_->UpdateMatrix();

	particleEmitter_->SetPosition(worldTransform_->transform.translate);

	particleEmitter_->Update();

	BaseObject::Update(worldTransform_->transform.translate);
}

void PlayerMachineGun::Draw(ViewProjection viewProjection, DirectionalLight directionalLight,
							PointLight pointLight, SpotLight spotLight) {
	if (isActive_) {
		//model_->Draw(*worldTransform_.get(), viewProjection, directionalLight, pointLight, spotLight);
	}
}

std::unique_ptr<PlayerMachineGun> PlayerMachineGun::Shoot(
	const Vector3 &playerPosition,
	FollowCamera *followCamera,
	Vector3 &outRecoilVelocity,
	float &outShakeIntensity) {

	// プレイヤーの向いている方向を取得（カメラの方向を基準にする）
	Vector3 forward = {0.0f, 0.0f, 1.0f}; // Z方向が基準
	Matrix4x4 rotationMatrix = MakeRotateMatrix(followCamera->GetViewProjection().transform.rotate);
	forward = TransformNormal(forward, rotationMatrix);

	// ランダムなばらつきを加える
	float randomX = ((rand() % 100) / 100.0f - 0.5f) * kSpread;
	float randomY = ((rand() % 100) / 100.0f - 0.5f) * kSpread;

	// ばらつきを適用
	Vector3 bulletVelocity = forward * kBulletSpeed + Vector3(randomX, randomY, 0.0f);
	bulletVelocity = Normalize(bulletVelocity) * kBulletSpeed; // 速度を一定に

	// 反動の計算
	Vector3 recoilDirection = -forward;
	outRecoilVelocity = recoilDirection * kRecoilStrength;

	// 画面の揺れを設定
	outShakeIntensity = kInitialShakeIntensity;

	// マシンガン弾を生成して返す
	return std::make_unique<PlayerMachineGun>(playerPosition, bulletVelocity);
}

///=============================================================================
///                        当たり判定の実装
///=============================================================================
void PlayerMachineGun::OnCollisionEnter(BaseObject *other) {
	// 敵接触
	if (BaseEnemy *enemy = dynamic_cast<BaseEnemy *>(other)) {

		//particleEmitter_->Emit();

		hitSprite_->SetPosition(worldTransform_->transform.translate);
		hitSprite_->Emit();

		//---------------------------------------
		// 弾を消す
		isActive_ = false;
	}
}

void PlayerMachineGun::OnCollisionStay(BaseObject *other) {
	// 今回は特に処理なし
}

void PlayerMachineGun::OnCollisionExit(BaseObject *other) {
	// 今回は特に処理なし
}
