#include "GroundTypeEnemy2.h"
#include <PlayerMachineGun.h>
#include <PlayerMissile.h>

void GroundTypeEnemy2::Initialize() {
	ModelManager::GetInstance()->LoadModel("enemy/Refrigerator/Refrigerator.obj");
	BaseEnemy::Initialize(ModelManager::GetInstance()->FindModel("enemy/Refrigerator/Refrigerator.obj"));

	worldTransform_->transform.scale = (Vector3(modelScale_, modelScale_, modelScale_));

	particleEmitter_->SetParticleCount(20);
	particleEmitter_->SetLifeTimeRange(ParticleManager::Range({1.0f, 1.0f}));
	particleEmitter_->SetVelocityRange(ParticleManager::Vec3Range({-3.0f, -3.0f, -3.0f}, {3.0f, 3.0f, 3.0f}));

	AudioManager::GetInstance()->SoundLoadFile("Resources/se/爆発1.mp3");
	se1_ = std::make_unique<Audio>();
	se1_->Initialize();
}

void GroundTypeEnemy2::Update() {
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
		if (t > 1.0f)
			t = 1.0f;

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

void GroundTypeEnemy2::Draw(ViewProjection viewProjection, DirectionalLight directionalLight, PointLight pointLight, SpotLight spotLight) {

	BaseEnemy::Draw(viewProjection, directionalLight, pointLight, spotLight);
}

void GroundTypeEnemy2::Attack() {
}

void GroundTypeEnemy2::OnCollisionEnter(BaseObject *other) {
	if (dynamic_cast<PlayerMissile *>(other)) {
		se1_->SoundPlay("Resources/se/爆発1.mp3", 0);
		--hp_;
		HitJump();
		particleEmitter_->Emit();
	}
	if (dynamic_cast<PlayerMachineGun *>(other)) {
		--hp_;
		HitJump();
		particleEmitter_->Emit();
	}
}

void GroundTypeEnemy2::OnCollisionStay(BaseObject *other) {
}

void GroundTypeEnemy2::OnCollisionExit(BaseObject *other) {
}

void GroundTypeEnemy2::HitJump() {
	isHitReacting_ = true;
	hitReactionTimer_ = 0.0f;
	modelScale_ = modelScale_ / 2.0f;
}

void GroundTypeEnemy2::UpdateWanderState() {

	BaseEnemy::RandomMove(modelScale_);
}

void GroundTypeEnemy2::UpdateChaseState() {
	if (target_) {
		BaseEnemy::MoveToTarget(); // BaseEnemyの追尾ロジックを利用
		worldTransform_->transform.scale = {modelScale_, modelScale_, modelScale_};
		SetModelColor(Vector4{1.0f, 1.0f, 1.0f, 1.0f});
	}
}

void GroundTypeEnemy2::UpdateCombatState() {
	if (target_) {
		BaseEnemy::MoveToTarget(); // BaseEnemyの追尾・距離維持ロジックを利用

		Vector3 toTarget = target_->transform.translate - worldTransform_->transform.translate;
		float distanceToTarget = Length(toTarget);

		// 点滅エフェクトと攻撃準備（プレイヤーが安全距離より少し内側に入ったら）
		if (distanceToTarget < safeDistance_ + 5.0f) { // safeDistance_ より少し広い範囲で反応
			if (!isBlinkEffect_) {
				isBlinkEffect_ = true;
				blinkEffectTimer_ = 0.0f;
			}
		}

		if (isBlinkEffect_) {
			blinkEffectTimer_ += 1.0f / 60.0f;

			float t = blinkEffectTimer_;
			float scale = modelScale_ + 0.35f * sinf(t * 10.0f); // 点滅中の拡縮
			worldTransform_->transform.scale = {scale, scale, scale};

			int blinkPhase = static_cast<int>(t * 5.0f) % 2;
			if (blinkPhase == 0) {
				SetModelColor({1.0f, 1.0f, 1.0f, 1.0f});
			} else {
				SetModelColor({1.0f, 0.0f, 0.0f, 1.0f}); // 赤く点滅
			}

			if (blinkEffectTimer_ >= blinkEffectDuration_) {
				isBlinkEffect_ = false;
				blinkEffectTimer_ = 0.0f;
				worldTransform_->transform.scale = {modelScale_, modelScale_, modelScale_};
				SetModelColor({1.0f, 1.0f, 1.0f, 1.0f});
				// ここで攻撃実行や、次の行動への移行など
				// 今回は点滅終了でリセットのみ
			}
		} else {
			// 点滅していない時は通常のスケールと色
			worldTransform_->transform.scale = {modelScale_, modelScale_, modelScale_};
			SetModelColor({1.0f, 1.0f, 1.0f, 1.0f});
		}
	}
}

void GroundTypeEnemy2::UpdateActionState() {
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
