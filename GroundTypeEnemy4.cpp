#include "GroundTypeEnemy4.h"
#include <PlayerMissile.h>
#include <PlayerMachineGun.h>

void GroundTypeEnemy4::Initialize() {
	ModelManager::GetInstance()->LoadModel("enemy/WH/WH.obj");
	BaseEnemy::Initialize(ModelManager::GetInstance()->FindModel("enemy/WH/WH.obj"));

	worldTransform_->transform.scale = (Vector3(modelScale_, modelScale_, modelScale_));

	particleEmitter_->SetParticleCount(20);
	particleEmitter_->SetLifeTimeRange(ParticleManager::Range({ 1.0f,1.0f }));
	particleEmitter_->SetVelocityRange(ParticleManager::Vec3Range({ -3.0f,-3.0f,-3.0f }, {3.0f,3.0f,3.0f}));

	AudioManager::GetInstance()->SoundLoadFile("Resources/se/爆発1.mp3");
	se1_ = std::make_unique<Audio>();
	se1_->Initialize();
}

void GroundTypeEnemy4::Update() {
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

		const float kTotalReactionTime = 0.3f; //
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

void GroundTypeEnemy4::Draw(ViewProjection viewProjection, DirectionalLight directionalLight, PointLight pointLight, SpotLight spotLight) {
	BaseEnemy::Draw(viewProjection, directionalLight, pointLight, spotLight);
	
}

void GroundTypeEnemy4::OnCollisionEnter(BaseObject* other) {
	if (dynamic_cast<PlayerMissile*>(other)) {
		se1_->SoundPlay("Resources/se/爆発1.mp3", 0);
		--hp_;
		HitJump();
		particleEmitter_->Emit();
	}
	if (dynamic_cast<PlayerMachineGun*>(other)) {
		--hp_;
		HitJump();
		particleEmitter_->Emit();
	}
}

void GroundTypeEnemy4::OnCollisionStay(BaseObject* other) {
}

void GroundTypeEnemy4::OnCollisionExit(BaseObject* other) {
}

void GroundTypeEnemy4::HitJump() {
	isHitReacting_ = true;
	hitReactionTimer_ = 0.0f;
	modelScale_ = modelScale_ / 2.0f;
}

void GroundTypeEnemy4::UpdateWanderState() {
	BaseEnemy::RandomMove(modelScale_);
}

void GroundTypeEnemy4::UpdateChaseState() {
	BaseEnemy::RandomMove(modelScale_);
}

void GroundTypeEnemy4::UpdateCombatState() {
	BaseEnemy::RandomMove(modelScale_);
}

void GroundTypeEnemy4::UpdateActionState() {
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
