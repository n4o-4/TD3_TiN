#pragma once
#include "Kouro.h"
#include "BaseObject.h"
#include "BaseEnemy.h"
class Spawn : public BaseEnemy {
public:

	Spawn() : BaseEnemy() {}

	void Initialize();
	void Update();
	void Draw(ViewProjection viewProjection, DirectionalLight directionalLight, PointLight pointLight, SpotLight spotLight);

	void SetPosition(const Vector3& position) {
        worldTransform_->transform.translate = position;
        spawnPosition_ = position; // スポーン位置も保存
	}

	bool IsAlive() const override { return spawnHp_ > 0; }

	const int GetHp() const { return spawnHp_; }

	void SetTarget(WorldTransform* target) { target_ = target; }

    /**----------------------------------------------------------------------------
     * \brief  SetHp HPを設定
     * \param  hp HP
     */
    void SetHp(const int hp) { spawnHp_ = hp; }

    /**----------------------------------------------------------------------------
     * \brief  GetPosition 位置を取得
     * \return 位置
     */
    const Vector3& GetPosition() const {
        return worldTransform_->transform.translate;
    }

private:
	/// \brief 衝突開始時の処理
	void OnCollisionEnter(BaseObject* other) override;

	/// \brief 衝突継続時の処理
	void OnCollisionStay(BaseObject* other) override;

	/// \brief 衝突終了時の処理
	void OnCollisionExit(BaseObject* other) override;

private:
	//Spawn model
	//std::unique_ptr<Object3d> spawnModel_ = nullptr;
	//Spawn 
	//std::unique_ptr<WorldTransform> spawnWorldTransform_ = nullptr;


};

