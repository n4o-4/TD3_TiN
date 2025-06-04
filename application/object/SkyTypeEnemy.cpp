#include "SkyTypeEnemy.h"
#include "PlayerMissile.h"
#include <PlayerMachineGun.h>

void SkyTypeEnemy::Initialize() {
	ModelManager::GetInstance()->LoadModel("enemy/Iron/Iron.obj");
	BaseEnemy::Initialize(ModelManager::GetInstance()->FindModel("enemy/Iron/Iron.obj"));

	worldTransform_->transform.scale = (Vector3(modelScale_, modelScale_, modelScale_));

	particleEmitter_->SetParticleCount(20);
	particleEmitter_->SetLifeTimeRange(ParticleManager::Range({ 1.0f,1.0f }));
	particleEmitter_->SetVelocityRange(ParticleManager::Vec3Range({ -3.0f,-3.0f,-3.0f }, {3.0f,3.0f,3.0f}));

	AudioManager::GetInstance()->SoundLoadFile("Resources/se/爆発1.mp3");
	se1_ = std::make_unique<Audio>();
	se1_->Initialize();
}

void SkyTypeEnemy::Update() {
	if (hp_ > 0) {
		// 状態の更新
		UpdateActionState();

		// 現在の状態に応じた行動
		switch (currentState_) {
		case ActionState::Wander:
			UpdateWanderState();
			break;
		case ActionState::Chase:
			UpdateChaseState();
			break;
		case ActionState::Combat:
			UpdateCombatState();
			break;
		}
	}

	
	if (isHitReacting_) {
		hitReactionTimer_ += 1.0f / 60.0f;

		const float kTotalReactionTime = 0.3f; // 全体時間
		float t = hitReactionTimer_ / kTotalReactionTime;

		// clamp
		if (t > 1.0f) t = 1.0f;

		// イージング（バウンド風：t=0で1.0、t=0.5で1.7、t=1.0で1.0）
		float scaleFactor = modelScale_ + 0.7f * sinf(t * 3.141592f); // πで1周期 → 1→1.7→1

		worldTransform_->transform.scale.x = scaleFactor;
		worldTransform_->transform.scale.y = scaleFactor;
		worldTransform_->transform.scale.z = scaleFactor;

		if (t >= 1.0f) {
			isHitReacting_ = false;
			hitReactionTimer_ = 0.0f;
			worldTransform_->transform.scale.x = modelScale_;
			worldTransform_->transform.scale.y = modelScale_;
			worldTransform_->transform.scale.z = modelScale_;
		}
	}




	BaseEnemy::Update();
}

void SkyTypeEnemy::Draw(ViewProjection viewProjection, DirectionalLight directionalLight, PointLight pointLight, SpotLight spotLight) {
	BaseEnemy::Draw(viewProjection, directionalLight, pointLight, spotLight);
}

void SkyTypeEnemy::MoveToRush() {
	if (target_) {
		Vector3 toTarget = target_->transform.translate - worldTransform_->transform.translate;
		Vector3 direction = Normalize(toTarget);

		// 
		velocity_ = direction * rushSpeed_ * rushSpeedMultiplier_;

		// 
		worldTransform_->transform.translate = worldTransform_->transform.translate + velocity_;

		// 
		float targetRotationY = std::atan2(direction.x, direction.z);
		worldTransform_->transform.rotate.y = targetRotationY;

		// 
		rushTimer_ += 1.0f / 60.0f;
		if (rushTimer_ >= rushDuration_) {
			isRushing_ = false; // 
			isRising_ = true;   // 
			hasJustRushed_ = true;
			rushTimer_ = 0.0f;
			riseTimer_ = 0.0f;  // 
		}
	}
}

void SkyTypeEnemy::MoveToRise() {
	if (!hasJustRushed_) return; // <- Rushした後でなければ実行禁止

	velocity_.y = riseSpeed_;
	worldTransform_->transform.translate.y += velocity_.y;

	riseTimer_ += 1.0f / 60.0f;
	if (riseTimer_ >= riseDuration_) {
		isRising_ = false;
		hasJustRushed_ = false; // <- 上昇完了後の初期化
		velocity_.y = 0.0f;
	}


}

void SkyTypeEnemy::Attack() {
}

void SkyTypeEnemy::OnCollisionEnter(BaseObject* other) {
	if (dynamic_cast<PlayerMissile*>(other)) {
		se1_->SoundPlay("Resources/se/爆発1.mp3", 0);
		--hp_;
		HitReaction();
		particleEmitter_->Emit();
	}
	if (dynamic_cast<PlayerMachineGun*>(other)) {
		--hp_;
		HitReaction();
		particleEmitter_->Emit();
	}
}

void SkyTypeEnemy::OnCollisionStay(BaseObject* other) {
}

void SkyTypeEnemy::OnCollisionExit(BaseObject* other) {
}

void SkyTypeEnemy::HitReaction()
{
	isHitReacting_ = true;
	hitReactionTimer_ = 0.0f;
	modelScale_ = modelScale_ / 2.0f;
}

void SkyTypeEnemy::UpdateWanderState() {
	BaseEnemy::RandomMove(modelScale_);
}

void SkyTypeEnemy::UpdateChaseState() {
	BaseEnemy::MoveToTarget();
}

void SkyTypeEnemy::UpdateCombatState() {
	// 
	if (isRising_) {
		MoveToRise();
		return;
	}

	//// 
	if (isRushing_) {
		MoveToRush();
		return;
	}

	// 
	Vector3 toTarget = target_->transform.translate - worldTransform_->transform.translate;
	toTarget.y = 0.0f;
	float distance = Length(toTarget);

	// 
	if (distance < combatDistance_ * 1.5f && !isRushing_ && !isRising_) {
		isRushing_ = true;
		rushTimer_ = 0.0f;
	} else {
		BaseEnemy::MoveToTarget(); // 
	}
}

void SkyTypeEnemy::UpdateActionState() {
	// タイマーの更新
	stateTimer_ += 1.0f / 60.0f;

	// ターゲットが存在しない場合は徘徊
	if (!target_) {
		currentState_ = ActionState::Wander;
		return;
	}

	// ターゲットとの距離を計算
	Vector3 toTarget = target_->transform.translate - worldTransform_->transform.translate;
	float distance = Length(toTarget);

	// 距離に基づいて状態を変更
	ActionState newState = currentState_; // デフォルトは現在の状態を維持

	if (distance > chaseDistance_) {
		// プレイヤーが遠い場合は徘徊
		newState = ActionState::Wander;
	} else if (distance > combatDistance_) {
		// 追跡範囲内なら追跡
		newState = ActionState::Chase;
	} else {
		// 戦闘範囲内なら戦闘
		newState = ActionState::Combat;
	}

	// 状態が変化した場合、タイマーをリセット
	if (newState != currentState_) {
		currentState_ = newState;
		stateTimer_ = 0.0f;
	}
}
