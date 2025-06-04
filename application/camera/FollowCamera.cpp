#define _USE_MATH_DEFINES
#define NOMINMAX
#include "FollowCamera.h"
#include "BaseEnemy.h"
#include "Input.h"
#include "MyMath.h"
#include <algorithm>
#include <cmath>

///=============================================================================
///                        初期化
void FollowCamera::Initialize() {
	// カメラの初期オフセット設定
	offset_ = defaultOffset_;

	// 基底クラスの初期化
	BaseCamera::Initialize();

	// 初期位置を設定
	if (target_) {
		currentPosition_ = target_->transform.translate + offset_;
		viewProjection_->transform.translate = currentPosition_;
	}
	// 軽く傾ける
	viewProjection_->transform.rotate = {0.1f, 0.0f, 0.0f};
}

///=============================================================================
///                        更新
void FollowCamera::Update() {
	// ターゲットが設定されていない場合はエラー
	if (target_ == nullptr) {
		assert(0);
	}

	if (!enableUpdate_)
	{
		return;
	}

	// エイムアシスト処理
	UpdateAimAssist();

	// オフセットの計算
	CalculationRotate();

	// 回転の計算
	CalculationTranslate();

	// 高度によるFOV演出
	UpdateHeightFOVEffect();

	// マシンガン発射時のカメラ演出を更新
	UpdateFiringCameraEffect();

	// ミサイル発射時のカメラ演出を更新
	UpdateMissileFiringCameraEffect();

	// 基底クラスの更新
	BaseCamera::Update();
}

///=============================================================================
///                        前方向ベクトルを取得
Vector3 FollowCamera::GetForwardDirection() const {
	// カメラの回転行列から前方向ベクトルを計算
	Matrix4x4 rotateMatrix = MakeRotateMatrix(viewProjection_->transform.rotate);
	Vector3 forward = {0.0f, 0.0f, 1.0f}; // デフォルトの前方向
	return TransformNormal(forward, rotateMatrix);
}

///=============================================================================
///                        カメラからの右方向ベクトルを取得
Vector3 FollowCamera::GetRightDirection() const {
	// カメラの回転行列から右方向ベクトルを計算
	Matrix4x4 rotateMatrix = MakeRotateMatrix(viewProjection_->transform.rotate);
	Vector3 right = {1.0f, 0.0f, 0.0f}; // デフォルトの右方向
	return TransformNormal(right, rotateMatrix);
}

///=============================================================================
///                        カメラからの上方向ベクトルを取得
Vector3 FollowCamera::GetUpDirection() const {
	// カメラの回転行列から上方向ベクトルを計算
	Matrix4x4 rotateMatrix = MakeRotateMatrix(viewProjection_->transform.rotate);
	Vector3 up = {0.0f, 1.0f, 0.0f}; // デフォルトの上方向
	return TransformNormal(up, rotateMatrix);
}

///=============================================================================
///                        オフセットの計算
Vector3 FollowCamera::CalculationOffset() {
	Vector3 offset = offset_;

	Matrix4x4 rotateMatrix = MakeRotateMatrix(viewProjection_->transform.rotate);

	offset = TransformNormal(offset, rotateMatrix);

	return offset;
}

///=============================================================================
///                        回転の計算
void FollowCamera::CalculationRotate() {
	/// 右スティック傾きを取得
	Vector2 rightStickVector = Input::GetInstance()->GetRightStick();

	// プレイヤーの入力がある場合、エイムアシストを弱める
	if (std::abs(rightStickVector.x) > assistInputTriggerThreshold_ ||
		std::abs(rightStickVector.y) > assistInputTriggerThreshold_) {
		DecayAimAssist();
		timeSinceLastInput_ = 0.0f;
	} else {
		// 入力がない時間を更新
		timeSinceLastInput_ += 1.0f / 60.0f; // 60FPSを想定
	}

	Vector2 rotate = {-rightStickVector.y * rotateSpeed_, rightStickVector.x * rotateSpeed_};

	destinationRotate = {destinationRotate.x + rotate.x, destinationRotate.y + rotate.y, destinationRotate.z};

	viewProjection_->transform.rotate = Lerp(viewProjection_->transform.rotate, destinationRotate, easingFactor_);

	// 回転Xを範囲制限
	float clampedX = std::clamp(viewProjection_->transform.rotate.x, -0.8f, 1.5f);
	if (clampedX != viewProjection_->transform.rotate.x) {
		destinationRotate.x = clampedX;
	}
	viewProjection_->transform.rotate.x = clampedX;
}

///=============================================================================
///                        位置の計算
void FollowCamera::CalculationTranslate() {
	interTarget_ = Lerp(interTarget_, target_->transform.translate, easingFactor_);

	Vector3 offset = CalculationOffset();

	viewProjection_->transform.translate = interTarget_ + offset;

	/// カメラの角度の修正が必要無ければ
	if (viewProjection_->transform.translate.y >= 0.1f) {
		return;
	}

	// カメラの高さが0.1以下なら、下方向（上向き）への回転を制限
	while (viewProjection_->transform.translate.y <= 0.1f) {
		// 下向きすぎないように制限（上を向かせすぎない）
		// destinationRotate.x = std::max(destinationRotate.x, 0.1f);

		viewProjection_->transform.rotate.x += 0.001f;

		interTarget_ = Lerp(interTarget_, target_->transform.translate, easingFactor_);

		Vector3 offset = CalculationOffset();

		viewProjection_->transform.translate = interTarget_ + offset;
	}

	// 目標角度を修正
	destinationRotate.x = viewProjection_->transform.rotate.x;
}

///=============================================================================
///                        高度によるFOV演出
void FollowCamera::UpdateHeightFOVEffect() {
	// ターゲットが設定されていない場合は処理しない
	if (!target_)
		return;

	// プレイヤーの高度を取得
	float playerHeight = target_->transform.translate.y;

	// 高度に基づいたFOV調整（Y座標が0.0以上の場合）
	if (playerHeight >= 0.0f) {
		// 高度に応じてFOV増加量を計算
		// 高度が高いほどFOVを広げる（最大でmaxHeightForFOV_まで考慮）
		float heightFactor = std::min(playerHeight / maxHeightForFOV_, 1.0f);

		// 高度に応じたFOV増加を計算
		float fovIncrease = heightFOVIncrease_ * heightFactor;
		targetFOV_ = baseFOV_ + fovIncrease;
	} else {
		// 地上（Y < 0.0）では基本FOVに戻す
		targetFOV_ = baseFOV_;
	}

	// 現在のFOVを目標値へ滑らかに補間
	currentFOV_ = currentFOV_ + (targetFOV_ - currentFOV_) * fovLerpSpeed_;

	// フォールバックチェック（極端な値にならないよう制限）
	currentFOV_ = std::max(baseFOV_, std::min(currentFOV_, baseFOV_ + heightFOVIncrease_));

	// FOVをカメラに適用（ラジアンへの変換が必要な場合）
	viewProjection_->fovY = currentFOV_ * (3.14159f / 180.0f);

	// 現在の高度を保存（必要に応じて）
	prevPlayerHeight_ = playerHeight;
}

///=============================================================================
///                        マシンガン発射時のカメラ演出を更新
void FollowCamera::UpdateFiringCameraEffect() {
	// ミサイル発射中はマシンガン発射中の演出を無効化
	if (isMissileFiring_)
		return;

	Vector3 targetOffset = defaultOffset_;

	// マシンガン発射中判定を更新
	// RTボタンが押されている間だけ近づける
	if (Input::GetInstance()->PushKey(DIK_J) ||
		Input::GetInstance()->PushGamePadButton(Input::GamePadButton::LEFT_SHOULDER)) {
		isFiring_ = true;
		// Z方向（距離）のみ調整して近づける
		targetOffset.z = defaultOffset_.z * firingOffsetFactor_;
	} else {
		isFiring_ = false;
	}

	// 現在のオフセットを目標値に滑らかに補間
	offset_ = Lerp(offset_, targetOffset, firingLerpSpeed_);
}

///=============================================================================
///                        ミサイル発射時のカメラ演出を更新
void FollowCamera::UpdateMissileFiringCameraEffect() {
	Vector3 targetOffset = defaultOffset_;

	// ミサイル発射ボタンが押された時に発射状態を開始
	if (Input::GetInstance()->PushKey(DIK_SPACE) ||
		Input::GetInstance()->PushGamePadButton(Input::GamePadButton::RIGHT_SHOULDER)) {
		isMissileFiring_ = true;
		missileFireTimer_ = 0.0f; // タイマーをリセット
	}

	// ミサイル発射中の処理（数秒間継続）
	if (isMissileFiring_) {
		// カメラを遠ざける
		targetOffset.z = defaultOffset_.z * missileFiringOffsetFactor_;

		// タイマーを進める（60FPSを想定）
		missileFireTimer_ += 1.0f / 60.0f;

		// 持続時間が経過したら演出を終了
		if (missileFireTimer_ >= missileFireDuration_) {
			isMissileFiring_ = false;
		}
	}

	// 現在のオフセットを目標値に滑らかに補間
	offset_ = Lerp(offset_, targetOffset, missileFiringLerpSpeed_);
}

///=============================================================================
///                        エイムアシスト処理
void FollowCamera::UpdateAimAssist() {
	// エイムアシストが無効または敵リストが存在しない場合は処理しない
	if (aimAssistFactor_ <= 0.0f || !enemies_ || !spawns_) {
		return;
	}

	// マシンガン射撃時のみエイムアシストを有効にする設定がONで、射撃していない場合は処理しない
	if (enableAimAssistOnlyWhileFiring_ && !isFiring_) {
		// 射撃していない時はアシスト強度をリセット
		currentAimAssistStrength_ = 0.0f;
		currentAimTarget_ = nullptr;
		targetSwitchCooldown_ = 0.0f;
		return;
	}

	// ターゲット切り替えクールダウンを更新
	if (targetSwitchCooldown_ > 0.0f) {
		targetSwitchCooldown_ -= 1.0f / 60.0f;
	}

	// 最も近い敵を見つける
	BaseEnemy *nearestEnemy = FindNearestEnemyInSight();

	// アシスト対象が変わった場合の処理
	if (nearestEnemy != currentAimTarget_) {
		// クールダウン中でない場合、または現在のターゲットが無効/死亡している場合のみ切り替え
		if (targetSwitchCooldown_ <= 0.0f ||
			!currentAimTarget_ ||
			currentAimTarget_->GetHp() <= 0 ||
			!IsEnemyInAssistRange(currentAimTarget_->GetPosition())) {

			currentAimTarget_ = nearestEnemy;
			currentAimAssistStrength_ = 0.0f;
			targetSwitchCooldown_ = targetSwitchCooldownTime_;
		}
	}

	// アシスト対象が存在しない場合は終了
	if (!currentAimTarget_ || currentAimTarget_->GetHp() <= 0) {
		currentAimAssistStrength_ = 0.0f;
		return;
	}

	// ターゲットが範囲外になった場合はリセット
	if (!IsEnemyInAssistRange(currentAimTarget_->GetPosition())) {
		currentAimTarget_ = nullptr;
		currentAimAssistStrength_ = 0.0f;
		return;
	}

	// プレイヤーからターゲットへの方向を計算
	Vector3 targetPos = currentAimTarget_->GetPosition();
	Vector3 playerPos = target_->transform.translate;
	Vector3 toTarget = {
		targetPos.x - playerPos.x,
		targetPos.y - playerPos.y,
		targetPos.z - playerPos.z};

	// 距離を計算
	float distance = sqrtf(toTarget.x * toTarget.x + toTarget.y * toTarget.y + toTarget.z * toTarget.z);
	if (distance < 0.001f)
		return; // ゼロ除算回避

	// 正規化
	toTarget.x /= distance;
	toTarget.y /= distance;
	toTarget.z /= distance;

	// Y軸回転（水平方向）の目標角度を計算
	float targetRotationY = atan2f(toTarget.x, toTarget.z);

	// X軸回転（垂直方向）の目標角度を計算
	float distanceXZ = sqrtf(toTarget.x * toTarget.x + toTarget.z * toTarget.z);
	float targetRotationX = -atan2f(toTarget.y, distanceXZ);

	// 射撃中はより早くアシスト強度を上げる
	float effectiveRampUpTime = enableAimAssistOnlyWhileFiring_ && isFiring_ ? assistRampUpTime_ * 0.3f : assistRampUpTime_;

	// 入力がないとき、徐々にアシスト強度を上げる
	if (timeSinceLastInput_ > 0.0f && timeSinceLastInput_ < effectiveRampUpTime) {
		currentAimAssistStrength_ = std::min(1.0f, timeSinceLastInput_ / effectiveRampUpTime);
	} else if (timeSinceLastInput_ >= effectiveRampUpTime) {
		currentAimAssistStrength_ = 1.0f;
	}

	// エイムアシストの強さを適用
	float effectiveAssistStrength = currentAimAssistStrength_ * aimAssistFactor_;

	// 射撃中はアシスト効果を強化
	if (enableAimAssistOnlyWhileFiring_ && isFiring_) {
		effectiveAssistStrength *= 1.5f;								   // 射撃中は50%強化
		effectiveAssistStrength = std::min(effectiveAssistStrength, 1.5f); // 上限は1.5
	}

	// 距離による補正（近い敵ほど強いアシスト）
	float distanceFactor = std::max(0.3f, 1.0f - (distance / aimAssistRange_));
	effectiveAssistStrength *= distanceFactor;

	// 現在の角度と目標角度の差分を計算
	float deltaY = targetRotationY - destinationRotate.y;

	// -πからπの範囲に正規化（改良版）
	while (deltaY > static_cast<float>(M_PI))
		deltaY -= 2.0f * static_cast<float>(M_PI);
	while (deltaY < -static_cast<float>(M_PI))
		deltaY += 2.0f * static_cast<float>(M_PI);

	float deltaX = targetRotationX - destinationRotate.x;

	// 角度差による補正をより緩やかに
	float absAngleY = fabsf(deltaY * (180.0f / static_cast<float>(M_PI)));
	float angleFactorY = 1.0f;
	if (absAngleY > assistInnerAngle_) {
		float angleFalloff = (absAngleY - assistInnerAngle_) / (aimAssistAngle_ - assistInnerAngle_);
		angleFactorY = std::max(0.1f, 1.0f - angleFalloff * 0.7f); // 完全に0にせず、0.1まで
	}

	float absAngleX = fabsf(deltaX * (180.0f / static_cast<float>(M_PI)));
	float angleFactorX = 1.0f;
	if (absAngleX > assistInnerAngle_) {
		float angleFalloff = (absAngleX - assistInnerAngle_) / (aimAssistAngle_ - assistInnerAngle_);
		angleFactorX = std::max(0.1f, 1.0f - angleFalloff * 0.7f);
	}

	// 最終的なアシスト回転量を計算
	float assistRotationY = deltaY * aimAssistSpeed_ * effectiveAssistStrength * angleFactorY;
	float assistRotationX = deltaX * aimAssistSpeed_ * effectiveAssistStrength * angleFactorX;

	// より滑らかな補間を適用
	float smoothFactor = 0.8f;
	assistRotationY *= smoothFactor;
	assistRotationX *= smoothFactor;

	// アシスト効果を目標角度に適用
	destinationRotate.y += assistRotationY;
	destinationRotate.x += assistRotationX;

	// X回転を範囲内に制限
	destinationRotate.x = std::clamp(destinationRotate.x, -0.8f, 1.5f);
}

///=============================================================================
///                        最も近い敵を見つける
BaseEnemy *FollowCamera::FindNearestEnemyInSight() {
	if (!enemies_ || !spawns_) {
		return nullptr;
	}

	BaseEnemy *nearestEnemy = nullptr;
	float bestScore = FLT_MAX;

	// プレイヤー位置
	Vector3 playerPos = target_->transform.translate;

	// カメラの前方向ベクトル
	Vector3 cameraForward = GetForwardDirection();

	// 全ての敵をチェック
	auto checkEnemy = [&](BaseEnemy *enemy) {
		if (enemy && enemy->GetHp() > 0) {
			Vector3 enemyPos = enemy->GetPosition();
			Vector3 toEnemy = {
				enemyPos.x - playerPos.x,
				enemyPos.y - playerPos.y,
				enemyPos.z - playerPos.z};

			float distance = sqrtf(toEnemy.x * toEnemy.x + toEnemy.y * toEnemy.y + toEnemy.z * toEnemy.z);

			// 距離が範囲内か
			if (distance < aimAssistRange_ && distance > 0.001f) {
				// 正規化
				Vector3 toEnemyNorm = {
					toEnemy.x / distance,
					toEnemy.y / distance,
					toEnemy.z / distance};

				// カメラの前方向と敵への方向の角度を計算
				float dotProduct = cameraForward.x * toEnemyNorm.x +
								   cameraForward.y * toEnemyNorm.y +
								   cameraForward.z * toEnemyNorm.z;

				dotProduct = std::clamp(dotProduct, -1.0f, 1.0f);
				float angle = acosf(dotProduct) * (180.0f / static_cast<float>(M_PI));

				// 視野角内かつスコア計算
				if (angle < aimAssistAngle_) {
					// スコア計算：角度を重視し、距離も考慮
					float angleWeight = 3.0f;	 // 角度の重要度を上げる
					float distanceWeight = 0.5f; // 距離の重要度を下げる
					float score = (angle * angleWeight) + (distance * distanceWeight);

					// 現在のターゲットに対してはボーナスを与える（ターゲット切り替えを抑制）
					if (enemy == currentAimTarget_) {
						score *= 0.7f; // 30%のボーナス
					}

					if (score < bestScore) {
						bestScore = score;
						nearestEnemy = enemy;
					}
				}
			}
		}
	};

	// 通常の敵をチェック
	if (enemies_) {
		for (const auto &enemyPtr : *enemies_) {
			checkEnemy(enemyPtr.get());
		}
	}

	// スポーン敵をチェック
	if (spawns_) {
		for (const auto &spawnPtr : *spawns_) {
			checkEnemy(spawnPtr.get());
		}
	}

	return nearestEnemy;
}

///=============================================================================
///                        エイムアシストの減衰（プレイヤー入力時）
void FollowCamera::DecayAimAssist() {
	currentAimAssistStrength_ *= aimAssistDecayRate_;
	if (currentAimAssistStrength_ < 0.01f) {
		currentAimAssistStrength_ = 0.0f;
	}
}

///=============================================================================
///                        敵がエイムアシスト対象となる角度内かチェック
bool FollowCamera::IsEnemyInAssistRange(const Vector3 &enemyPos) {
	// プレイヤーの位置
	Vector3 playerPos = target_->transform.translate;

	// カメラの前方向
	Vector3 cameraForward = GetForwardDirection();

	// プレイヤーから敵への方向
	Vector3 toEnemy = enemyPos - playerPos;
	float distance = Length(toEnemy);

	// 距離が範囲外なら対象外
	if (distance > aimAssistRange_) {
		return false;
	}

	// 正規化
	Vector3 toEnemyNorm = {0.0f, 0.0f, 0.0f};
	if (distance > 0.001f) { // ゼロ除算を避ける
		toEnemyNorm.x = toEnemy.x / distance;
		toEnemyNorm.y = toEnemy.y / distance;
		toEnemyNorm.z = toEnemy.z / distance;
	}

	// カメラの前方向と敵への方向のドット積から角度を算出
	float dotProduct = Dot(cameraForward, toEnemyNorm);
	float angle = std::acos(std::clamp(dotProduct, -1.0f, 1.0f)) * (180.0f / static_cast<float>(M_PI)); // ラジアンから度に変換

	// 視野角内かどうか
	return angle <= aimAssistAngle_;
}