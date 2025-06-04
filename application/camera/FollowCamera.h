#pragma once
#include "BaseCamera.h"
#include "Input.h"
#include "WorldTransform.h"

class BaseEnemy;
class FollowCamera : public BaseCamera {
public:
	// 初期化
	void Initialize();
	// 更新
	void Update();

	void SetTarget(WorldTransform *target) {
		target_ = target;
	}

	// 前方向ベクトルを取得
	Vector3 GetForwardDirection() const;

	// 右方向ベクトルを取得
	Vector3 GetRightDirection() const;

	// 上方向ベクトルを取得
	Vector3 GetUpDirection() const;

	// カメラの位置を取得
	Vector3 GetPosition() const {
		return currentPosition_;
	}

	void SetPosition(const Vector3 &position) {
		currentPosition_ = position;
	}

	// カメラのビュープロジェクションを取得
	ViewProjection &GetViewProjection() {
		return *viewProjection_;
	}

	// カメラの回転を取得
	Vector3 GetRotation() const {
		return destinationRotate;
	}

	// カメラのオフセットを取得
	Vector3 GetOffset() const {
		return offset_;
	}

	// カメラのデフォルトオフセットを取得
	Vector3 GetDefaultOffset() const {
		return defaultOffset_;
	}

	// カメラのデフォルトオフセットを設定
	void SetDefaultOffset(const Vector3 &offset) {
		defaultOffset_ = offset;
	}

	// エネミーのリストを取得
	void SetEnemiesAndSpawns(const std::vector<std::unique_ptr<BaseEnemy>> *enemies,
							 const std::vector<std::unique_ptr<BaseEnemy>> *spawns) {
		enemies_ = enemies;
		spawns_ = spawns;
	}

private:
	Vector3 CalculationOffset();

	void CalculationRotate();

	void CalculationTranslate();

	// 高さによるFOV演出
	void UpdateHeightFOVEffect();

	// マシンガン発射時のカメラ演出を更新
	void UpdateFiringCameraEffect();

	// ミサイル発射時のカメラ演出を更新
	void UpdateMissileFiringCameraEffect();

	// エイムアシスト処理
	void UpdateAimAssist();

	// 最も近い敵を見つける
	BaseEnemy *FindNearestEnemyInSight();

	// 敵がエイムアシスト対象となる角度内かチェック
	bool IsEnemyInAssistRange(const Vector3 &enemyPos);

	// エイムアシストの減衰（プレイヤー入力時）
	void DecayAimAssist();

private:
	//========================================
	// ターゲット
	WorldTransform *target_ = nullptr;
	//========================================

	// 追従対象の残像座標
	Vector3 interTarget_ = {};
	//========================================

	// オフセット
	Vector3 offset_ = {};
	// デフォルトのオフセット（元の値を保持）
	Vector3 defaultOffset_ = {0.0f, 3.0f, -15.0f};

	//========================================
	// 現在のカメラ位置
	Vector3 currentPosition_ = {};
	// 目標のカメラ位置
	Vector3 destinationRotate = {0.0f, 0.0f, 0.0f};
	// イージング係数
	float easingFactor_ = 0.85f;
	// 回転速度
	float rotateSpeed_ = 0.04f;

	//========================================
	// FOV関連パラメータ
	float baseFOV_ = 40.0f;			  // 基本FOV値（地上での視野角）
	float currentFOV_ = 40.0f;		  // 現在のFOV
	float targetFOV_ = 40.0f;		  // 目標FOV
	float fovLerpSpeed_ = 0.4f;		  // FOV補間速度
	float heightFOVIncrease_ = 15.0f; // 最大高度での追加FOV量
	float maxHeightForFOV_ = 30.0f;	  // 最大効果を得るための高度
	// 高度追跡用
	float prevPlayerHeight_ = 0.0f; // 前フレームの高さ（必要に応じて）

	//========================================
	// マシンガン発射演出関連
	bool isFiring_ = false;			  // マシンガン発射中フラグ
	float firingOffsetFactor_ = 0.4f; // 発射中のカメラ距離係数（1.0より小さい値）
	float firingLerpSpeed_ = 0.9f;	  // カメラ距離変更の補間速度

	//========================================
	// ミサイル発射演出
	bool isMissileFiring_ = false;			 // ミサイル発射中フラグ
	float missileFiringOffsetFactor_ = 2.0f; // ミサイル発射中のカメラ距離係数（1.0より大きい値で遠ざける）
	float missileFiringLerpSpeed_ = 0.9f;	 // ミサイル発射中のカメラ距離変更の補間速度
	float missileFireTimer_ = 0.0f;			 // ミサイル発射タイマー
	float missileFireDuration_ = 2.0f;		 // ミサイル発射演出の持続時間（秒）

	//========================================
	// エイムアシスト
	// 目標の位置を取得
	// 敵のリスト
	const std::vector<std::unique_ptr<BaseEnemy>> *enemies_ = nullptr;
	// スポーンのリスト
	const std::vector<std::unique_ptr<BaseEnemy>> *spawns_ = nullptr;

	// エイムアシスト関連パラメータ
	bool enableAimAssistOnlyWhileFiring_ = true; // マシンガン射撃時のみエイムアシストを有効にするフラグ
	float aimAssistFactor_ = 1.2f;				 // エイムアシストの強さ（0.0fから1.0f）- 強化
	float aimAssistRange_ = 1024.0f;			 // エイムアシスト検知の距離範囲（メートル）
	float aimAssistAngle_ = 45.0f;				 // エイムアシスト検知の角度範囲（度）- 拡大
	float aimAssistSpeed_ = 0.35f;				 // エイムアシスト回転速度 - 高速化
	float aimAssistDecayRate_ = 0.92f;			 // プレイヤー入力時の減衰率 - 緩やかに
	float currentAimAssistStrength_ = 0.0f;		 // 現在のアシスト強度（0.0f〜1.0f）
	BaseEnemy *currentAimTarget_ = nullptr;		 // 現在のアシスト対象
	float assistInputTriggerThreshold_ = 0.08f;	 // プレイヤー入力検出閾値 - 感度を下げる
	float assistInnerAngle_ = 25.0f;			 // 強いアシストを適用する内側の角度範囲 - 拡大
	float timeSinceLastInput_ = 0.0f;			 // 最後の入力からの経過時間
	float assistRampUpTime_ = 0.15f;			 // アシスト強度が上がるまでの時間 - 高速化
	float targetSwitchCooldown_ = 0.0f;			 // ターゲット切り替えのクールダウン
	float targetSwitchCooldownTime_ = 0.3f;		 // ターゲット切り替えクールダウン時間

public:

	bool enableUpdate_ = true; 
};
