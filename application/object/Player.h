#pragma once
#include "BaseObject.h" // 当たり判定用
#include "FollowCamera.h"
#include "Kouro.h"
#include "LockOn.h"
#include "MyMath.h"
#include "PlayerMachineGun.h" // プレイヤーのマシンガン
#include "PlayerMissile.h"
#include "memory"
#include <vector>

class Player : public BaseObject {
public:
	//==============================================================================
	// メンバ関数
	//==============================================================================
	/// @brief 初期化
	void Initialize();
	/// @brief 更新
	void Update();
	/// @brief 描画
	/// @param viewProjection ビュー射影行列
	/// @param directionalLight ディレクショナルライト
	/// @param pointLight ポイントライト
	/// @param spotLight スポットライト
	void Draw(ViewProjection viewProjection, DirectionalLight directionalLight, PointLight pointLight, SpotLight spotLight);
	/// @brief 終了処理
	void Finalize();
	/// @brief ImGui描画
	void DrawImGui();

	//==============================================================================
	// 静的メンバ関数
	//==============================================================================
private:
	//==============================================================================
	// 移動処理
	//==============================================================================
	/// @brief 移動入力の取得
	Vector3 GetMovementInput();
	/// @brief 移動処理を更新
	void UpdateMove(Vector3 direction);

	//==============================================================================
	// ジャンプ処理
	//==============================================================================
	/// @brief ジャンプ処理を更新
	void UpdateJump();

	//==============================================================================
	// 弾の処理
	//==============================================================================
	/// @brief 弾の処理と更新
	void UpdateBullets();
	/// @brief ミサイル専用の更新処理
	void UpdateMissiles();
	/// @brief マシンガンと熱量専用の更新処理
	void UpdateMachineGunAndHeat();
	/// @brief 射撃
	void Shoot();
	/// @brief マシンガンの射撃処理
	void ShootMachineGun();

	//==============================================================================
	// ブースト処理
	//==============================================================================
	/// @brief ブースト処理
	bool HandleBoost();
	/// @brief クイックブーストのクールダウンやリチャージ管理
	void UpdateQuickBoostCooldowns();
	/// @brief クイックブースト中の処理
	bool ProcessActiveQuickBoost();
	/// @brief クイックブーストの入力判定と起動処理
	bool HandleQuickBoostActivation();
	/// @brief ブーストエネルギーの回復処理
	void RecoverBoostEnergy();

	//==============================================================================
	// 反動・揺れ処理
	//==============================================================================
	/// @brief 反動処理
	void ApplyRecoil();
	/// @brief プレイヤーの揺れ処理
	void ApplyShake();

	//==============================================================================
	// カメラの揺れ処理
	//==============================================================================
	/// @brief カメラの揺れ処理
	void ApplyCameraShake();
	/// @brief カメラシェイク
	void CameraShake();

	//==============================================================================
	// ドア処理
	//==============================================================================
	/// @brief ドア開閉
	void OpenDoor();

	//==============================================================================
	// 浮遊処理
	//==============================================================================
	/// @brief プレイヤーの浮遊演出適用
	void ApplyFloating();

	//==============================================================================
	// 当たり判定
	//==============================================================================
	/// @brief 衝突開始時の処理
	void OnCollisionEnter(BaseObject *other) override;
	/// @brief 衝突継続時の処理
	void OnCollisionStay(BaseObject *other) override;
	/// @brief 衝突終了時の処理
	void OnCollisionExit(BaseObject *other) override;
	/// @brief ダメージ処理と無敵化の共通処理
	void HandleDamageAndInvincibility();

	//==============================================================================
	// 入出力関数
	//==============================================================================
public:
	/// @brief カメラをセット
	/// @param followCamera カメラ
	void SetFollowCamera(FollowCamera *followCamera) {
		followCamera_ = followCamera;
	}

	/// @brief ロックオンシステムをセット
	/// @param lockOnSystem ロックオンシステム
	void SetLockOnSystem(LockOn *lockOnSystem) {
		lockOnSystem_ = lockOnSystem;
	}

	/// @brief 位置の取得
	/// @return 位置
	Vector3 GetPosition() {
		return objectTransform_->transform.translate;
	}

	/// @brief ワールドトランスフォームの取得
	/// @return ワールドトランスフォーム
	WorldTransform *GetWorldTransform() {
		return objectTransform_.get();
	}

	/// @brief 弾の取得
	/// @return 弾のリスト
	std::vector<std::unique_ptr<PlayerMissile>> &GetBullets() {
		return bullets_;
	}

	/// @brief マシンガンの弾の取得
	/// @return マシンガンの弾のリスト
	std::vector<std::unique_ptr<PlayerMachineGun>> &GetMachineGunBullets() {
		return machineGunBullets_;
	}

	/// @brief HPの取得
	/// @return HP
	int GetHp() {
		return hp_;
	}

	/// @brief HPの設定
	/// @param hp HP
	void SetHp(int hp) {
		hp_ = hp;
	}

	/// @brief 無敵状態の取得
	/// @return 無敵状態
	bool GetIsInvincible() const {
		return isInvincible_;
	}

	// HUD表示用のゲッター関数
	/// @brief 現在のブースト時間取得
	float GetCurrentBoostTime() const {
		return currentBoostTime_;
	}
	/// @brief 最大ブースト時間取得
	float GetMaxBoostTime() const {
		return maxBoostTime_;
	}
	/// @brief クイックブースト使用回数取得
	int GetQuickBoostUsedCount() const {
		return quickBoostUsedCount_;
	}
	/// @brief 最大クイックブースト使用回数取得
	int GetMaxQuickBoostUses() const {
		return maxQuickBoostUses_;
	}
	/// @brief クイックブーストチャージクールダウン取得
	int GetQuickBoostChargeCooldown() const {
		return quickBoostChargeCooldown_;
	}
	/// @brief クイックブーストチャージ時間取得
	int GetQuickBoostChargeTime() const {
		return quickBoostChargeTime_;
	}

	/// @brief 熱量レベル取得
	float GetHeatLevel() const {
		return heatLevel_;
	}
	/// @brief 最大熱量取得
	float GetMaxHeat() const {
		return maxHeat_;
	}
	/// @brief オーバーヒート状態取得
	bool IsOverheated() const {
		return isOverheated_;
	}
	/// @brief マシンガンクールダウン取得
	int GetMachineGunCooldown() const {
		return machineGunCooldown_;
	}
	/// @brief マシンガン発射間隔取得
	static int GetMachineGunFireInterval() {
		return kMachineGunFireInterval_;
	}

	/// @brief ミサイルクールダウン取得
	int GetMissileCooldown() const {
		return missileCooldown_;
	}
	/// @brief ミサイル最大クールダウン取得
	int GetMissileCooldownMax() const {
		return missileCooldownMax_;
	}

	/// @brief 速度取得
	Vector3 GetVelocity() const {
		return velocity_;
	}

	/// @brief マシンガン音停止
	void StopMachineGunSound();

private:
	//==============================================================================
	// 3Dオブジェクト
	//==============================================================================
	std::unique_ptr<Object3d> object3d_;	   // 3Dオブジェクト
	std::unique_ptr<Object3d> door_;		   // ドア
	std::unique_ptr<Object3d> machineGunBody_; // マシンガンボディ
	std::unique_ptr<Object3d> machineGunHead_; // マシンガンヘッド

	// ワールド変換
	std::unique_ptr<WorldTransform> objectTransform_;		  // オブジェクト
	std::unique_ptr<WorldTransform> doorObjectTransform_;	  // ドア
	std::unique_ptr<WorldTransform> machineGunBodyTransform_; // マシンガンボディ
	std::unique_ptr<WorldTransform> machineGunHeadTransform_; // マシンガンヘッド

	//==============================================================================
	// 弾
	//==============================================================================
	std::vector<std::unique_ptr<PlayerMissile>> bullets_; // 弾のリスト

	//==============================================================================
	// ロックオン
	//==============================================================================
	LockOn *lockOnSystem_ = nullptr; // ロックオンシステム

	//==============================================================================
	// HP
	//==============================================================================
	int hp_ = 10; // HP

	//==============================================================================
	// 移動関連
	//==============================================================================
	Vector3 velocity_ = {0.0f, 0.0f, 0.0f};		// 現在の速度ベクトル
	Vector3 acceleration_ = {0.0f, 0.0f, 0.0f}; // 加速度ベクトル
	float currentInputMagnitude_ = 0.0f;		// 現在の入力の大きさ (0.0 ~ 1.0)
	float maxSpeed_ = 1.35f;					// 最大速度
	float accelerationRate_ = 0.03f;			// 加速度係数
	float friction_ = 0.02f;					// 摩擦
	float boostSpin_ = 0.0f;					// 回転量
	bool isBoostSpinning_ = false;				// 回転中フラグ
	int boostSpinFrame_ = 0;					// 回転フレーム数
	float floatTimer_ = 0.0f;					// 浮遊時間
	const float floatAmplitude_ = 0.1f;			// 上下の振れ幅
	const float floatFrequency_ = 2.0f;			// 1秒あたりの揺れ回数（Hz）

	// ブースト関連
	bool isBoosting_ = false;				  // 通常ブースト中かどうか (クイックブーストとは別)
	float boostFactor_ = 1.8f;				  // 通常ブースト時の基本速度倍率
	float boostAccelerationFactor_ = 2.0f;	  // 通常ブースト時の加速度倍率
	float boostMaxSpeedFactor_ = 2.2f;		  // 通常ブースト時の最大速度倍率 (限界突破用)
	float boostEnergyConsumptionRate_ = 0.5f; // 通常ブーストのエネルギー消費率 (フレーム毎)
	float boostCooldown_ = 0.0f;			  // ブーストのクールダウン
	float maxBoostTime_ = 60.0f;			  // 最大ブースト時間 (エネルギー総量)
	float currentBoostTime_ = 30.0f;		  // 現在のブースト残量
	float boostRecoveryRate_ = 0.15f;		  // ブースト回復速度
	int quickBoostUsedCount_ = 0;			  // 使用した回数
	int quickBoostChargeCooldown_ = 0;		  // クールタイム用カウント（フレーム単位）
	const int maxQuickBoostUses_ = 3;		  // 最大使用回数
	const int quickBoostChargeTime_ = 180;	  // クールタイム（3秒 = 180フレーム）
	int quickBoostRegenTimer_ = 0;			  // クイックブーストのリチャージタイマー
	const int quickBoostRegenInterval_ = 60;  // 1回復までの時間（1秒）

	// クイックブースト関連
	bool isQuickBoosting_ = false;		  // クイックブースト中かどうか
	int quickBoostFrames_ = 0;			  // クイックブーストの残りフレーム
	int maxQuickBoostFrames_ = 10;		  // クイックブーストの最大フレーム数
	float quickBoostCooldown_ = 0.0f;	  // クイックブーストのクールダウン
	float maxQuickBoostCooldown_ = 16.0f; // クイックブーストの最大クールダウン
	float quickBoostConsumption_ = 16.0f; // クイックブーストの消費量

	// 操作感度
	float airControlFactor_ = 0.8f; // 空中操作係数
	bool isQuickTurning_ = false;	// クイックターン中か
	float quickTurnFactor_ = 2.5f;	// クイックターン係数

	//==============================================================================
	// ジャンプ関連
	//==============================================================================
	bool isJumping_ = false;	 // ジャンプ中かどうか
	bool isFloating_ = false;	 // 浮遊中かどうか
	float jumpVelocity_ = 0.2f;	 // 上昇速度
	float fallSpeed_ = 0.0f;	 // 下降速度
	float gravity_ = 0.16f;		 // 下降加速度
	float initialY_ = 1.0f;		 // 初期Y座標
	float floatBoost_ = 0.2f;	 // 離した瞬間の追加上昇量
	float boostVelocity_ = 0.0f; // 追加上昇速度
	float boostDecay_ = 0.02f;	 // 追加上昇の減衰量
	float maxFallSpeed_ = 0.15f; // 下降速度の最大値

	//==============================================================================
	// マシンガン関連
	//==============================================================================
	std::vector<std::unique_ptr<PlayerMachineGun>> machineGunBullets_; // マシンガンの弾のリスト
	bool isShootingMachineGun_ = false;								   // マシンガンを撃っているか
	int machineGunCooldown_ = 0;									   // マシンガンのクールダウン
	bool machineGunSoundPlayed_ = false;							   // マシンガン音再生フラグ

	// オーバーヒート関連
	float heatLevel_ = 0.0f;								  // 現在の熱量
	float maxHeat_ = 100.0f;								  // オーバーヒートライン
	float heatPerShot_ = 5.0f;								  // 1発ごとの加熱量
	float heatCooldownRate_ = 0.8f;							  // 自然冷却速度（毎フレーム）
	bool isOverheated_ = false;								  // オーバーヒート中か
	int overheatRecoveryTime_ = 300;						  // オーバーヒート時のクールダウン（フレーム）
	int overheatTimer_ = 0;									  // クールダウン用カウント
	std::unique_ptr<ParticleEmitter> smokeEmitter_ = nullptr; // 煙のエミッター

	//==============================================================================
	// 反動関連
	//==============================================================================
	Vector3 recoilVelocity_ = {0.0f, 0.0f, 0.0f}; // 反動による速度
	float recoilStrength_ = 0.05f;				  // 反動の強さ
	float recoilDecay_ = 0.9f;					  // 反動の減衰率
	float shakeIntensity_ = 0.0f;				  // 揺れの強さ
	float shakeDecay_ = 0.9f;					  // 揺れの減衰率

	//==============================================================================
	// 無敵時間用
	//==============================================================================
	bool isInvincible_ = false; // 無敵状態
	int invincibleTimer_ = 0;	// 無敵タイマー
	bool isVisible_ = true;		// 点滅制御用

	//==============================================================================
	// カメラ
	//==============================================================================
	FollowCamera *followCamera_ = nullptr;						  // フォローカメラ
	float distinationRotateY_;									  // 目標回転Y
	std::unique_ptr<ParticleEmitter> explosionEmitter_ = nullptr; // 爆発エミッター

	//==============================================================================
	// ロックオン関連
	//==============================================================================
	int missileCooldown_ = 0;			 // ミサイルのクールダウンタイマー
	const int missileCooldownMax_ = 300; // 5秒（60FPS換算）

	//==============================================================================
	// ドア開閉制御用
	//==============================================================================
	bool isDoorOpening_ = false;			 // ドア開放中
	bool isDoorClosing_ = false;			 // ドア閉鎖中
	float doorAngle_ = 0.0f;				 // ドア角度
	const float kDoorOpenAngle_ = 3.0f;		 // ドア開放角度
	const float kDoorCloseAngle_ = 0.0f;	 // ドア閉鎖角度
	const float kDoorOpenCloseSpeed_ = 0.2f; // 補間速度
	int doorOpenTimer_ = 0;					 // ドア開放タイマー
	const int kDoorStayOpenFrames_ = 60;	 // 開いてから閉じるまでの時間

	// 初期化関連
	const float kInitialZ_ = 3.0f;			  // 初期Z座標
	const int kExplosionParticleCount_ = 10;  // 爆発パーティクル数
	const float kExplosionFrequency_ = 0.04f; // 爆発頻度
	const float kExplosionLifeTime_ = 0.01f;  // 爆発ライフタイム

	// 回転関連
	const float kBoostSpinSpeed_ = 0.4f;	  // ブーストスピン速度
	const float kTwoPI_ = 2.0f * 3.14159265f; // 2π
	const int kBoostSpinMaxFrames_ = 16;	  // ブーストスピン最大フレーム
	const float kRotationLerpFactor_ = 0.35f; // 回転補間係数

	// 無敵時間関連
	const int kInvincibleBlinkInterval_ = 10; // 無敵時点滅間隔
	const int kInvincibleDuration_ = 60 * 4;  // 無敵時間（4秒間、60FPS換算）

	// 移動関連
	const float kVelocityStopThreshold_ = 0.01f; // 速度停止閾値
	const float kPostBoostSlowdownRate_ = 0.95f; // ブースト後減速率

	// ジャンプ関連
	const float kJumpInitialVelocity_ = 0.6f; // ジャンプ初期速度
	const float kJumpVelocityDecay_ = 0.005f; // ジャンプ速度減衰
	const float kFloatJumpVelocity_ = 0.2f;	  // 浮遊ジャンプ速度

	// ブースト関連
	const float kPreciseLockDirectionFactor_ = 0.7f;		// 精密ロック方向係数
	const float kPreciseLockVerticalFactor_ = 0.3f;			// 精密ロック垂直係数
	const float kPreciseLockInitialSpeed_ = 0.35f;			// 精密ロック初期速度
	const float kSimpleLockDirectionFactor_ = 0.3f;			// 簡易ロック方向係数
	const float kSimpleLockVerticalFactor_ = 0.7f;			// 簡易ロック垂直係数
	const float kSimpleLockInitialSpeed_ = 0.25f;			// 簡易ロック初期速度
	const float kQuickBoostSpeedMultiplier_ = 4.0f;			// クイックブースト速度倍率
	const float kQuickBoostSpeedRetainFactor_ = 0.8f;		// クイックブースト速度保持係数
	const float kStationaryBoostRecoveryMultiplier_ = 2.0f; // 静止時ブースト回復倍率

	const float kRecoilThreshold_ = 0.001f; // 反動閾値
	const float kShakeDecayRate_ = 0.92f;	// 揺れ減衰率

	// Player.cppから移動した定数
	static constexpr float kMovementFrictionThreshold_ = 0.1f; // 移動摩擦閾値
	static constexpr float kMovementTiltFactor_ = -0.1f;	   // 移動傾き係数
	static constexpr float kMovementMaxTilt_ = 0.5f;		   // 移動最大傾き
	static constexpr float kMovementTiltLerpFactor_ = 0.2f;	   // 移動傾き補間係数
	static constexpr int kMachineGunFireInterval_ = 5;		   // マシンガン発射間隔
	static constexpr float kShakeBaseIntensityFactor_ = 0.1f;  // 揺れ基本強度係数
	static constexpr float kShakeOffsetXFactor_ = 0.02f;	   // 揺れオフセットX係数
	static constexpr float kShakeOffsetRotFactor_ = 0.002f;	   // 揺れオフセット回転係数
	static constexpr int kShakeRandRangeX_ = 80;			   // 揺れランダム範囲X
	static constexpr int kShakeRandRangeRot_ = 200;			   // 揺れランダム範囲回転

	// カメラシェイク用変数
	static constexpr float kShakeTime_ = 0.1f;		// シェイクの時間
	float shakeCurrentTime_ = 0.0f;					// シェイクの経過時間
	static constexpr float kShakeMagnitude_ = 0.3f; // シェイクの大きさ
	bool isCameraShaking_ = false;					// カメラシェイク中かどうか

	std::mt19937 randomEngine; // 乱数エンジン

	// Audio関連
	std::unique_ptr<Audio> se1_ = nullptr; // 効果音1
	std::unique_ptr<Audio> se2_ = nullptr; // 効果音2
	std::unique_ptr<Audio> se3_ = nullptr; // 効果音3
};