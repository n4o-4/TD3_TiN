#include "GameScene.h"
#include "LineManager.h"
#include "imgui.h"
///=============================================================================
///						マトリックス表示
void ShowMatrix4x4(const Matrix4x4 &matrix, const char *label) {
	ImGui::Text("%s", label);
	if (ImGui::BeginTable(label, 4, ImGuiTableFlags_Borders)) {
		//
		for (int i = 0; i < 4; ++i) {
			ImGui::TableNextRow();
			for (int j = 0; j < 4; ++j) {
				ImGui::TableSetColumnIndex(j);
				ImGui::Text("%.3f", matrix.m[i][j]);
			}
		}
		ImGui::EndTable();
	}
}

struct GradientPoint {
	float position; // 0.0～1.0の範囲
	ImVec4 color;	// 色
};

std::vector<GradientPoint> gradientPoints = {
	{0.0f, ImVec4(1.0f, 0.0f, 0.0f, 1.0f)}, // 赤
	{1.0f, ImVec4(0.0f, 0.0f, 1.0f, 1.0f)}	// 青
};

void ShowGradientEditor() {
	ImGui::Text("Gradient Editor");

	// 制御点の編集
	for (size_t i = 0; i < gradientPoints.size(); ++i) {
		ImGui::PushID(static_cast<int>(i));
		ImGui::DragFloat("Position", &gradientPoints[i].position, 0.01f, 0.0f, 1.0f);
		ImGui::ColorEdit4("Color", (float *)&gradientPoints[i].color);
		ImGui::PopID();
	}

	// グラデーションのプレビュー
	ImDrawList *drawList = ImGui::GetWindowDrawList();
	ImVec2 canvasPos = ImGui::GetCursorScreenPos();
	ImVec2 canvasSize = ImVec2(300, 50); // プレビューのサイズ
	drawList->AddRectFilled(canvasPos, ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y), IM_COL32(50, 50, 50, 255));

	for (size_t i = 0; i + 1 < gradientPoints.size(); ++i) {
		ImVec2 start = ImVec2(canvasPos.x + gradientPoints[i].position * canvasSize.x, canvasPos.y);
		ImVec2 end = ImVec2(canvasPos.x + gradientPoints[i + 1].position * canvasSize.x, canvasPos.y + canvasSize.y);
		ImU32 colStart = ImGui::ColorConvertFloat4ToU32(gradientPoints[i].color);
		ImU32 colEnd = ImGui::ColorConvertFloat4ToU32(gradientPoints[i + 1].color);
		drawList->AddRectFilledMultiColor(start, end, colStart, colEnd, colEnd, colStart);
	}

	ImGui::Dummy(canvasSize); // スペースを確保
}

///=============================================================================
///						初期化
void GameScene::Initialize() {
	//========================================
	// 基底シーン
	BaseScene::Initialize();

	//========================================
	// テクスチャの読み込み

	//========================================
	// ライト
	// 指向性
	directionalLight = std::make_unique<DirectionalLight>();
	directionalLight->Initilaize();
	directionalLight->intensity_ = 0.0f;
	// 点光源
	pointLight = std::make_unique<PointLight>();
	pointLight->Initilize();
	pointLight->intensity_ = 0.0f;
	// スポットライト
	spotLight = std::make_unique<SpotLight>();
	spotLight->Initialize();
	spotLight->direction_ = {0.0f, -1.0f, 0.0f};
	spotLight->position_ = {0.0f, 2000.0f, 0.0f};
	spotLight->intensity_ = 3.0f;
	spotLight->decay_ = 1.6f;
	spotLight->distance_ = 3200.0f;
	spotLight->cosAngle_ = 0.2f;
	spotLight->cosFalloffStart_;
	//========================================
	// 天球
	skyDome_ = std::make_unique<SkyDome>();
	skyDome_->Initialize();
	//========================================
	// 地面
	ground_ = std::make_unique<Ground>();
	ground_->Initialize();
	//========================================
	// プレイヤーを生成
	player_ = std::make_unique<Player>();
	player_->Initialize();
	//========================================
	// 🔽 LockOnを初期化
	lockOnSystem_ = std::make_unique<LockOn>();
	lockOnSystem_->Initialize();
	// 🔹 SetLockOnSystem() に std::move を使用
	player_->SetLockOnSystem(lockOnSystem_.get()); // 🔹 `std::move()` を使わず `get()` でポインタを渡す
	//========================================

	EwaveCsvPaths_ = {
		"./Resources/enemySpawn1.csv",
		"./Resources/enemySpawn2.csv",
		"./Resources/enemySpawn3.csv"};
	NwaveCsvPaths_ = {
		"./Resources/enemySpawn4.csv",
		"./Resources/enemySpawn5.csv",
		"./Resources/enemySpawn6.csv"};
	HwaveCsvPaths_ = {
		"./Resources/enemySpawn7.csv",
		"./Resources/enemySpawn8.csv",
		"./Resources/enemySpawn9.csv"};

	SceneTransitionData &transitionData = SceneManager::GetInstance()->GetTransitionData();
	easy_ = transitionData.easy;
	nomal_ = transitionData.nomal;
	hard_ = transitionData.hard;

	// 敵出現
	//========================================
	LoadEnemyPopData(waveIndex_);

	//========================================
	// 当たり判定マネージャ
	collisionManager_ = std::make_unique<CollisionManager>();
	collisionManager_->Initialize(32.0f);

	//========================================
	// フォローカメラのターゲットを設定
	cameraManager_->SetFollowCameraTarget(player_->GetWorldTransform());
	// フォローカメラへ敵のリストの受け渡し
	cameraManager_->GetFollowCamera()->SetEnemiesAndSpawns(&enemies_, &spawns_);

	// アクティブカメラをフォローカメラに設定
	cameraManager_->useFollowCamera_ = true;

	player_->SetFollowCamera(cameraManager_->GetFollowCamera());

	// sceneManager_->GetPostEffect()->ApplyEffect(PostEffect::EffectType::Grayscale); //完
	// sceneManager_->GetPostEffect()->ApplyEffect(PostEffect::EffectType::Vignette); //完
	// sceneManager_->GetPostEffect()->ApplyEffect(PostEffect::EffectType::BoxFilter); //完
	// sceneManager_->GetPostEffect()->ApplyEffect(PostEffect::EffectType::GaussianFilter); //完
	// sceneManager_->GetPostEffect()->ApplyEffect(PostEffect::EffectType::LuminanceBasedOutline); //完
	//sceneManager_->GetPostEffect()->ApplyEffect("Outline", PostEffect::EffectType::DepthBasedOutline); // 完
	// sceneManager_->GetPostEffect()->ApplyEffect(PostEffect::EffectType::RadialBlur); //完
	// sceneManager_->GetPostEffect()->ApplyEffect("dissolve",PostEffect::EffectType::Dissolve); //完
	// sceneManager_->GetPostEffect()->ApplyEffect(PostEffect::EffectType::Random); //完
	//sceneManager_->GetPostEffect()->ApplyEffect("fog",PostEffect::EffectType::LinearFog); //完
	sceneManager_->GetPostEffect()->ApplyEffect("Blur", PostEffect::EffectType::MotionBlur);
	// sceneManager_->GetPostEffect()->ApplyEffect("Grtich",PostEffect::EffectType::Gritch); //完

	dissolve_ = dynamic_cast<Dissolve *>(sceneManager_->GetPostEffect()->GetEffectData("dissolve"));
	blur_ = dynamic_cast<MotionBlur *>(sceneManager_->GetPostEffect()->GetEffectData("Blur"));

	///========================================
	///		ライン描画
	lineDrawer_ = std::make_unique<LineDrawerBase>();
	lineDrawer_->Initialize(sceneManager_->GetDxCommon(), sceneManager_->GetSrvManager());

	lineDrawer_->CreateLineObject(LineDrawerBase::Type::Grid, nullptr);

	/*///========================================
	/// アニメーションマネージャ
	animationManager = std::make_unique<AnimationManager>();
	animationManager->LoadAnimationFile("./Resources/human", "walk.gltf");
	animationManager->StartAnimation("walk.gltf", 0);
	animationManager->Update();
	lineDrawer_->CreateSkeletonObject(animationManager->GetActiveAnimation("walk.gltf").skeleton,nullptr);*/

	/// LoadModelでエラー発生中
	/*ModelManager::GetInstance()->LoadModel("human/wlak.gltf");	///

	human_ = std::make_unique<Object3d>();
	human_->Initialize(Object3dCommon::GetInstance());
	human_->SetModel(ModelManager::GetInstance()->FindModel("human/wlak.gltf"));*/

	//BGM
	AudioManager::GetInstance()->Initialize();
	AudioManager::GetInstance()->SoundLoadFile("Resources/bgm/easy.mp3");
	AudioManager::GetInstance()->SoundLoadFile("Resources/bgm/nomal.mp3");
	AudioManager::GetInstance()->SoundLoadFile("Resources/bgm/hard.mp3");

	easyBGM_ = std::make_unique<Audio>();
	nomalBGM_ = std::make_unique<Audio>();
	hardBGM_ = std::make_unique<Audio>();

	easyBGM_->Initialize();
	nomalBGM_->Initialize();
	hardBGM_->Initialize();

	if (easy_) {
		easyBGM_->SoundPlay("Resources/bgm/easy.mp3", 9999);
		easyBGM_->SetVolume(0.6f); 
	} else if (nomal_) {
		nomalBGM_->SoundPlay("Resources/bgm/nomal.mp3", 9999);
		nomalBGM_->SetVolume(0.6f);
	} else if (hard_) {
		hardBGM_->SoundPlay("Resources/bgm/hard.mp3", 9999);
		hardBGM_->SetVolume(0.6f);
	}

	//wave sprite
	TextureManager::GetInstance()->LoadTexture("Resources/text/wave1.png");
	wave1_ = std::make_unique<Sprite>();
	wave1_->Initialize(SpriteCommon::GetInstance(), "Resources/text/wave1.png");
	wave1_->SetTexSize({1280.0f, 720.0f});
	wave1_->SetSize({1280.0f, 720.0f});
	wave1_->SetPosition({0.0f, 0.0f});

	TextureManager::GetInstance()->LoadTexture("Resources/text/wave2.png");
	wave2_ = std::make_unique<Sprite>();
	wave2_->Initialize(SpriteCommon::GetInstance(), "Resources/text/wave2.png");
	wave2_->SetTexSize({1280.0f, 720.0f});
	wave2_->SetSize({1280.0f, 720.0f});
	wave2_->SetPosition({0.0f, 0.0f});

	TextureManager::GetInstance()->LoadTexture("Resources/text/wave3.png");
	wave3_ = std::make_unique<Sprite>();
	wave3_->Initialize(SpriteCommon::GetInstance(), "Resources/text/wave3.png");
	wave3_->SetTexSize({1280.0f, 720.0f});
	wave3_->SetSize({1280.0f, 720.0f});
	wave3_->SetPosition({0.0f, 0.0f});
	//========================================
	// HUD
	hud_ = std::make_unique<Hud>();
	// 敵とスポーンの情報を最新に保つ
	hud_->SetEnemiesAndSpawns(&enemies_, &spawns_);
	hud_->Initialize(cameraManager_->GetFollowCamera(), player_.get(), lockOnSystem_.get());

	TextureManager::GetInstance()->LoadTexture("Resources/white.png");
	TextureManager::GetInstance()->LoadTexture("Resources/operation2.png");
	TextureManager::GetInstance()->LoadTexture("Resources/contGame.png");
	TextureManager::GetInstance()->LoadTexture("Resources/returnTitle.png");
	TextureManager::GetInstance()->LoadTexture("Resources/xbox_dpad_round_vertical.png");
	TextureManager::GetInstance()->LoadTexture("Resources/xbox_button_menu_outline.png");
	TextureManager::GetInstance()->LoadTexture("Resources/pose.png");
	TextureManager::GetInstance()->LoadTexture("Resources/pointer.png");
	//========================================
	// 背景

	backGround = std::make_unique<Sprite>();
	backGround->Initialize(SpriteCommon::GetInstance(), "Resources/white.png");
	backGround->SetPosition({ 0.0f, 0.0f });
	backGround->SetTexSize({ 1.0f, 1.0f });   // テクスチャの描画範囲
	backGround->SetSize({ 1280.0f, 720.0f });        // 描画サイズ

	backGround->SetColor({ 0.5f,0.5f,0.5f,0.5f });

	backGround->Update();

	//========================================
	// 説明
	explanation = std::make_unique<Sprite>();
	explanation->Initialize(SpriteCommon::GetInstance(), "Resources/operation2.png");
	explanation->SetPosition({ 550.0f, 0.0f });
	explanation->SetTexSize({ 1280.0f, 720.0f });   // テクスチャの描画範囲
	explanation->SetSize({ 1280.0f, 720.0f });        // 描画サイズ

	explanation->Update();


	contGame = std::make_unique<Sprite>();
	contGame->Initialize(SpriteCommon::GetInstance(), "Resources/contGame.png");
	contGame->SetAnchorPoint({ 0.5f, 0.5f }); // アンカーポイントを中央に設定
	contGame->SetPosition({ 300.0f, 180.0f });
	contGame->SetTexSize({ 512.0f, 128.0f });   // テクスチャの描画範囲
	contGame->SetSize({ 512.0f, 256.0f });        // 描画サイズ
	contGame->Update();

	returnTitle = std::make_unique<Sprite>();
	returnTitle->Initialize(SpriteCommon::GetInstance(), "Resources/returnTitle.png");
	returnTitle->SetAnchorPoint({ 0.5f, 0.5f }); // アンカーポイントを中央に設定
	returnTitle->SetPosition({ 300.0f, 540.0f });
	returnTitle->SetTexSize({ 512.0f, 128.0f });   // テクスチャの描画範囲
	returnTitle->SetSize({ 512.0f, 256.0f });        // 描画サイズ
	returnTitle->Update();


	menuBotton = std::make_unique<Sprite>();
	menuBotton->Initialize(SpriteCommon::GetInstance(), "Resources/xbox_button_menu_outline.png");
	menuBotton->SetPosition({ 50.0f, 50.0f });
	menuBotton->SetTexSize({ 64.0f, 64.0f });   // テクスチャの描画範囲
	menuBotton->SetSize({ 64.0f, 64.0f });        // 描画サイズ

	menuBotton->Update();

	pose = std::make_unique<Sprite>();
	pose->Initialize(SpriteCommon::GetInstance(), "Resources/pose.png");
	pose->SetPosition({ 150.0f, 50.0f });
	pose->SetTexSize({ 256.0f, 64.0f });   // テクスチャの描画範囲
	pose->SetSize({ 256.0f, 64.0f });        // 描画サイズ

	pose->Update();

	pointer = std::make_unique<Sprite>();
	pointer->Initialize(SpriteCommon::GetInstance(), "Resources/pointer.png");
	pointer->SetAnchorPoint({ 0.5f,0.5f });
	pointer->SetPosition({ 640.0f, 360.0f });
	pointer->SetTexSize({ 64.0f, 64.0f });   // テクスチャの描画範囲
	pointer->SetSize({ 64.0f, 64.0f });        // 描画サイズ
	pointer->Update();
}
///=============================================================================
///						終了処理
void GameScene::Finalize() {
	BaseScene::Finalize();

	if (easy_) {
		easyBGM_->SoundStop("Resources/bgm/easy.mp3");
	} else if (nomal_) {
		nomalBGM_->SoundStop("Resources/bgm/nomal.mp3");
	} else if (hard_) {
		hardBGM_->SoundStop("Resources/bgm/hard.mp3");
	}

	skyDome_.reset();
	ground_.reset();

	player_->Finalize();
}
///=============================================================================
///						更新
void GameScene::Update() {
	/*animationManager->Update();
	lineDrawer_->SkeletonUpdate(animationManager->GetActiveAnimation("walk.gltf").skeleton);*/
	// human_->Update();

	BaseScene::Update();
	//========================================
	// フェーズ切り替え
	switch (phase_) {
		///=============================================================================
		// フェードイン
	case Phase::kFadeIn:

		if (fade_->IsFinished()) {

			phase_ = Phase::kPlay;

			// 入力の受付をオンに
			Input::GetInstance()->SetIsReception(true);
		}

		//---------------------------------------
		// プレイヤーの更新
		player_->Update();
		//---------------------------------------
		// 天球
		skyDome_->Update();
		//---------------------------------------
		// 地面
		ground_->Update();

		///=============================================================================
		// ゲームプレイ
		break;
	case Phase::kPlay:

		//---------------------------------------
		// ゲームのクリア条件
		// クリアの場合
		// TODO: クリア条件を設定
		if (false) {
			isGameClear_ = true;
			isContinue_ = false;
		}
		// ゲームオーバーの場合
		if (player_->GetHp() <= 0) {
			isGameOver_ = true;
			isContinue_ = false;
		}
		// リセット
		if (Input::GetInstance()->Triggerkey(DIK_R)) {
			isContinue_ = false;
		}
		if (!isContinue_) {
			phase_ = Phase::kFadeOut;
			fade_->Start(Fade::Status::FadeOut, fadeTime_);
		}
		if (isGameClear_) {
			phase_ = Phase::kFadeOut;
			fade_->Start(Fade::Status::FadeOut, fadeTime_);
		}

		//---------------------------------------
		// プレイヤーの更新
		//player_->Update();
		if (waveDisplayTimer_ <= 0) {
			player_->Update(); 
			cameraManager_->GetFollowCamera()->enableUpdate_ = true;
		} else {
			player_->StopMachineGunSound();
			cameraManager_->GetFollowCamera()->enableUpdate_ = false;
		}

		//---------------------------------------
		// 天球
		skyDome_->Update();

		//---------------------------------------
		// 地面
		ground_->Update();

		//---------------------------------------
		// 敵出現
		UpdateEnemyPopCommands();
		// spawnの更新
		for (const auto &Spawn : spawns_) {
			Spawn->Update();
		}
		// Spawn
		spawns_.erase(
			std::remove_if(spawns_.begin(), spawns_.end(),
						   [this](const std::unique_ptr<BaseEnemy> &spawn) {
							   if (spawn->GetSpawnHp() <= 0) {
								   // ロックオンシステムから敵を削除
								   if (lockOnSystem_) {
									   lockOnSystem_->RemoveLockedEnemy(spawn.get());
								   }

								   // 削除したエネミーをターゲットに持つプレイヤーのミサイルのターゲットをnullptrに設定
								   for (auto it = player_->GetBullets().begin(); it != player_->GetBullets().end(); ++it) {

									   if (it->get()->GetTarget() == spawn.get()) {
										   it->get()->SetTarget(nullptr);
									   }
								   }
								   return true;
							   }
							   return false;
						   }),
			spawns_.end());
		if (spawns_.empty() && waveReady_) {
			player_->StopMachineGunSound();
			enemies_.clear();

			waveIndex_++;
			if (easy_) {
				
				if (waveIndex_ < EwaveCsvPaths_.size()) {

					currentWaveImageIndex_ = waveIndex_ + 1;
					waveDisplayTimer_ = waveDisplayDuration_;

					player_->GetBullets().clear();
					player_->GetMachineGunBullets().clear();
					player_->StopMachineGunSound();

					LoadEnemyPopData(waveIndex_);
					waveReady_ = false;
				} else {
					player_->GetBullets().clear();
					player_->GetMachineGunBullets().clear();
					player_->StopMachineGunSound();
					isGameClear_ = true;
				}
			} else if (nomal_) {
				if (waveIndex_ < NwaveCsvPaths_.size()) {

					currentWaveImageIndex_ = waveIndex_ + 1;
					waveDisplayTimer_ = waveDisplayDuration_;

					player_->GetBullets().clear();
					player_->GetMachineGunBullets().clear();
					player_->StopMachineGunSound();

					LoadEnemyPopData(waveIndex_);
					waveReady_ = false;
				} else {
					player_->GetBullets().clear();
					player_->GetMachineGunBullets().clear();
					player_->StopMachineGunSound();
					isGameClear_ = true;
				}
			} else if (hard_) {
				if (waveIndex_ < HwaveCsvPaths_.size()) {

					currentWaveImageIndex_ = waveIndex_ + 1;
					waveDisplayTimer_ = waveDisplayDuration_;

					player_->GetBullets().clear();
					player_->GetMachineGunBullets().clear();
					player_->StopMachineGunSound();

					LoadEnemyPopData(waveIndex_);
					waveReady_ = false;
				} else {
					player_->GetBullets().clear();
					player_->GetMachineGunBullets().clear();
					player_->StopMachineGunSound();
					isGameClear_ = true;
				}
			}
		}
		// 敵リスト
		for (const auto &enemy : enemies_) {
			enemy->Update();
		}
		{
			std::vector<BaseEnemy *> allEnemies;
			for (const auto &e : enemies_) {
				allEnemies.push_back(e.get());
			}
			for (const auto &s : spawns_) {
				allEnemies.push_back(s.get());
			}
			AvoidOverlap(allEnemies, 4.0f);
		}
		// 敵の削除
		enemies_.erase(
			// 削除条件
			std::remove_if(enemies_.begin(), enemies_.end(),
						   [this](const std::unique_ptr<BaseEnemy> &enemy) {
							   // HPが0以下の場合
							   if (enemy->GetHp() <= 0) {
								   // ロックオンシステムから敵を削除
								   if (lockOnSystem_) {
									   lockOnSystem_->RemoveLockedEnemy(enemy.get());
								   }

								   // 削除したエネミーをターゲットに持つプレイヤーのミサイルのターゲットをnullptrに設定
								   for (auto it = player_->GetBullets().begin(); it != player_->GetBullets().end(); ++it) {

									   if (it->get()->GetTarget() == enemy.get()) {
										   it->get()->SetTarget(nullptr);
									   }
								   }

								   return true; // 削除する
							   }
							   return false; // 削除しない
						   }),
			// 実際に削除する
			enemies_.end());

		//---------------------------------------
		// ロックオンの処理追加
		if (lockOnSystem_) {
			// プレイヤーの位置をロックオンシステムにセット
			lockOnSystem_->SetPosition(player_->GetPosition());

			// カメラがFollowCameraの場合、視点方向を設定
			auto activeCamera = cameraManager_->GetActiveCamera();
			if (auto followCamera = dynamic_cast<FollowCamera *>(activeCamera)) {
				// カメラからの視点方向をロックオンシステムに設定
				lockOnSystem_->SetViewDirection(followCamera->GetForwardDirection());
			}
			std::vector<BaseEnemy *> allTargets;
			for (const auto &enemy : enemies_) {
				allTargets.push_back(enemy.get());
			}
			for (const auto &spawn : spawns_) {
				allTargets.push_back(spawn.get());
			}

			lockOnSystem_->DetectEnemiesRaw(allTargets);
			lockOnSystem_->UpdateRaw(allTargets);
			//// 敵の検出
			// lockOnSystem_->DetectEnemies(enemies_);
			// lockOnSystem_->DetectEnemies(spawns_);
			//// ロックオン更新
			// lockOnSystem_->Update(enemies_);
			// lockOnSystem_->Update(spawns_);
		}

		//---------------------------------------
		// 当たり判定
		// リセット
		collisionManager_->Reset();

		// エネミー
		for (auto &enemy : enemies_) {
			collisionManager_->AddCollider(enemy.get());
			// エネミーの弾リスト
			for (auto &bullet : enemy->GetBullets()) {
				collisionManager_->AddCollider(bullet.get());
			}
		}
		// spwan
		for (auto &spawn : spawns_) {
			collisionManager_->AddCollider(spawn.get());
		}
		// プレイヤー
		collisionManager_->AddCollider(player_.get());
		// プレイヤーの弾リスト
		for (auto &bullet : player_->GetBullets()) {
			collisionManager_->AddCollider(bullet.get());
		}
		// プレイヤーのマシンガン弾リスト
		for (auto &machineGunBullet : player_->GetMachineGunBullets()) {
			collisionManager_->AddCollider(machineGunBullet.get());
		}
		// 更新
		collisionManager_->Update();

		//---------------------------------------
		// HUD
		hud_->SetEnemiesAndSpawns(&enemies_, &spawns_);
		hud_->Update();

		//---------------------------------------
		// パーティクル
		ParticleManager::GetInstance()->Update();

		// wave sprite
		wave1_->Update();
		wave2_->Update();
		wave3_->Update();

		if (waveDisplayTimer_ > 0) {
			--waveDisplayTimer_;
			if (waveDisplayTimer_ <= 0) {
				currentWaveImageIndex_ = -1;
			}
		}

		if (Input::GetInstance()->TriggerGamePadButton(Input::GamePadButton::START))
		{
			phase_ = Phase::kPose;

			cameraManager_->GetFollowCamera()->enableUpdate_ = false; // カメラの更新を無効化

			pointer->SetPosition({ 640.0f, 360.0f }); // ポインターの位置を中央にリセット
		}

		//========================================
		// フェードアウト
		break;
	case Phase::kFadeOut:
		//---------------------------------------
		// `フェードアウトが終わった場合
		if (fade_->IsFinished()) {
			// ゲームをクリアした場合
			if (isGameClear_) {
				// ゲームクリアシーンに遷移
				SceneManager::GetInstance()->ChangeScene("CLEAR");
			} else if (isGameOver_) {
				// ゲームオーバーシーンに遷移
				SceneManager::GetInstance()->ChangeScene("OVER");
			} else {
				// タイトルシーンに遷移
				SceneManager::GetInstance()->ChangeScene("TITLE");
			}
		}

		////---------------------------------------
		//// プレイヤーの更新
		//player_->Update();

		//---------------------------------------
		// 天球
		skyDome_->Update();

		//---------------------------------------
		// 地面
		ground_->Update();

		//========================================
		// パーティクル
		ParticleManager::GetInstance()->Update();

		break;
	case Phase::kMain:
		break;
	case Phase::kPose:

	    Vector2 pointerPos = pointer->GetPosition();

	    Vector2 vect = Input::GetInstance()->GetLeftStick();
 
		pointerPos = { pointerPos.x + vect.x * 2.0f , pointerPos.y + -vect.y * 2.0f};

	    pointer->SetPosition(pointerPos);

		if (pointerPos.x < 640)
		{
			if (pointerPos.y < 360)
			{
				contGame->SetSize({ 665.6f, 166.4f });
				returnTitle->SetSize({ 512.0f, 128.0f });

				if (Input::GetInstance()->TriggerGamePadButton(Input::GamePadButton::A))
				{
					phase_ = Phase::kPlay;

					cameraManager_->GetFollowCamera()->enableUpdate_ = true; // カメラの更新を有効化
				}
			}
			else if (pointerPos.y > 360)
			{
				returnTitle->SetSize({ 665.6f, 166.4f });
				contGame->SetSize({ 512.0f, 128.0f });

				if (Input::GetInstance()->TriggerGamePadButton(Input::GamePadButton::A))
				{
					SceneManager::GetInstance()->ChangeScene("TITLE");

					cameraManager_->GetFollowCamera()->enableUpdate_ = true; // カメラの更新を有効化
				}
			}
		}
		else
		{
			contGame->SetSize({ 512.0f, 128.0f });
			returnTitle->SetSize({ 512.0f, 128.0f });
		}

		contGame->Update();
		returnTitle->Update();

		pointer->Update();

		break;
	}

	// lineDrawer_->Update();

	//========================================
	// ライト
	//
	//========================================
	// ディレクショナルライト
	directionalLight->Update();
	//========================================
	// ポイントライト
	pointLight->Update();
	//========================================
	// スポットライト
	spotLight->Update();

	//

#ifdef _DEBUG

	if (ImGui::TreeNode("directionalLight")) {
		ImGui::ColorEdit4("directionalLight.color", &directionalLight->color_.x, ImGuiColorEditFlags_None);
		if (ImGui::DragFloat3("directionalLight.direction", &directionalLight->direction_.x, 0.01f)) {
			directionalLight->direction_ = Normalize(directionalLight->direction_);
		}
		ImGui::DragFloat("directionalLight.intensity", &directionalLight->intensity_, 0.01f);
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("pointLight")) {
		ImGui::ColorEdit4("pointLight.color", &pointLight->color_.x, ImGuiColorEditFlags_None);
		ImGui::DragFloat3("pointLight.position", &pointLight->position_.x, 0.01f);
		ImGui::DragFloat("pointLight.decay", &pointLight->decay_, 0.01f);
		ImGui::DragFloat("pointLight.radius", &pointLight->radius_, 0.01f);
		ImGui::DragFloat("pointLight.intensity", &pointLight->intensity_, 0.01f);
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("spotLight")) {
		ImGui::ColorEdit4("spotlLight.color", &spotLight->color_.x, ImGuiColorEditFlags_None);
		if (ImGui::DragFloat3("spotLight.direction", &spotLight->direction_.x, 0.01f)) {
			spotLight->direction_ = Normalize(spotLight->direction_);
		}
		ImGui::DragFloat3("spotLight.position", &spotLight->position_.x, 0.01f);
		ImGui::DragFloat("spotLight.decay", &spotLight->decay_, 0.01f);
		ImGui::DragFloat("spotLight.intensity", &spotLight->intensity_, 0.01f);
		ImGui::DragFloat("spotLight.distance", &spotLight->distance_, 0.01f);
		ImGui::DragFloat("spotLight.cosAngle", &spotLight->cosAngle_, 0.01f);
		ImGui::DragFloat("spotLight.cosFalloffStart", &spotLight->cosFalloffStart_, 0.01f);
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("wave")) {
		ImGui::TextWrapped("waveDisplayTimer_ : %d", waveDisplayTimer_);
		ImGui::TextWrapped("currentWaveImageIndex_ : %d", currentWaveImageIndex_);
		ImGui::TextWrapped("waveIndex_ : %d", waveIndex_);
		ImGui::TextWrapped("waveCsvPaths_ : %d", EwaveCsvPaths_.size());
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Difficulty")) {
		ImGui::Text("easy_  : %s", easy_ ? "true" : "false");
		ImGui::Text("nomal_ : %s", nomal_ ? "true" : "false");
		ImGui::Text("hard_  : %s", hard_ ? "true" : "false");

		std::string currentCSV;
		if (easy_ && waveIndex_ < EwaveCsvPaths_.size()) {
			currentCSV = EwaveCsvPaths_[waveIndex_];
		} else if (nomal_ && waveIndex_ < NwaveCsvPaths_.size()) {
			currentCSV = NwaveCsvPaths_[waveIndex_];
		} else if (hard_ && waveIndex_ < HwaveCsvPaths_.size()) {
			currentCSV = HwaveCsvPaths_[waveIndex_];
		} else {
			currentCSV = "Out of range";
		}

		ImGui::Text("Current CSV: %s", currentCSV.c_str());

		ImGui::TreePop();
	}

	ImGui::Checkbox("useDebugCamera", &cameraManager_->useDebugCamera_);
	ImGui::Checkbox("sceneConticue", &isContinue);

	// ImGui::DragFloat("dissolve.threshold", &dissolve_->threshold, 0.01f);

	ImGui::DragFloat("MotionBlur.BlurWidth", &blur_->blurWidth_, 0.01f);
	ImGui::DragInt("MotionBlur.NumSapmles", &blur_->numSamples_);

	ShowGradientEditor();

	hud_->DrawImGui();

#endif

	lineDrawer_->Update();
}

///=============================================================================
///						描画
void GameScene::Draw() {

	switch (phase_) {
		///=============================================================================
	case Phase::kFadeIn:

		DrawBackgroundSprite();
		/// 背景スプライト描画

		DrawObject();
		/// オブジェクト描画
		//========================================
		// 天球
		skyDome_->Draw(cameraManager_->GetActiveCamera()->GetViewProjection(),
					   *directionalLight.get(),
					   *pointLight.get(),
					   *spotLight.get());
		//========================================
		// 地面
		ground_->Draw(cameraManager_->GetActiveCamera()->GetViewProjection(),
					  *directionalLight.get(),
					  *pointLight.get(),
					  *spotLight.get());
		//========================================
		// プレイヤーの描画
		player_->Draw(cameraManager_->GetActiveCamera()->GetViewProjection(),
					  *directionalLight.get(),
					  *pointLight.get(),
					  *spotLight.get());

		DrawForegroundSprite();
		/// 前景スプライト描画

		// フェード描画
		DrawFade();

		break;
		///=============================================================================
	case Phase::kPlay:

		DrawBackgroundSprite();
		/// 背景スプライト描画

		DrawObject();
		/// オブジェクト描画
		//========================================
		// 天球
		skyDome_->Draw(cameraManager_->GetActiveCamera()->GetViewProjection(),
					   *directionalLight.get(),
					   *pointLight.get(),
					   *spotLight.get());
		//========================================
		// 地面
		ground_->Draw(cameraManager_->GetActiveCamera()->GetViewProjection(),
					  *directionalLight.get(),
					  *pointLight.get(),
					  *spotLight.get());
		//========================================
		// spawnの描画
		for (const auto &Spawn : spawns_) {
			Spawn->Draw(cameraManager_->GetActiveCamera()->GetViewProjection(),
						*directionalLight.get(),
						*pointLight.get(),
						*spotLight.get());
		}
		//========================================
		// 敵
		for (const auto &enemy : enemies_) {
			enemy->Draw(cameraManager_->GetActiveCamera()->GetViewProjection(),
						*directionalLight.get(),
						*pointLight.get(),
						*spotLight.get());
		}
		//========================================
		// プレイヤーの描画
		player_->Draw(cameraManager_->GetActiveCamera()->GetViewProjection(),
					  *directionalLight.get(),
					  *pointLight.get(),
					  *spotLight.get());
		//========================================
		// LockOn
		// 🔽 LockOnの描画処理を追加
		// if(lockOnSystem_) {
		//	lockOnSystem_->Draw(cameraManager_->GetActiveCamera()->GetViewProjection(),
		//		*directionalLight.get(),
		//		*pointLight.get(),
		//		*spotLight.get());
		//}
		//========================================
		// 当たり判定マネージャ
		collisionManager_->Draw();

		//========================================
		// HUD
		hud_->Draw(cameraManager_->GetActiveCamera()->GetViewProjection());

		DrawForegroundSprite();
		/// 前景スプライト描画

		if (currentWaveImageIndex_ == 1 && wave1_) {
			wave1_->Draw();
		} else if (currentWaveImageIndex_ == 2 && wave2_) {
			wave2_->Draw();
		} else if (currentWaveImageIndex_ == 3 && wave3_) {
			wave3_->Draw();
		}

		menuBotton->Draw();

		pose->Draw();

		break;
		///=============================================================================
	case Phase::kFadeOut:

		DrawBackgroundSprite();
		/// 背景スプライト描画

		DrawObject();
		/// オブジェクト描画
		//========================================
		// 天球
		skyDome_->Draw(cameraManager_->GetActiveCamera()->GetViewProjection(),
					   *directionalLight.get(),
					   *pointLight.get(),
					   *spotLight.get());
		//========================================
		// 地面
		ground_->Draw(cameraManager_->GetActiveCamera()->GetViewProjection(),
					  *directionalLight.get(),
					  *pointLight.get(),
					  *spotLight.get());
		//========================================
		// 敵
		for (const auto &enemy : enemies_) {
			enemy->Draw(cameraManager_->GetActiveCamera()->GetViewProjection(),
						*directionalLight.get(),
						*pointLight.get(),
						*spotLight.get());
		}
		//========================================
		// プレイヤーの描画
		player_->Draw(cameraManager_->GetActiveCamera()->GetViewProjection(),
					  *directionalLight.get(),
					  *pointLight.get(),
					  *spotLight.get());
		//========================================
		// LockOn
		// 🔽 LockOnの描画処理を追加
		if (lockOnSystem_) {
			lockOnSystem_->Draw(cameraManager_->GetActiveCamera()->GetViewProjection(),
								*directionalLight.get(),
								*pointLight.get(),
								*spotLight.get());
		}
		//========================================
		// 当たり判定マネージャ
		collisionManager_->Draw();

		DrawForegroundSprite();
		/// 前景スプライト描画

		// フェード描画
		DrawFade();

		break;
		///=============================================================================
	case Phase::kMain:

		break;
		///=============================================================================
	case Phase::kPose:

		DrawBackgroundSprite();
		/// 背景スプライト描画

		DrawObject();
		/// オブジェクト描画
		//========================================
		// 天球
		skyDome_->Draw(cameraManager_->GetActiveCamera()->GetViewProjection(),
			*directionalLight.get(),
			*pointLight.get(),
			*spotLight.get());
		//========================================
		// 地面
		ground_->Draw(cameraManager_->GetActiveCamera()->GetViewProjection(),
			*directionalLight.get(),
			*pointLight.get(),
			*spotLight.get());
		//========================================
		// spawnの描画
		for (const auto& Spawn : spawns_) {
			Spawn->Draw(cameraManager_->GetActiveCamera()->GetViewProjection(),
				*directionalLight.get(),
				*pointLight.get(),
				*spotLight.get());
		}
		//========================================
		// 敵
		for (const auto& enemy : enemies_) {
			enemy->Draw(cameraManager_->GetActiveCamera()->GetViewProjection(),
				*directionalLight.get(),
				*pointLight.get(),
				*spotLight.get());
		}
		//========================================
		// プレイヤーの描画
		player_->Draw(cameraManager_->GetActiveCamera()->GetViewProjection(),
			*directionalLight.get(),
			*pointLight.get(),
			*spotLight.get());
		//========================================
		// LockOn
		// 🔽 LockOnの描画処理を追加
		// if(lockOnSystem_) {
		//	lockOnSystem_->Draw(cameraManager_->GetActiveCamera()->GetViewProjection(),
		//		*directionalLight.get(),
		//		*pointLight.get(),
		//		*spotLight.get());
		//}
		//========================================
		// 当たり判定マネージャ
		collisionManager_->Draw();

		//========================================
		// HUD
		//hud_->Draw(cameraManager_->GetActiveCamera()->GetViewProjection());

		DrawForegroundSprite();
		/// 前景スプライト描画

		if (currentWaveImageIndex_ == 1 && wave1_) {
			wave1_->Draw();
		}
		else if (currentWaveImageIndex_ == 2 && wave2_) {
			wave2_->Draw();
		}
		else if (currentWaveImageIndex_ == 3 && wave3_) {
			wave3_->Draw();
		}

		backGround->Draw();

		explanation->Draw();

		contGame->Draw();

		returnTitle->Draw();

		pointer->Draw();

		break;
	}

	//========================================
	// パーティクルの描画
	ParticleManager::GetInstance()->Draw("Resources/circle.png");

	// lineDrawer_->Draw(cameraManager_->GetActiveCamera()->GetViewProjection());
}

///=============================================================================
///                        静的メンバ関数
///--------------------------------------------------------------
///                        敵の出現データの読み込み
void GameScene::LoadEnemyPopData(int index) {
	enemyPopCommands.str("");
	enemyPopCommands.clear();
	if (easy_) {
		if (index < 0 || index >= EwaveCsvPaths_.size()) {
			return;
		}
		std::ifstream file(EwaveCsvPaths_[index]);
		assert(file.is_open());
		enemyPopCommands << file.rdbuf();
		file.close();
	} else if (nomal_) {
		if (index < 0 || index >= NwaveCsvPaths_.size()) {
			return;
		}
		std::ifstream file(NwaveCsvPaths_[index]);
		assert(file.is_open());
		enemyPopCommands << file.rdbuf();
		file.close();
	} else if (hard_) {
		if (index < 0 || index >= HwaveCsvPaths_.size()) {
			return;
		}
		std::ifstream file(HwaveCsvPaths_[index]);
		assert(file.is_open());
		enemyPopCommands << file.rdbuf();
		file.close();
	}
}
///--------------------------------------------------------------
///                        敵の出現データの更新
void GameScene::UpdateEnemyPopCommands() {
    // まず、遅延召喚キューから敵を召喚
    int spawnedThisFrame = 0;
    while (!delayedSpawnQueue_.empty() && spawnedThisFrame < spawnPerFrame_) {
        DelayedSpawnData spawnData = delayedSpawnQueue_.front();
        delayedSpawnQueue_.pop();
        
        // 実際の召喚処理
        if (spawnData.type == "KUMO") {
            SpawnEnemyKumo(spawnData.position, spawnData.hp);
        } else if (spawnData.type == "BAT") {
            SpawnEnemyBat(spawnData.position, spawnData.hp);
        } else if (spawnData.type == "BOMB") {
            SpawnEnemyBomb(spawnData.position, spawnData.hp);
        } else if (spawnData.type == "CHAIR") {
            SpawnEnemyChair(spawnData.position, spawnData.hp);
        } else if (spawnData.type == "WM") {
            SpawnEnemyWM(spawnData.position, spawnData.hp);
        } else {
            SpawnEnemy(spawnData.position);
        }
        
        spawnedThisFrame++;
    }
    
    // キューに残りがある場合は次のフレームに持ち越し
    if (!delayedSpawnQueue_.empty()) {
        return;
    }

    if (isWaiting_) {
        --waitTimer_;
        if (--waitTimer_ <= 0) {
            isWaiting_ = false;
        }
        return;
    }
    std::string line;

    while (getline(enemyPopCommands, line)) {

        std::istringstream line_stream(line);

        std::string word;

        getline(line_stream, word, ',');

        if (word.find("//") == 0) {
            continue;
        }
        if (word.find("POP") == 0) {
            getline(line_stream, word, ',');
            float x = (float)std::atof(word.c_str());

            getline(line_stream, word, ',');
            float y = (float)std::atof(word.c_str());

            getline(line_stream, word, ',');
            float z = (float)std::atof(word.c_str());
            SpawnSet(Vector3(x, y, z));

            getline(line_stream, word, ',');
            int hp = std::stoi(word);

            getline(line_stream, word, ',');
            int num = std::stoi(word);

            getline(line_stream, word, ',');
            currentSpawnType_ = word;

            float spawnRangeX = 10.0f;
            float spawnRangeY = 5.0f;
            float spawnRangeZ = 10.0f;

            std::uniform_real_distribution<float> randomX(x - spawnRangeX, x + spawnRangeX);
            std::uniform_real_distribution<float> randomY(y - spawnRangeY, y + spawnRangeY);
            std::uniform_real_distribution<float> randomZ(z - spawnRangeZ, z + spawnRangeZ);

            // 敵をキューに追加（即座に召喚せず）
            for (int i = 0; i < num; i++) {
                DelayedSpawnData spawnData;
                spawnData.hp = hp;
                spawnData.type = currentSpawnType_;
                
                if (currentSpawnType_ == "KUMO") {
                    spawnData.position = Vector3(randomX(gen), y, randomZ(gen));
                } else if (currentSpawnType_ == "BAT") {
                    spawnData.position = Vector3(randomX(gen), randomY(gen), randomZ(gen));
                } else if (currentSpawnType_ == "BOMB") {
                    spawnData.position = Vector3(randomX(gen), y, randomZ(gen));
                } else if (currentSpawnType_ == "CHAIR") {
                    spawnData.position = Vector3(randomX(gen), y, randomZ(gen));
                } else if (currentSpawnType_ == "WM") {
                    spawnData.position = Vector3(randomX(gen), y, randomZ(gen));
                } else {
                    spawnData.position = Vector3(x, y, z);
                }
                
                delayedSpawnQueue_.push(spawnData);
            }

        } else if (word.find("WAIT") == 0) {

            getline(line_stream, word, ',');

            int32_t waitTime = atoi(word.c_str());

            isWaiting_ = true;
            waitTimer_ = waitTime;

            break;
        }
    }
    if (enemyPopCommands.eof()) {
        waveReady_ = true;
    }
}
///--------------------------------------------------------------
///                        敵の出現
void GameScene::SpawnEnemy(const Vector3 &position) {

	std::unique_ptr<BaseEnemy> newEnemy = std::make_unique<Enemy>();
	if (auto *enemyNormal = dynamic_cast<Enemy *>(newEnemy.get())) {
		enemyNormal->Initialize();
		enemyNormal->SetPosition(position);
		enemyNormal->SetTarget(player_->GetWorldTransform());
	}

	enemies_.push_back(std::move(newEnemy));
}

void GameScene::SpawnEnemyKumo(const Vector3 &position, int hp) {
	std::unique_ptr<BaseEnemy> newEnemy = std::make_unique<GroundTypeEnemy>();
	if (auto *enemyNormal = dynamic_cast<GroundTypeEnemy *>(newEnemy.get())) {
		enemyNormal->Initialize();
		enemyNormal->SetPosition(position);
		enemyNormal->SetTarget(player_->GetWorldTransform());
		enemyNormal->SetHp(hp);
	}

	enemies_.push_back(std::move(newEnemy));
}

void GameScene::SpawnEnemyBat(const Vector3 &position, int hp) {
	std::unique_ptr<BaseEnemy> newEnemy = std::make_unique<SkyTypeEnemy>();
	if (auto *enemyNormal = dynamic_cast<SkyTypeEnemy *>(newEnemy.get())) {
		enemyNormal->Initialize();
		enemyNormal->SetPosition(position);
		enemyNormal->SetTarget(player_->GetWorldTransform());
		enemyNormal->SetHp(hp);
	}

	enemies_.push_back(std::move(newEnemy));
}

void GameScene::SpawnEnemyBomb(const Vector3 &position, int hp) {
	std::unique_ptr<BaseEnemy> newEnemy = std::make_unique<GroundTypeEnemy2>();
	if (auto *enemyNormal = dynamic_cast<GroundTypeEnemy2 *>(newEnemy.get())) {
		enemyNormal->Initialize();
		enemyNormal->SetPosition(position);
		enemyNormal->SetTarget(player_->GetWorldTransform());
		enemyNormal->SetHp(hp);
	}

	enemies_.push_back(std::move(newEnemy));
}

void GameScene::SpawnEnemyChair(const Vector3 &position, int hp) {
	std::unique_ptr<BaseEnemy> newEnemy = std::make_unique<GroundTypeEnemy3>();
	if (auto *enemyNormal = dynamic_cast<GroundTypeEnemy3 *>(newEnemy.get())) {
		enemyNormal->Initialize();
		enemyNormal->SetPosition(position);
		enemyNormal->SetTarget(player_->GetWorldTransform());
		enemyNormal->SetHp(hp);
	}

	enemies_.push_back(std::move(newEnemy));
}

void GameScene::SpawnEnemyWM(const Vector3 &position, int hp) {
	std::unique_ptr<BaseEnemy> newEnemy = std::make_unique<GroundTypeEnemy4>();
	if (auto *enemyNormal = dynamic_cast<GroundTypeEnemy4 *>(newEnemy.get())) {
		enemyNormal->Initialize();
		enemyNormal->SetPosition(position);
		enemyNormal->SetTarget(player_->GetWorldTransform());
		enemyNormal->SetHp(hp);
	}

	enemies_.push_back(std::move(newEnemy));
}

void GameScene::SpawnSet(const Vector3 &position) {
	std::unique_ptr<BaseEnemy> newSpawn = std::make_unique<Spawn>();
	if (auto *spawnNormal = dynamic_cast<Spawn *>(newSpawn.get())) {
		spawnNormal->Initialize();
		spawnNormal->SetPosition(position);
		spawnNormal->SetTarget(player_->GetWorldTransform());
	}

	spawns_.push_back(std::move(newSpawn));
}

void GameScene::AvoidOverlap(std::vector<BaseEnemy *> &allEnemies, float avoidRadius) {
	for (auto *self : allEnemies) {
		Vector3 avoidance = {0.0f, 0.0f, 0.0f};

		for (auto *other : allEnemies) {
			if (self == other)
				continue;

			Vector3 diff = self->GetPosition() - other->GetPosition();
			float dist = Length(diff);

			if (dist < avoidRadius && dist > 0.001f) {
				float weight = (avoidRadius - dist) / avoidRadius;
				avoidance += Normalize(diff) * weight; // 近いほど強い反発
			}
		}

		// 自然に回避加速度を適用
		avoidance *= 0.05f; // 強度調整
		self->AddAvoidance(avoidance);

		// y軸固定（必要に応じて取り外し可能）
		if (dynamic_cast<GroundTypeEnemy *>(self) || dynamic_cast<GroundTypeEnemy2 *>(self)) {
			self->SetVelocityY(0.0f);
		}
	}
}
