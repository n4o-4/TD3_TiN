#include "GroundTypeEnemy3.h"
#include "Player.h"
#include "PlayerMachineGun.h"
#include "PlayerMissile.h"
#include <algorithm>
#include <cmath>

void GroundTypeEnemy3::Initialize() {
	ModelManager::GetInstance()->LoadModel("enemy/cp/cp.obj");
	BaseEnemy::Initialize(ModelManager::GetInstance()->FindModel("enemy/cp/cp.obj"));

	worldTransform_->transform.scale = (Vector3(modelScale_, modelScale_, modelScale_));

	particleEmitter_->SetParticleCount(20);
	particleEmitter_->SetLifeTimeRange(ParticleManager::Range({1.0f, 1.0f}));
	particleEmitter_->SetVelocityRange(ParticleManager::Vec3Range({-3.0f, -3.0f, -3.0f}, {3.0f, 3.0f, 3.0f}));

	AudioManager::GetInstance()->SoundLoadFile("Resources/se/爆発1.mp3");
	se1_ = std::make_unique<Audio>();
	se1_->Initialize();
}

void GroundTypeEnemy3::Update() {
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

void GroundTypeEnemy3::Draw(ViewProjection viewProjection, DirectionalLight directionalLight, PointLight pointLight, SpotLight spotLight) {
	BaseEnemy::Draw(viewProjection, directionalLight, pointLight, spotLight);
}

void GroundTypeEnemy3::PushTargetAway() {
	if (!target_)
		return;
	// 現在地を保存
	Vector3 enemyPos = worldTransform_->transform.translate;
	Vector3 targetPos = target_->transform.translate;

	// 敵→ターゲット方向
	Vector3 direction = targetPos - enemyPos;
	direction.y = 0.0f; // y軸の影響を取り除く

	// 方向ベクトルの正規化
	direction = Normalize(direction);

	// 押し出す力
	const float pushStrength = 30.5f;

	// y軸値の保存
	float originalY = target_->transform.translate.y;

	// 押し出す
	target_->transform.translate += direction * pushStrength;

	// y軸の復元
	target_->transform.translate.y = originalY;
}

void GroundTypeEnemy3::OnCollisionEnter(BaseObject *other) {
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

void GroundTypeEnemy3::OnCollisionStay(BaseObject *other) {
}

void GroundTypeEnemy3::OnCollisionExit(BaseObject *other) {
}

void GroundTypeEnemy3::HitJump() {
	isHitReacting_ = true;
	hitReactionTimer_ = 0.0f;
	modelScale_ = modelScale_ / 2.0f;
}

void GroundTypeEnemy3::UpdateWanderState() {
	BaseEnemy::RandomMove(modelScale_);
}

void GroundTypeEnemy3::UpdateChaseState() {
	if (target_) {
		BaseEnemy::MoveToTarget(); // BaseEnemyの追尾・距離維持ロジックを利用
		// モデルの色やスケール設定は必要に応じてここで行う
		SetModelColor(Vector4{1.0f, 1.0f, 1.0f, 1.0f});
		worldTransform_->transform.scale = {modelScale_, modelScale_, modelScale_};
	}
}

void GroundTypeEnemy3::UpdateCombatState() {
	if (!target_)
		return;

	BaseEnemy::MoveToTarget(); // BaseEnemyの追尾・距離維持ロジックを利用

	// ターゲットに向かうベクトルを計算 (PushTargetAwayの条件判定用)
	Vector3 toTargetForPush = target_->transform.translate - worldTransform_->transform.translate;

	// y軸の差を確認（高さがほぼ同じかチェック）
	float yDiff = std::abs(toTargetForPush.y);
	const float yTolerance = 3.0f; // 高さの差が3以下のときのみ押すことができます

	// 水平方向のベクトル（XZ平面のみで距離計算）
	toTargetForPush.y = 0.0f;
	float distanceForPush = Length(toTargetForPush);

	// 一定距離内にあり、y軸もほぼ同じときだけ押し出す
	const float pushThreshold = 1.0f; // この閾値は敵のサイズやゲームデザインに応じて調整
	if (distanceForPush < pushThreshold && yDiff < yTolerance) {
		PushTargetAway();
	}

	// カラー変更(戦闘状態用)
	SetModelColor(Vector4{0.4f, 0.0f, 0.0f, 1.0f});
	worldTransform_->transform.scale = {modelScale_, modelScale_, modelScale_};
}

void GroundTypeEnemy3::UpdateActionState() {
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
