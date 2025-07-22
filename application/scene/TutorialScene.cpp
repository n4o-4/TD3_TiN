#include "TutorialScene.h"
#include "TutorialEnemy.h"
#include <cstdlib>

void TutorialScene::Initialize() {
	// 共通初期化
	BaseScene::Initialize();

	// テクスチャ読み込み（チュートリアル用のみに限定）
	TextureManager::GetInstance()->LoadTexture("Resources/white.png");
	TextureManager::GetInstance()->LoadTexture("Resources/operation2.png");
	TextureManager::GetInstance()->LoadTexture("Resources/CheckBox.png");
	TextureManager::GetInstance()->LoadTexture("Resources/checkMark.png");
	TextureManager::GetInstance()->LoadTexture("Resources/B_select.png");

	// 背景スプライト
	backGround = std::make_unique<Sprite>();
	backGround->Initialize(SpriteCommon::GetInstance(), "Resources/white.png");
	backGround->SetPosition({ 840.0f, 0.0f });
	backGround->SetTexSize({ 1.0f, 1.0f });
	backGround->SetSize({ 1280.0f, 720.0f });
	backGround->SetColor({ 1.0f, 1.0f, 1.0f, 0.5f });
	backGround->Update();

	// 説明スプライト
	explanation = std::make_unique<Sprite>();
	explanation->Initialize(SpriteCommon::GetInstance(), "Resources/operation2.png");
	explanation->SetPosition({ 700.0f, 0.0f });
	explanation->SetTexSize({ 1280.0f, 720.0f });
	explanation->SetSize({ 1280.0f, 720.0f });
	explanation->Update();

	// Bボタン選択
	B_select = std::make_unique<Sprite>();
	B_select->Initialize(SpriteCommon::GetInstance(), "Resources/B_select.png");
	B_select->SetTexSize({ 1024.0f, 315.0f });
	B_select->SetSize({ 300.0f, 92.0f });
	B_select->SetPosition({ 920.0f, 600.0f });
	B_select->Update();

	// ミッションチェックUI
	for (int i = 0; i < 6; ++i) {
		checkBox_[i] = std::make_unique<Sprite>();
		checkBox_[i]->Initialize(SpriteCommon::GetInstance(), "Resources/CheckBox.png");
		checkBox_[i]->SetPosition({ 880.0f, 160.0f + (i * 76.0f) });
		checkBox_[i]->SetTexSize({ 128.0f, 128.0f });
		checkBox_[i]->SetSize({ 48.0f, 48.0f });
		checkBox_[i]->Update();

		checkMark_[i] = std::make_unique<Sprite>();
		checkMark_[i]->Initialize(SpriteCommon::GetInstance(), "Resources/checkMark.png");
		checkMark_[i]->SetPosition({ 880.0f, 160.0f + (i * 76.0f) });
		checkMark_[i]->SetTexSize({ 128.0f, 128.0f });
		checkMark_[i]->SetSize({ 48.0f, 48.0f });
		checkMark_[i]->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
		checkMark_[i]->Update();
	}

	// ライト設定（共通）
	directionalLight = std::make_unique<DirectionalLight>();
	directionalLight->Initilaize();
	directionalLight->intensity_ = 0.0f;

	pointLight = std::make_unique<PointLight>();
	pointLight->Initilize();
	pointLight->intensity_ = 0.0f;

	spotLight = std::make_unique<SpotLight>();
	spotLight->Initialize();
	spotLight->direction_ = { 0.0f, -1.0f, 0.0f };
	spotLight->position_ = { 0.0f, 2000.0f, 0.0f };
	spotLight->intensity_ = 3.0f;
	spotLight->decay_ = 1.6f;
	spotLight->distance_ = 3200.0f;
	spotLight->cosAngle_ = 0.2f;

	// 地形
	skyDome_ = std::make_unique<SkyDome>();
	skyDome_->Initialize();

	ground_ = std::make_unique<Ground>();
	ground_->Initialize();

	// プレイヤー
	player_ = std::make_unique<Player>();
	player_->Initialize();

	// ロックオン
	lockOnSystem_ = std::make_unique<LockOn>();
	lockOnSystem_->Initialize();
	player_->SetLockOnSystem(lockOnSystem_.get());

	// 当たり判定
	collisionManager_ = std::make_unique<CollisionManager>();
	collisionManager_->Initialize(32.0f);

	// カメラ設定
	cameraManager_->SetFollowCameraTarget(player_->GetWorldTransform());
	cameraManager_->GetFollowCamera()->SetEnemiesAndSpawns(&enemies_, &spawns_);
	cameraManager_->useFollowCamera_ = true;
	player_->SetFollowCamera(cameraManager_->GetFollowCamera());

	// HUD
	hud_ = std::make_unique<Hud>();
	hud_->SetEnemiesAndSpawns(&enemies_, &spawns_);
	hud_->Initialize(cameraManager_->GetFollowCamera(), player_.get(), lockOnSystem_.get());

	// フェーズ初期化
	tutorialPhase_ = TutorialPhase::kExplain;

	srand(static_cast<unsigned int>(time(nullptr)));
}


void TutorialScene::Finalize()
{
	skyDome_.reset();
	ground_.reset();

	player_->Finalize();
}

void TutorialScene::Update() {
	BaseScene::Update();

	// ライト更新
	directionalLight->Update();
	pointLight->Update();
	spotLight->Update();

	switch (phase_) {
	case Phase::kFadeIn:
		if (fade_->IsFinished()) {
			phase_ = Phase::kPlay;
			Input::GetInstance()->SetIsReception(true);
		}
		player_->Update();
		skyDome_->Update();
		ground_->Update();
		break;

	case Phase::kPlay:

		// Bボタンでタイトルへ戻る
		if (Input::GetInstance()->TriggerGamePadButton(Input::GamePadButton::B)) {
			isGameOver_ = false; // 明示的にfalseをセット（戻るだけなので）
			phase_ = Phase::kFadeOut;
			fade_->Start(Fade::Status::FadeOut, fadeTime_);
		}

		if (player_->GetHp() <= 0) {
			isGameOver_ = true;
			phase_ = Phase::kFadeOut;
			fade_->Start(Fade::Status::FadeOut, fadeTime_);
		}

		player_->Update();
		skyDome_->Update();
		ground_->Update();

		if (tutorialPhase_ == TutorialPhase::kExplain) {
			CheckMissions();

			bool allTrue = true;
			for (bool flag : missionFlags_) {
				if (!flag) {
					allTrue = false;
					break;
				}
			}

			if (allTrue) {
				tutorialPhase_ = TutorialPhase::kPlay;

				for (int i = 0; i < 10; ++i) {
					auto enemy = std::make_unique<TutorialEnemy>();
					enemy->Initialize(ModelManager::GetInstance()->FindModel("enemy/kumo/kumo.obj"));
					enemy->SetPosition({ float(rand() % 101 - 50), 0.0f, float(rand() % 101 - 50) });
					enemies_.push_back(std::move(enemy));
				}
			}
		}

		// 敵更新と削除
		for (auto& enemy : enemies_) enemy->Update();
		enemies_.erase(std::remove_if(enemies_.begin(), enemies_.end(),
			[this](const std::unique_ptr<BaseEnemy>& enemy) {
				if (enemy->GetHp() <= 0) {
					if (lockOnSystem_) lockOnSystem_->RemoveLockedEnemy(enemy.get());
					for (auto& b : player_->GetBullets()) {
						if (b->GetTarget() == enemy.get()) b->SetTarget(nullptr);
					}
					return true;
				}
				return false;
			}), enemies_.end());

		// LockOn更新
		if (lockOnSystem_) {
			lockOnSystem_->SetPosition(player_->GetPosition());

			if (auto follow = dynamic_cast<FollowCamera*>(cameraManager_->GetActiveCamera())) {
				lockOnSystem_->SetViewDirection(follow->GetForwardDirection());
			}

			std::vector<BaseEnemy*> allTargets;
			for (auto& e : enemies_) allTargets.push_back(e.get());
			for (auto& s : spawns_) allTargets.push_back(s.get());

			lockOnSystem_->DetectEnemiesRaw(allTargets);
			lockOnSystem_->UpdateRaw(allTargets);
		}

		// 当たり判定
		collisionManager_->Reset();
		collisionManager_->AddCollider(player_.get());
		for (auto& b : player_->GetBullets()) collisionManager_->AddCollider(b.get());
		for (auto& b : player_->GetMachineGunBullets()) collisionManager_->AddCollider(b.get());
		for (auto& e : enemies_) {
			collisionManager_->AddCollider(e.get());
			for (auto& b : e->GetBullets()) collisionManager_->AddCollider(b.get());
		}
		collisionManager_->Update();

		ParticleManager::GetInstance()->Update();
		break;

	case Phase::kFadeOut:
		if (fade_->IsFinished()) {
			SceneManager::GetInstance()->ChangeScene(isGameOver_ ? "OVER" : "TITLE");
		}
		player_->Update();
		skyDome_->Update();
		ground_->Update();
		ParticleManager::GetInstance()->Update();
		break;

	default:
		break;
	}
}


void TutorialScene::Draw() {
	DrawBackgroundSprite();
	DrawObject();

	skyDome_->Draw(cameraManager_->GetActiveCamera()->GetViewProjection(),
		*directionalLight, *pointLight, *spotLight);

	ground_->Draw(cameraManager_->GetActiveCamera()->GetViewProjection(),
		*directionalLight, *pointLight, *spotLight);

	for (const auto& enemy : enemies_) {
		enemy->Draw(cameraManager_->GetActiveCamera()->GetViewProjection(),
			*directionalLight, *pointLight, *spotLight);
	}

	player_->Draw(cameraManager_->GetActiveCamera()->GetViewProjection(),
		*directionalLight, *pointLight, *spotLight);

	if (tutorialPhase_ == TutorialPhase::kPlay && lockOnSystem_) {
		lockOnSystem_->Draw(cameraManager_->GetActiveCamera()->GetViewProjection(),
			*directionalLight, *pointLight, *spotLight);
	}

	collisionManager_->Draw();

	if (tutorialPhase_ == TutorialPhase::kPlay) {
		hud_->Draw(cameraManager_->GetActiveCamera()->GetViewProjection());
	}

	DrawForegroundSprite();

	if (tutorialPhase_ == TutorialPhase::kExplain) {
		backGround->Draw();
		explanation->Draw();
		for (int i = 0; i < 6; ++i) checkBox_[i]->Draw();
		for (int i = 0; i < 6; ++i) if (missionFlags_[i]) checkMark_[i]->Draw();
	}

	B_select->Draw();

	if (phase_ == Phase::kFadeIn || phase_ == Phase::kFadeOut) {
		DrawFade();
	}

	ParticleManager::GetInstance()->Draw("Resources/circle.png");
}

