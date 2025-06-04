#pragma once
#include "Kouro.h"
#include "BaseObject.h"
#include <iostream>
#include <list>
#include <memory>
#include <random>
#include "PostEffect.h"
#include "EnemyBullet.h"

#include "BaseEnemy.h"
class GroundTypeEnemy4 :public BaseEnemy {
    // 行動状態
    enum class ActionState {
        Wander,   // 徘徊（スポーン付近で動き回る）
        Chase,    // 追跡（プレイヤーに近づく）
        Combat    // 戦闘（ストラッフィングしながら攻撃）
    };

    ///--------------------------------------------------------------
    ///						メンバ関数 
public:


    GroundTypeEnemy4() : BaseEnemy() {}

    // @brief 初期化
    void Initialize();
    // @brief 更新
    void Update();
    /**----------------------------------------------------------------------------
      * \brief  Draw 描画
      * \param  viewProjection ビュー射影行列
      * \param  directionalLight ディレクショナルライト
      * \param  pointLight ポイントライト
      * \param  spotLight スポットライト
      */
    void Draw(ViewProjection viewProjection, DirectionalLight directionalLight, PointLight pointLight, SpotLight spotLight);

    ///--------------------------------------------------------------
    ///						
private:
    /// \brief 衝突開始時の処理
    void OnCollisionEnter(BaseObject* other) override;

    /// \brief 衝突継続時の処理
    void OnCollisionStay(BaseObject* other) override;

    /// \brief 衝突終了時の処理
    void OnCollisionExit(BaseObject* other) override;

    ///--------------------------------------------------------------
    ///						 
public:

    //ジャンプ
    void HitJump();

    /**----------------------------------------------------------------------------
      * \brief  SetPosition 位置を設定
      * \param  position 位置
      */
    void SetPosition(const Vector3& position) {
        worldTransform_->transform.translate = position;
        spawnPosition_ = position; // ?X?|?[????u????
    }

    /**----------------------------------------------------------------------------
      * \brief  GetHp HPを取得
      * \return HP
      */
    void SetHp(const int hp) { hp_ = hp; }

    /**----------------------------------------------------------------------------
     * \brief  GetPosition
     * \return
     */
    const Vector3& GetPosition() const {
        return worldTransform_->transform.translate;
    }

    /**----------------------------------------------------------------------------
       * \brief  GetPosition 位置を取得
       * \return 位置
       */
    const int GetHp() const { return hp_; }

    void SetTarget(WorldTransform* target) { target_ = target; }

    //弾の取得
    std::list<std::unique_ptr<EnemyBullet>>& GetBullets() { return bullets_; }

private:
    // 行動状態別の更新メソッド
    void UpdateWanderState();
    void UpdateChaseState();
    void UpdateCombatState();

    // 状態を選択するメソッド
    void UpdateActionState();


    ///--------------------------------------------------------------
    /// メンバ変数
private:


    // 行動状態
    ActionState currentState_ = ActionState::Wander;

    // クラス内の private: の下に追加
    bool isHitReacting_ = false;
    float hitReactionTimer_ = 0.0f;
    float modelScale_ = 5.0f;

    std::unique_ptr<Audio> se1_ = nullptr;
};

