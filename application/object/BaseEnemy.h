#pragma once
#include "BaseObject.h"
#include "Kouro.h"
#include <iostream>
#include <list>
#include <memory>
#include <random>
#include <vector> // 追加

#include "EnemyBullet.h"

class BaseEnemy : public BaseObject {
	///--------------------------------------------------------------
	///						 メンバ関数
public:
	virtual ~BaseEnemy() = default;

	// @brief 初期化
	virtual void Initialize(Model *model);
	// @brief 更新
	virtual void Update();
	/**----------------------------------------------------------------------------
	 * \brief  Draw 描画
	 * \param  viewProjection ビュー射影行列
	 * \param  directionalLight ディレクショナルライト
	 * \param  pointLight ポイントライト
	 * \param  spotLight スポットライト
	 */
	void Draw(ViewProjection viewProjection, DirectionalLight directionalLight, PointLight pointLight, SpotLight spotLight);

	const Vector3 &GetPosition() const {
		return worldTransform_->transform.translate;
	}

	std::list<std::unique_ptr<EnemyBullet>> &GetBullets() {
		return bullets_;
	}

	virtual WorldTransform *GetWorldTransform() {
		return worldTransform_.get();
	}

	virtual bool IsAlive() const {
		return hp_ > 0;
	}

	void AddAvoidance(const Vector3 &offset) {
		avoidanceVelocity_ = offset;
	}
	void SetVelocityY(float y) {
		velocity_.y = y;
	}

	void SetVelocity(const Vector3& velocity) {
		velocity_ = velocity;
	}
	/**----------------------------------------------------------------------------
	 * \brief  GetHp HPを取得
	 * \return HP
	 */
	const int GetHp() const {
		return hp_;
	}
	const int GetSpawnHp() const {
		return spawnHp_;
	}

	void SetOtherEnemies(std::vector<BaseEnemy *> *enemies) {
		otherEnemies_ = enemies;
	}
	///--------------------------------------------------------------
	///						 当たり判定
private:
	/// \brief 衝突開始時の処理
	virtual void OnCollisionEnter(BaseObject *other) override;

	/// \brief 衝突継続時の処理
	virtual void OnCollisionStay(BaseObject *other) override;

	/// \brief 衝突終了時の処理
	virtual void OnCollisionExit(BaseObject *other) override;

	///--------------------------------------------------------------
	///						 入出力関数
protected:
	// 方向を選択するメソッド
	Vector3 SelectDirection();

	void CalculateAvoidance(); // 他の敵との衝突回避計算

	void BulletUpdate();
	void BulletDraw(ViewProjection viewProjection, DirectionalLight directionalLight, PointLight pointLight, SpotLight spotLight);
	void Fire();

	void MoveToTarget();

	void RandomMove(float scale);

	Vector3 Lerp(const Vector3 &a, const Vector3 &b, float t) {
		return a * (1.0f - t) + b * t;
	};

	// ジャンプ
	void HitJump();

	////
	void SetModelColor(const Vector4 &color) {
		model_->SetColor(color);
	}

	void SetModelScale(const Vector3 &scale) {
		model_->SetScale(scale);
	}

	///--------------------------------------------------------------
	/// メンバ変数
protected:
	//========================================
	// モデル
	std::unique_ptr<Object3d> model_ = nullptr;

	// ワールド変換
	std::unique_ptr<WorldTransform> worldTransform_ = nullptr;

	// HP
	int hp_ = 3;
	int spawnHp_ = 1;

	// 移動関連
	float speed_ = 1.9f;							 // 移動速度
	float minX_ = -100.0f;							 // 左の限界
	float maxX_ = 100.0f;							 // 右の限界
	float minY_ = 0.0f;								 // 下の限界
	float maxY_ = 100.0f;							 // 上の限界
	int direction_ = 1;								 // 移動方向 (1:右, -1:左)
	float rotationVelocityY_ = 0.0f;				 // Y軸回転速度
	float targetRotationY_ = 0.0f;					 // ターゲットのY軸回転
	Vector3 velocity_ = {0.0f, 0.0f, 0.0f};			 // 現在の速度ベクトル
	Vector3 spawnPosition_ = {0.0f, 0.0f, 0.0f};	 // スポーン位置
	Vector3 avoidanceVelocity_ = {0.0f, 0.0f, 0.0f}; // 回避ベクトル
	float avoidanceRadius_ = 15.0f;					 // 敵同士の回避半径

	// 行動状態
	float stateTimer_ = 0.0f;
	float directionChangeTimer_ = 0.0f;

	// 行動パラメータ
	float chaseDistance_ = 64.0f;		   // この距離内ならプレイヤーを追いかける
	float combatDistance_ = 32.0f;		   // 戦闘を行う距離
	float safeDistance_ = 20.0f;		   // 安全距離（この距離を保つ）
	float wanderRadius_ = 32.0f;		   // 徘徊半径
	float directionChangeInterval_ = 3.0f; // 方向変更の間隔

	// 乱数生成器
	std::mt19937 rng_;
	std::uniform_real_distribution<float> angleDist_{0.0f, 2.0f * 3.14159f};
	std::uniform_real_distribution<float> strengthDist_{0.5f, 1.0f};

	WorldTransform *target_;
	std::list<std::unique_ptr<EnemyBullet>> bullets_;
	const float kIntervalTiem = 5.0f;
	float intervalCounter_ = 0.0f;

	std::unique_ptr<ParticleEmitter> particleEmitter_ = nullptr;

	// Vector3 avoidanceVelocity_ = {0.0f, 0.0f, 0.0f}; // 上に移動しました
	std::vector<BaseEnemy *> *otherEnemies_; // 他の敵オブジェクトのリストへのポインタ
};