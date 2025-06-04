#pragma once
#include "Vectors.h"
#include "Object3d.h"
#include "WorldTransform.h"
#include "BaseObject.h"

///=============================================================================
///						敵の弾クラス
class EnemyBullet : public BaseObject
{
	///--------------------------------------------------------------
	///							メンバ関数
public:

	// 初期化
	void Initialize(const WorldTransform worldTransform,Vector3 target);

	// 更新処理
	void Update();

	// 描画
	void Draw(ViewProjection viewProjection, DirectionalLight directionalLight, PointLight pointLight, SpotLight spotLight);

	bool GetIsActive() const { return isActive_; }

private:
	///--------------------------------------------------------------
	///						 当たり判定
	/// \brief 衝突開始時の処理
	void OnCollisionEnter(BaseObject* other) override;

	/// \brief 衝突継続時の処理
	void OnCollisionStay(BaseObject* other) override;

	/// \brief 衝突終了時の処理
	void OnCollisionExit(BaseObject* other) override;

	///--------------------------------------------------------------
	///						 メンバ変数
private:
	//========================================
	// 3Dオブジェクト
	std::unique_ptr<Object3d> object3d_;
	std::unique_ptr<WorldTransform> objectTransform_; // `objectTransform_` に変更

	Vector3 targetPosition_ = {};

	Vector3 velocity_ = {};

	bool isActive_ = true;

	const float kActiveTime_ = 30.0f;

	float activeTime_ = 0.0f;
};

