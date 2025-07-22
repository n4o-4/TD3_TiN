#pragma once

class PlayerConstant {
public:
	// 初期化関連
	static constexpr float kInitialZ = 3.0f;			// 初期Z座標
	static constexpr int kExplosionParticleCount = 10;	// 爆発パーティクル数
	static constexpr float kExplosionFrequency = 0.04f; // 爆発頻度
	static constexpr float kExplosionLifeTime = 0.01f;	// 爆発ライフタイム

	// 回転関連
	static constexpr float kBoostSpinSpeed = 0.4f;		// ブーストスピン速度
	static constexpr float kTwoPI = 2.0f * 3.14159265f; // 2π
	static constexpr int kBoostSpinMaxFrames = 16;		// ブーストスピン最大フレーム
	static constexpr float kRotationLerpFactor = 0.35f; // 回転補間係数

	// 無敵時間関連
	static constexpr int kInvincibleBlinkInterval = 10; // 無敵時点滅間隔
	static constexpr int kInvincibleDuration = 60 * 4;	// 無敵時間（4秒間、60FPS換算）

	// 移動関連
	static constexpr float kVelocityStopThreshold = 0.01f; // 速度停止閾値
	static constexpr float kPostBoostSlowdownRate = 0.95f; // ブースト後減速率

	// ジャンプ関連
	static constexpr float kJumpInitialVelocity = 0.6f; // ジャンプ初期速度
	static constexpr float kJumpVelocityDecay = 0.005f; // ジャンプ速度減衰
	static constexpr float kFloatJumpVelocity = 0.2f;	// 浮遊ジャンプ速度

	// ブースト関連
	static constexpr float kPreciseLockDirectionFactor = 0.7f;		  // 精密ロック方向係数
	static constexpr float kPreciseLockVerticalFactor = 0.3f;		  // 精密ロック垂直係数
	static constexpr float kPreciseLockInitialSpeed = 0.35f;		  // 精密ロック初期速度
	static constexpr float kSimpleLockDirectionFactor = 0.3f;		  // 簡易ロック方向係数
	static constexpr float kSimpleLockVerticalFactor = 0.7f;		  // 簡易ロック垂直係数
	static constexpr float kSimpleLockInitialSpeed = 0.25f;			  // 簡易ロック初期速度
	static constexpr float kQuickBoostSpeedMultiplier = 4.0f;		  // クイックブースト速度倍率
	static constexpr float kQuickBoostSpeedRetainFactor = 0.8f;		  // クイックブースト速度保持係数
	static constexpr float kStationaryBoostRecoveryMultiplier = 2.0f; // 静止時ブースト回復倍率

	static constexpr float kRecoilThreshold = 0.001f; // 反動閾値
	static constexpr float kShakeDecayRate = 0.92f;	  // 揺れ減衰率

	// 移動処理関連
	static constexpr float kMovementFrictionThreshold = 0.1f; // 移動摩擦閾値
	static constexpr float kMovementTiltFactor = -0.1f;		  // 移動傾き係数
	static constexpr float kMovementMaxTilt = 0.5f;			  // 移動最大傾き
	static constexpr float kMovementTiltLerpFactor = 0.2f;	  // 移動傾き補間係数

	// マシンガン関連
	static constexpr int kMachineGunFireInterval = 5; // マシンガン発射間隔

	// 揺れ関連
	static constexpr float kShakeBaseIntensityFactor = 0.1f; // 揺れ基本強度係数
	static constexpr float kShakeOffsetXFactor = 0.02f;		 // 揺れオフセットX係数
	static constexpr float kShakeOffsetRotFactor = 0.002f;	 // 揺れオフセット回転係数
	static constexpr int kShakeRandRangeX = 80;				 // 揺れランダム範囲X
	static constexpr int kShakeRandRangeRot = 200;			 // 揺れランダム範囲回転

	// カメラシェイク関連
	static constexpr float kShakeTime = 0.1f;	   // シェイクの時間
	static constexpr float kShakeMagnitude = 0.3f; // シェイクの大きさ

	// ドア関連
	static constexpr float kDoorOpenAngle = 3.0f;	   // ドア開放角度
	static constexpr float kDoorCloseAngle = 0.0f;	   // ドア閉鎖角度
	static constexpr float kDoorOpenCloseSpeed = 0.2f; // 補間速度
	static constexpr int kDoorStayOpenFrames = 60;	   // 開いてから閉じるまでの時間
};
