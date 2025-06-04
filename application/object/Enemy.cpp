#include "Enemy.h"
#include "PlayerMissile.h"
#include "PlayerMachineGun.h"
#include "PlayerMissile.h"
#include <cmath>
#include <algorithm>

///=============================================================================
///						初期化
void Enemy::Initialize() {
	// モデルを読み込む
	ModelManager::GetInstance()->LoadModel("enemy/enemy.obj");
	BaseEnemy::Initialize(ModelManager::GetInstance()->FindModel("enemy/enemy.obj")); // モデルを読み込む

}

///=============================================================================
///						描画
void Enemy::Update() {

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

	// 弾の発射処理
	intervalCounter_ += 1.0f / 60.0f;
	if (kIntervalTiem <= intervalCounter_ && currentState_ == ActionState::Combat) {
		Fire();
		intervalCounter_ = fmod(intervalCounter_, kIntervalTiem);
	}

	// ジャンプ処理
	if (worldTransform_->transform.translate.y > 0.0f || jumpVelocity_ > 0.0f) {
		jumpVelocity_ -= 0.01f; // 重力
		worldTransform_->transform.translate.y += jumpVelocity_;

		// 地面に戻ったら停止
		if (worldTransform_->transform.translate.y < 0.0f) {
			worldTransform_->transform.translate.y = 0.0f;
			jumpVelocity_ = 0.0f;
		}
	}

	BaseEnemy::Update();
}

///=============================================================================
///						描画
void Enemy::Draw(ViewProjection viewProjection, DirectionalLight directionalLight, PointLight pointLight, SpotLight spotLight) {
	BaseEnemy::Draw(viewProjection, directionalLight, pointLight, spotLight);
}

///=============================================================================
///						当たり判定
///--------------------------------------------------------------
///						接触開始処理
void Enemy::OnCollisionEnter(BaseObject* other) {
	if (dynamic_cast<PlayerMissile*>(other)) {
		--hp_;
		HitJump(); // ミサイルに当たったらジャンプ
	}
	if (dynamic_cast<PlayerMachineGun*>(other)) {
		--hp_;
		HitJump(); // ミサイルに当たったらジャンプ
	}
}

///--------------------------------------------------------------
///						接触継続処理
void Enemy::OnCollisionStay(BaseObject* other) {
}

///--------------------------------------------------------------
///						接触終了処理
void Enemy::OnCollisionExit(BaseObject* other) {
}

void Enemy::HitJump() {
	//ジャンプ
	  // 簡単なジャンプ処理：ジャンプ速度を加算
	if (worldTransform_->transform.translate.y <= 0.1f) { // 地面にいる時だけジャンプ
		jumpVelocity_ = 0.3f; // ジャンプ力（調整可）
	}
}

// 状態の更新
void Enemy::UpdateActionState() {
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

// 徘徊状態の更新
void Enemy::UpdateWanderState() {
	BaseEnemy::RandomMove(modelScale_);
}

// 追跡状態の更新
void Enemy::UpdateChaseState() {
	BaseEnemy::MoveToTarget();
}

// 戦闘状態の更新
void Enemy::UpdateCombatState() {
	if (target_) {
		// ターゲットに向かうベクトルを計算
		Vector3 toTarget = target_->transform.translate - worldTransform_->transform.translate;
		float distance = Length(toTarget);
		Vector3 direction = Normalize(toTarget);

		// ストラッフィング（横移動）のベクトルを計算
		Vector3 strafeDir = { direction.z, 0.0f, -direction.x }; // 90度回転

		// 時間とともに方向を変えるための係数（-1から1の範囲）
		float strafeFactor = sinf(stateTimer_ * 0.5f);

		// 距離に基づいて行動を決定
		Vector3 moveDirection;

		if (distance < safeDistance_) {
			// 近すぎる場合、後退
			//moveDirection = -direction * 0.7f + strafeDir * strafeFactor * 0.3f;
			moveDirection.x = -direction.x * 0.7f + strafeDir.x * strafeFactor * 0.3f;
			moveDirection.y = 0.0f;
			moveDirection.z = -direction.z * 0.7f + strafeDir.z * strafeFactor * 0.3f;
		} else if (distance > combatDistance_) {
			// 遠すぎる場合、接近
			moveDirection = direction * 0.7f + strafeDir * strafeFactor * 0.3f;
		} else {
			// 適切な距離の場合、ストラッフィングのみ
			moveDirection = strafeDir * strafeFactor;
		}

		// 速度を更新
		velocity_ = Normalize(moveDirection) * speed_ * 0.8f;

		// 位置を更新
		worldTransform_->transform.translate = worldTransform_->transform.translate + velocity_;

		// 常にターゲットの方を向く
		float targetRotationY = std::atan2(direction.x, direction.z);
		worldTransform_->transform.rotate.y = targetRotationY;
	}
}
