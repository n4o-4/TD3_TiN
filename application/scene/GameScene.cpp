#include "GameScene.h"
#include "LineManager.h"
#include "imgui.h"

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
	spotLight->direction_ = { 0.0f, -1.0f, 0.0f };
	spotLight->position_ = { 0.0f, 2000.0f, 0.0f };
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
	enemySystem_ = std::make_unique<EnemySystem>();
	enemySystem_->Initialize();
	enemySystem_->SetPlayer(player_.get());
	enemySystem_->SetLockOnSystem(lockOnSystem_.get());
	
	// 敵出現
	//========================================
	DifficultyManager::GetInstance()->PreloadCSVFiles();
	Difficulty difficulty = DifficultyManager::GetInstance()->GetDifficulty();
	switch (difficulty) {
	case Difficulty::Easy:
		easy_ = true;
		break;
	case Difficulty::Normal:
		nomal_ = true;
		break;
	case Difficulty::Hard:
		hard_ = true;
		break;
	}
	if (DifficultyManager::GetInstance()->GetWaveCount() > 0) {
		waveIndex_ = 0;
		enemySystem_->LoadWaveData(waveIndex_, DifficultyManager::GetInstance()->GetWaveStream(waveIndex_));
		
	}
	//========================================
	// 当たり判定マネージャ
	collisionManager_ = std::make_unique<CollisionManager>();
	collisionManager_->Initialize(32.0f);

	//========================================
	// フォローカメラのターゲットを設定
	cameraManager_->SetFollowCameraTarget(player_->GetWorldTransform());
	// フォローカメラへ敵のリストの受け渡し
	cameraManager_->GetFollowCamera()->SetEnemiesAndSpawns(&enemySystem_->GetEnemies(), &enemySystem_->GetSpawns());

	// アクティブカメラをフォローカメラに設定
	cameraManager_->useFollowCamera_ = true;

	player_->SetFollowCamera(cameraManager_->GetFollowCamera());

	sceneManager_->GetPostEffect()->ApplyEffect("Blur", PostEffect::EffectType::MotionBlur);

	dissolve_ = dynamic_cast<Dissolve*>(sceneManager_->GetPostEffect()->GetEffectData("dissolve"));
	blur_ = dynamic_cast<MotionBlur*>(sceneManager_->GetPostEffect()->GetEffectData("Blur"));

	///========================================
	///		ライン描画
	lineDrawer_ = std::make_unique<LineDrawerBase>();
	lineDrawer_->Initialize(sceneManager_->GetDxCommon(), sceneManager_->GetSrvManager());

	lineDrawer_->CreateLineObject(LineDrawerBase::Type::Grid, nullptr);

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
	wave1_->SetTexSize({ 1280.0f, 720.0f });
	wave1_->SetSize({ 1280.0f, 720.0f });
	wave1_->SetPosition({ 0.0f, 0.0f });

	TextureManager::GetInstance()->LoadTexture("Resources/text/wave2.png");
	wave2_ = std::make_unique<Sprite>();
	wave2_->Initialize(SpriteCommon::GetInstance(), "Resources/text/wave2.png");
	wave2_->SetTexSize({ 1280.0f, 720.0f });
	wave2_->SetSize({ 1280.0f, 720.0f });
	wave2_->SetPosition({ 0.0f, 0.0f });

	TextureManager::GetInstance()->LoadTexture("Resources/text/wave3.png");
	wave3_ = std::make_unique<Sprite>();
	wave3_->Initialize(SpriteCommon::GetInstance(), "Resources/text/wave3.png");
	wave3_->SetTexSize({ 1280.0f, 720.0f });
	wave3_->SetSize({ 1280.0f, 720.0f });
	wave3_->SetPosition({ 0.0f, 0.0f });
	//========================================

	// HUD
	hud_ = std::make_unique<Hud>();
	// 敵とスポーンの情報を最新に保つ
	hud_->Initialize(cameraManager_->GetFollowCamera(), player_.get(), lockOnSystem_.get());
	hud_->SetEnemiesAndSpawns(&enemySystem_->GetEnemies(), &enemySystem_->GetSpawns());

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
	// その他初期値
	waveDisplayTimer_ = 0;
	currentWaveImageIndex_ = 1;
	waveDisplayTimer_ = waveDisplayDuration_;
	isGameClear_ = false;
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
	lockOnSystem_.reset();
	enemySystem_.reset();

}
///=============================================================================
///						更新
void GameScene::Update() {

	BaseScene::Update();

	//========================================
	// 関数テーブルで更新処理を呼び出す
	auto it = updateTable_.find(phase_);
	if (it != updateTable_.end()) {
		it->second(); // 対応する Update 関数を呼び出す
	}


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
		Difficulty difficulty = DifficultyManager::GetInstance()->GetDifficulty();

		// 현재 난이도 텍스트
		const char* difficultyStr = "Unknown";
		switch (difficulty) {
		case Difficulty::Easy:   difficultyStr = "Easy"; break;
		case Difficulty::Normal: difficultyStr = "Normal"; break;
		case Difficulty::Hard:   difficultyStr = "Hard"; break;
		}
		ImGui::BulletText("Current Difficulty: %s", difficultyStr);
		ImGui::BulletText("Wave Index: %d", waveIndex_);

		// 현재 CSV 파일 경로
		ImGui::Separator();
		ImGui::Text("CSV File:");
		//ImGui::TextWrapped("%s", DifficultyManager::GetInstance()->GetWaveStreamPath(waveIndex_).c_str());

		// 현재 CSV 파일의 내용 미리보기
		ImGui::Separator();
		if (ImGui::TreeNode("CSV Contents")) {
			const auto& stream = DifficultyManager::GetInstance()->GetWaveStream(waveIndex_);
			std::string content = stream.str(); // istringstream → string 변환

			ImGui::BeginChild("CSVWindow", ImVec2(0, 150), true);
			ImGui::TextWrapped("%s", content.c_str());
			ImGui::EndChild();
			ImGui::TreePop();
		}

		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Enemy Transform Debug")) {
		for (const auto& enemy : enemySystem_->GetEnemies()) {
			const auto& pos = enemy->GetPosition();
			const auto& scale = enemy->GetWorldTransform()->transform.scale;
			ImGui::Text("Enemy Pos: (%.1f, %.1f, %.1f)", pos.x, pos.y, pos.z);
			ImGui::Text("Enemy Scale: (%.1f, %.1f, %.1f)", scale.x, scale.y, scale.z);
			ImGui::Separator();
		}

		for (const auto& spawn : enemySystem_->GetSpawns()) {
			const auto& pos = spawn->GetPosition();
			const auto& scale = spawn->GetWorldTransform()->transform.scale;
			ImGui::Text("Spawn Pos: (%.1f, %.1f, %.1f)", pos.x, pos.y, pos.z);
			ImGui::Text("Spawn Scale: (%.1f, %.1f, %.1f)", scale.x, scale.y, scale.z);
			ImGui::Separator();
		}

		ImGui::TreePop();
	}
	const auto& lockedEnemies = lockOnSystem_->GetLockedEnemies();
	ImGui::Begin("LockOn Debug");
	ImGui::Text("Locked Enemy Count: %d", lockedEnemies.size());

	if (!lockedEnemies.empty()) {
		
	}
	ImGui::End();

	if (ImGui::TreeNode("Enemy Debug")) {
		ImGui::Text("Enemy Count: %zu", enemySystem_->GetEnemies().size());
		ImGui::TreePop();
	}

	ImGui::Checkbox("useDebugCamera", &cameraManager_->useDebugCamera_);
	ImGui::Checkbox("sceneConticue", &isContinue);

	// ImGui::DragFloat("dissolve.threshold", &dissolve_->threshold, 0.01f);

	ImGui::DragFloat("MotionBlur.BlurWidth", &blur_->blurWidth_, 0.01f);
	ImGui::DragInt("MotionBlur.NumSapmles", &blur_->numSamples_);

	hud_->DrawImGui();

#endif

	lineDrawer_->Update();
}

///=============================================================================
///						描画
void GameScene::Draw() {

	//========================================
	// 関数テーブルで描画処理を呼び出す
	auto it = drawTable_.find(phase_);
	if (it != drawTable_.end()) {
		it->second(); // 対応する draw 関数を呼び出す
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


void GameScene::FadeInUpdate() {
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
}

void GameScene::MainUpdate() {
}

void GameScene::PlayUpdate() {
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
	// 敵システム更新（出現処理など含む）
	// 敵システム全体のアップデート
	enemySystem_->Update();
	enemySystem_->AvoidOverlap(4.0f);
	enemySystem_->RemoveDeadEnemies(lockOnSystem_.get(), player_.get());
	enemySystem_->RemoveDeadSpawns(lockOnSystem_.get(), player_.get());

	if (lockOnSystem_) {
		// プレイヤーの位置をロックオンシステムにセット
		lockOnSystem_->SetPosition(player_->GetPosition());

		// カメラがFollowCameraの場合、視点方向を設定
		auto activeCamera = cameraManager_->GetActiveCamera();
		if (auto followCamera = dynamic_cast<FollowCamera*>(activeCamera)) {
			// カメラからの視点方向をロックオンシステムに設定
			lockOnSystem_->SetViewDirection(followCamera->GetForwardDirection());
		}
		std::vector<BaseEnemy*> allTargets;
		// 敵リストから死んだ敵は削除し、生きた敵だけを追加
		for (const auto& enemy : enemySystem_->GetEnemies()) {
			if (enemy->GetHp() <= 0) {
				lockOnSystem_->RemoveLockedEnemy(enemy.get());
			} else {
				allTargets.push_back(enemy.get());
			}
		}

		// スポーンブロックは死の概念がないので、すべて追加
		for (const auto& spawn : enemySystem_->GetSpawns()) {
			allTargets.push_back(spawn.get());
		}

		// ロックオンの検出と更新
		lockOnSystem_->DetectEnemiesRaw(allTargets);
		lockOnSystem_->UpdateRaw(allTargets);

	}
	
	hud_->Update();

	// Wave クリア判定
	if (enemySystem_->GetSpawns().empty() && enemySystem_->IsWaveReady()) {
		player_->StopMachineGunSound();
		player_->GetBullets().clear();
		player_->GetMachineGunBullets().clear();

		// [1] HUD, LockOn, Player 内部参照の削除
		if (lockOnSystem_) {
			for (const auto& enemy : enemySystem_->GetEnemies()) {
				lockOnSystem_->RemoveLockedEnemy(enemy.get());
			}
		}

		// [2] 内部リスト clear
		enemySystem_->ClearAllEnemies();

		// [3] 次の wave
		waveIndex_++;
		if (waveIndex_ < DifficultyManager::GetInstance()->GetWaveCount()) {
			currentWaveImageIndex_ = waveIndex_ + 1;
			waveDisplayTimer_ = waveDisplayDuration_;
			enemySystem_->LoadWaveData(waveIndex_, DifficultyManager::GetInstance()->GetWaveStream(waveIndex_));
		} else {
			isGameClear_ = true;
		}
	}

	// 敵の更新
	for (auto& enemy : enemySystem_->GetEnemies()) {
		enemy->Update();
	}

	// スポーンブロックの更新
	for (auto& spawn : enemySystem_->GetSpawns()) {
		spawn->Update();
	}
	//---------------------------------------
	// 当たり判定
	// リセット
	collisionManager_->Reset();

	// エネミー
	for (auto& e : enemySystem_->GetEnemies()) {
		collisionManager_->AddCollider(e.get());
		for (auto& b : e->GetBullets()) {
			collisionManager_->AddCollider(b.get());
		}
	}
	// spwan
	for (auto& s : enemySystem_->GetSpawns()) {
		collisionManager_->AddCollider(s.get());
	}
	// プレイヤー
	collisionManager_->AddCollider(player_.get());
	// プレイヤーの弾リスト
	for (auto& bullet : player_->GetBullets()) {
		collisionManager_->AddCollider(bullet.get());
	}
	// プレイヤーのマシンガン弾リスト
	for (auto& machineGunBullet : player_->GetMachineGunBullets()) {
		collisionManager_->AddCollider(machineGunBullet.get());
	}
	// 更新
	collisionManager_->Update();

	//---------------------------------------
	// HUD
	
	hud_->SetEnemiesAndSpawns(&enemySystem_->GetEnemies(), &enemySystem_->GetSpawns());
	hud_->Update();

	//---------------------------------------
	// パーティクル
	ParticleManager::GetInstance()->Update();

	// wave sprite
	wave1_->Update(); wave2_->Update(); wave3_->Update();
	if (waveDisplayTimer_ > 0 && --waveDisplayTimer_ <= 0) {
		currentWaveImageIndex_ = -1;
	}

	if (Input::GetInstance()->TriggerGamePadButton(Input::GamePadButton::START)) {
		phase_ = Phase::kPose;

		cameraManager_->GetFollowCamera()->enableUpdate_ = false; // カメラの更新を無効化

		pointer->SetPosition({ 640.0f, 360.0f }); // ポインターの位置を中央にリセット
	}

}

void GameScene::FadeOutUpdate() {
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

	//---------------------------------------
	// 天球
	skyDome_->Update();

	//---------------------------------------
	// 地面
	ground_->Update();

	//========================================
	// パーティクル
	ParticleManager::GetInstance()->Update();
}

void GameScene::PoseUpdate() {
	Vector2 pointerPos = pointer->GetPosition();

	Vector2 vect = Input::GetInstance()->GetLeftStick();

	pointerPos = { pointerPos.x + vect.x * 2.0f , pointerPos.y + -vect.y * 2.0f };

	pointer->SetPosition(pointerPos);

	if (pointerPos.x < 640) {
		if (pointerPos.y < 360) {
			contGame->SetSize({ 665.6f, 166.4f });
			returnTitle->SetSize({ 512.0f, 128.0f });

			if (Input::GetInstance()->TriggerGamePadButton(Input::GamePadButton::A)) {
				phase_ = Phase::kPlay;

				cameraManager_->GetFollowCamera()->enableUpdate_ = true; // カメラの更新を有効化
			}
		} else if (pointerPos.y > 360) {
			returnTitle->SetSize({ 665.6f, 166.4f });
			contGame->SetSize({ 512.0f, 128.0f });

			if (Input::GetInstance()->TriggerGamePadButton(Input::GamePadButton::A)) {
				SceneManager::GetInstance()->ChangeScene("TITLE");

				cameraManager_->GetFollowCamera()->enableUpdate_ = true; // カメラの更新を有効化
			}
		}
	} else {
		contGame->SetSize({ 512.0f, 128.0f });
		returnTitle->SetSize({ 512.0f, 128.0f });
	}

	contGame->Update();
	returnTitle->Update();

	pointer->Update();
}

void GameScene::FadeInDraw() {
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
}

void GameScene::MainDraw() {

}

void GameScene::PlayDraw() {
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
	for (const auto& spawn : enemySystem_->GetSpawns()) {
		spawn->Draw(cameraManager_->GetActiveCamera()->GetViewProjection(),
			*directionalLight.get(), *pointLight.get(), *spotLight.get());
	}
	//========================================
	// 敵
	for (const auto& enemy : enemySystem_->GetEnemies()) {
		enemy->Draw(cameraManager_->GetActiveCamera()->GetViewProjection(),
			*directionalLight.get(), *pointLight.get(), *spotLight.get());
	}
	//========================================
	// プレイヤーの描画
	player_->Draw(cameraManager_->GetActiveCamera()->GetViewProjection(),
		*directionalLight.get(),
		*pointLight.get(),
		*spotLight.get());

	//========================================
	// 当たり判定マネージャ
	collisionManager_->Draw();

	//========================================
	// HUD
	hud_->Draw(cameraManager_->GetActiveCamera()->GetViewProjection());
	//hud_->Update();
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
}

void GameScene::FadeOutDraw() {
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
	for (const auto& enemy : enemySystem_->GetEnemies()) {
		enemy->Draw(cameraManager_->GetActiveCamera()->GetViewProjection(),
			*directionalLight.get(), *pointLight.get(), *spotLight.get());
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
}

void GameScene::PoseDraw() {
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
	for (const auto& spawn : enemySystem_->GetSpawns()) {
		spawn->Draw(cameraManager_->GetActiveCamera()->GetViewProjection(),
			*directionalLight.get(), *pointLight.get(), *spotLight.get());
	}
	//========================================
	// 敵
	for (const auto& enemy : enemySystem_->GetEnemies()) {
		enemy->Draw(cameraManager_->GetActiveCamera()->GetViewProjection(),
			*directionalLight.get(), *pointLight.get(), *spotLight.get());
	}
	//========================================
	// プレイヤーの描画
	player_->Draw(cameraManager_->GetActiveCamera()->GetViewProjection(),
		*directionalLight.get(),
		*pointLight.get(),
		*spotLight.get());

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
	} else if (currentWaveImageIndex_ == 2 && wave2_) {
		wave2_->Draw();
	} else if (currentWaveImageIndex_ == 3 && wave3_) {
		wave3_->Draw();
	}

	backGround->Draw();

	explanation->Draw();

	contGame->Draw();

	returnTitle->Draw();

	pointer->Draw();
}
