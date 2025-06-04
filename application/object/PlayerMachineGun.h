#pragma once
#include "BaseObject.h"
#include "FollowCamera.h"
#include "Kouro.h"
#include <Object3d.h>

class PlayerMachineGun : public BaseObject {
public:
	// コンストラクタ
	PlayerMachineGun(const Vector3 &position, const Vector3 &velocity);

	// 更新処理
	void Update();

	// 描画処理
	void Draw(ViewProjection viewProjection, DirectionalLight directionalLight,
			  PointLight pointLight, SpotLight spotLight);

	// アクティブ状態の取得
	bool IsActive() const {
		return isActive_;
	}

	/// 【追加】BaseObject の仮想関数を実装
	void OnCollisionEnter(BaseObject *other) override;
	void OnCollisionStay(BaseObject *other) override;
	void OnCollisionExit(BaseObject *other) override;

	// マシンガン発射処理
	static std::unique_ptr<PlayerMachineGun> Shoot(
		const Vector3 &playerPosition,
		FollowCamera *followCamera,
		Vector3 &outRecoilVelocity,
		float &outShakeIntensity);

	// 反動強度を取得
	static float GetRecoilStrength() {
		return kRecoilStrength;
	}

	// 反動減衰率を取得
	static float GetRecoilDecay() {
		return kRecoilDecay;
	}

private:
	// 移動速度
	Vector3 velocity_;

	// モデル
	std::unique_ptr<Object3d> model_;
	std::unique_ptr<WorldTransform> worldTransform_;

	// アクティブ状態
	bool isActive_ = true;

	// 弾の寿命
	int lifeTime_ = 0;

	std::unique_ptr<ParticleEmitter> particleEmitter_; // 弾道
	
	std::unique_ptr<ParticleEmitter> hitSprite_;

	std::unique_ptr<ParticleEmitter> hitSpark_;

	// マシンガン関連の定数
	static constexpr int kCooldownTime = 5;
	static constexpr float kSpread = 0.0f;
	static constexpr float kBulletSpeed = 64.0f;
	static constexpr float kInitialShakeIntensity = 2.0f;
	static constexpr float kRecoilStrength = 0.05f;
	static constexpr float kRecoilDecay = 0.9f;
};