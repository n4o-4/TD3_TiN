#pragma once
#include "BaseScene.h"
#include "Kouro.h"
//========================================
// アプリケーション
#include "BaseEnemy.h"
#include "CollisionManager.h"
#include "Enemy.h"
#include "FollowCamera.h"
#include "Ground.h"
#include "GroundTypeEnemy.h"
#include "GroundTypeEnemy2.h"
#include "GroundTypeEnemy3.h"
#include "GroundTypeEnemy4.h"
#include "Hud.h"
#include "LineDrawerBase.h"
#include "LockOn.h"
#include "Player.h"
#include "SkyDome.h"
#include "SkyTypeEnemy.h"
#include "Spawn.h"
#include <random>

#include <algorithm>

class TutorialScene : public BaseScene
{
private:

	//Tutorialフェーズ
	enum class TutorialPhase
	{
		kExplain, // 説明フェーズ
		kPlay,   // プレイフェーズ
	};

	//========================================
	// ライトクラス
	std::unique_ptr<DirectionalLight> directionalLight = nullptr;
	std::unique_ptr<PointLight> pointLight = nullptr;
	std::unique_ptr<SpotLight> spotLight = nullptr;

	//========================================
	// ゲームの状態
	// ゲームクリア
	bool isGameClear_ = false;
	// ゲームオーバー
	bool isGameOver_ = false;
	// コンティニュー
	bool isContinue_ = true;

	//========================================
	// 天球
	std::unique_ptr<SkyDome> skyDome_ = nullptr;
	//========================================
	// 地面
	std::unique_ptr<Ground> ground_ = nullptr;
	//========================================
	// プレイヤー
	std::unique_ptr<Player> player_ = nullptr;
	// ロックオン
	std::unique_ptr<LockOn> lockOnSystem_ = nullptr;
	// HUD
	std::unique_ptr<Hud> hud_ = nullptr;
	// エネミー
	std::vector<std::unique_ptr<GroundTypeEnemy>> groundEnemies_;

	//========================================
	// 敵出現
	std::stringstream enemyPopCommands;
	// 敵のリスト
	std::vector<std::unique_ptr<BaseEnemy>> enemies_;
	// 待機フラグ
	bool isWaiting_ = false;
	// 待機時間
	int32_t waitTimer_ = 0;
	//========================================
	// 当たり判定マネージャ
	std::unique_ptr<CollisionManager> collisionManager_ = nullptr;
	//
	std::vector<std::unique_ptr<BaseEnemy>> spawns_;
	//=========================================
	// フェーズ
	TutorialPhase tutorialPhase_;
	bool allTrue = true;// 全てのミッションが達成されたかどうか

	// ミッションフラグ群
	bool missionFlags_[6] = { false };

	void MissionMove()
	{
		// プレイヤーが動いたらtrue
		if (float(Length(player_->GetVelocity())) > 0.0f) {
			missionFlags_[0] = true;
		}
	}
	
	void MissionCamera()
	{
		// カメラを動かしたらtrue

		Vector2 vec2 = Input::GetInstance()->GetRightStick();

		float length = std::sqrt(vec2.x * vec2.x + vec2.y * vec2.y);

		if (length > 0.0f)
		{
			missionFlags_[1] = true;
		}
	}

	void MissionJump()
	{
		if (Input::GetInstance()->Triggerkey(DIK_SPACE) ||
			Input::GetInstance()->TriggerGamePadButton(Input::GamePadButton::A))
		{
			missionFlags_[2] = true;
		}
	}

	void MissionBoost()
	{
		if (Input::GetInstance()->Triggerkey(DIK_LSHIFT) ||
			Input::GetInstance()->TriggerGamePadButton(Input::GamePadButton::X))
		{
			missionFlags_[3] = true;
		}
	}
	void MissionFireGun()
	{
		if (Input::GetInstance()->TriggerGamePadButton(Input::GamePadButton::LEFT_SHOULDER)) 
		{
			missionFlags_[4] = true;
		}
	}

	void MissionFireMissile()
	{
		if (Input::GetInstance()->TriggerGamePadButton(Input::GamePadButton::RIGHT_SHOULDER))
		{
			missionFlags_[5] = true;
		}
	}
	
	// ミッション関数
	void CheckMissions()
	{
		MissionMove();
		MissionCamera();
		MissionJump();
		MissionBoost();
		MissionFireGun();
		MissionFireMissile();
	}

	std::unique_ptr<Sprite> backGround = nullptr;

	std::unique_ptr<Sprite> explanation = nullptr; // 説明のスプライト

	std::unique_ptr<Sprite> B_select = nullptr; 

	std::unique_ptr<Sprite> checkBox_[6] = { nullptr }; // チェックボックスのスプライト配列

	std::unique_ptr<Sprite> checkMark_[6] = { nullptr }; // チェックマークのスプライト配列

public: // メンバ関数
	// 初期化
	void Initialize() override;

	// 終了
	void Finalize() override;

	// 毎フレーム更新
	void Update() override;

	void Draw();



};

