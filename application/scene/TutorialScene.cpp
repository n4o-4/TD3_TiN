#include "TutorialScene.h"
#include "TutorialEnemy.h"
#include <cstdlib>

void TutorialScene::Initialize()
{
	//========================================
	// 基底シーン
	BaseScene::Initialize();

	//========================================
	// テクスチャの読み込み
	TextureManager::GetInstance()->LoadTexture("Resources/white.png");
	TextureManager::GetInstance()->LoadTexture("Resources/operation2.png");
	TextureManager::GetInstance()->LoadTexture("Resources/CheckBox.png");
	TextureManager::GetInstance()->LoadTexture("Resources/checkMark.png");
	TextureManager::GetInstance()->LoadTexture("Resources/B_select.png");
	//========================================
	// 背景

	backGround = std::make_unique<Sprite>();
	backGround->Initialize(SpriteCommon::GetInstance(), "Resources/white.png");
	backGround->SetPosition({ 840.0f, 0.0f });
	backGround->SetTexSize({ 1.0f, 1.0f });   // テクスチャの描画範囲
	backGround->SetSize({ 1280.0f, 720.0f });        // 描画サイズ

	backGround->SetColor({ 1.0f,1.0f,1.0f,0.5f });

	backGround->Update();

	//========================================
	// 説明
	explanation = std::make_unique<Sprite>();
	explanation->Initialize(SpriteCommon::GetInstance(), "Resources/operation2.png");
	explanation->SetPosition({ 700.0f, 0.0f });
	explanation->SetTexSize({ 1280.0f, 720.0f });   // テクスチャの描画範囲
	explanation->SetSize({ 1280.0f, 720.0f });        // 描画サイズ

	explanation->Update();

	//========================================
	// B_select
	B_select = std::make_unique<Sprite>();
	B_select->Initialize(SpriteCommon::GetInstance(), "Resources/B_select.png");
	B_select->SetTexSize({ 1024.0f, 315.0f });   // ←元画像サイズ
	B_select->SetSize({ 300.0f, 92.0f });        // ←表示サイズ（画面に合わせて調整）
	B_select->SetPosition({ 920.0f, 600.0f });   // ←表示位置（画面右下とかにしたいなら）

	B_select->Update();

	for (int i = 0; i < 6; ++i) {
		checkBox_[i] = std::make_unique<Sprite>();
		checkBox_[i]->Initialize(SpriteCommon::GetInstance(), "Resources/CheckBox.png");
		checkBox_[i]->SetPosition({ 880.0f, 160.0f + (i * 76.0f) });
		checkBox_[i]->SetTexSize({ 128.0f, 128.0f });   // テクスチャの描画範囲
		checkBox_[i]->SetSize({ 48.0f, 48.0f });        // 描画サイズ
		checkBox_[i]->Update();
	}

	for (int i = 0; i < 6; ++i) {
		checkMark_[i] = std::make_unique<Sprite>();
		checkMark_[i]->Initialize(SpriteCommon::GetInstance(), "Resources/checkMark.png");
		checkMark_[i]->SetPosition({ 880.0f, 160.0f + (i * 76.0f) });
		checkMark_[i]->SetTexSize({ 128.0f, 128.0f });   // テクスチャの描画範囲
		checkMark_[i]->SetSize({ 48.0f, 48.0f });        // 描画サイズ
		checkMark_[i]->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f }); // 初期は透明
		checkMark_[i]->Update();
	}

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

	//========================================
	// HUD
	hud_ = std::make_unique<Hud>();
	// 敵とスポーンの情報を最新に保つ
	hud_->SetEnemiesAndSpawns(&enemies_, &spawns_);
	hud_->Initialize(cameraManager_->GetFollowCamera(), player_.get(), lockOnSystem_.get());

	tutorialPhase_ = TutorialPhase::kExplain;

	srand(static_cast<unsigned int>(time(nullptr)));
}

void TutorialScene::Finalize()
{
	skyDome_.reset();
	ground_.reset();

	player_->Finalize();
}

void TutorialScene::Update()
{
	BaseScene::Update();

	bool allTrue = true;
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

		// Bボタンでタイトルに戻る
		if (Input::GetInstance()->TriggerGamePadButton(Input::GamePadButton::B)) {
			isContinue_ = false;
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

		

		//---------------------------------------
		// ロックオンの処理追加
		if (lockOnSystem_ && tutorialPhase_ == TutorialPhase::kPlay) {
			lockOnSystem_->SetPosition(player_->GetPosition());

			// カメラがFollowCameraの場合、視点方向を設定
			auto activeCamera = cameraManager_->GetActiveCamera();
			if (auto followCamera = dynamic_cast<FollowCamera*>(activeCamera)) {
				// カメラからの視点方向をロックオンシステムに設定
				lockOnSystem_->SetViewDirection(followCamera->GetForwardDirection());
			}
			std::vector<BaseEnemy*> allTargets;
			for (const auto& enemy : enemies_) {
				allTargets.push_back(enemy.get());
			}
			for (const auto& spawn : spawns_) {
				allTargets.push_back(spawn.get());
			}

			lockOnSystem_->DetectEnemiesRaw(allTargets);
			lockOnSystem_->UpdateRaw(allTargets);
		}


		//---------------------------------------
		// 当たり判定
		// リセット
		collisionManager_->Reset();

		// spwan
		for (auto& spawn : spawns_) {
			collisionManager_->AddCollider(spawn.get());
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
		// パーティクル
		ParticleManager::GetInstance()->Update();

		//----------------------------------------
		// Tutorialフェーズ
		switch (tutorialPhase_) {
		// 説明フェーズ
		case TutorialPhase::kExplain:

			//playerが動いたらtrue

			CheckMissions();

			
			for (int i = 0; i < 6; ++i) {
				if (!missionFlags_[i]) {
					allTrue = false;
					break;
				}
			}

			if (allTrue) {
				tutorialPhase_ = TutorialPhase::kPlay;

				for (int i = 0; i < 10; ++i) {
					auto enemy = std::make_unique<TutorialEnemy>();
					enemy->Initialize(ModelManager::GetInstance()->FindModel("enemy/kumo/kumo.obj"));

					// XZ平面上にランダムにばらけて配置（例: -50〜50の範囲）
					float x = static_cast<float>(rand() % 101 - 50); // -50〜+50
					float z = static_cast<float>(rand() % 101 - 50); // -50〜+50

					enemy->SetPosition({ x, 0.0f, z });
					enemy->SetTarget(nullptr);
					enemy->SetVelocity({ 0.0f, 0.0f, 0.0f });

					enemies_.push_back(std::move(enemy));
				}
			}

			break;
		// プレイフェーズ
		case TutorialPhase::kPlay:

			// 敵リスト
			for (const auto& enemy : enemies_) {
				enemy->Update();
			}
			{
				std::vector<BaseEnemy*> allEnemies;
				for (const auto& e : enemies_) {
					allEnemies.push_back(e.get());
				}
				for (const auto& s : spawns_) {
					allEnemies.push_back(s.get());
				}
			}
			// 敵の削除
			enemies_.erase(
				// 削除条件
				std::remove_if(enemies_.begin(), enemies_.end(),
					[this](const std::unique_ptr<BaseEnemy>& enemy) {
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

			// エネミー
			for (auto& enemy : enemies_) {
				collisionManager_->AddCollider(enemy.get());
				// エネミーの弾リスト
				for (auto& bullet : enemy->GetBullets()) {
					collisionManager_->AddCollider(bullet.get());
				}
			}
			collisionManager_->Update();

			break;
		}

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

		//---------------------------------------
		// プレイヤーの更新
		player_->Update();

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

#endif

}

void TutorialScene::Draw()
{
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
		// 当たり判定マネージャ
		collisionManager_->Draw();

		//========================================
		// HUD
		if (tutorialPhase_ == TutorialPhase::kPlay) {
		hud_->Draw(cameraManager_->GetActiveCamera()->GetViewProjection());
		}

		DrawForegroundSprite();

		B_select->Draw();

		// フェード描画
		switch (tutorialPhase_) {
		case TutorialPhase::kExplain:

			DrawBackgroundSprite();
			/// 背景スプライト描画

			DrawObject();
			/// オブジェクト描画

			DrawForegroundSprite();
			/// 前景スプライト描画

			backGround->Draw();

			explanation->Draw();


			for (int i = 0; i < 6; ++i) {
				checkBox_[i]->Draw();
			}

			for (int i = 0; i < 6; ++i) {
				if (missionFlags_[i]) {
					checkMark_[i]->Draw();
				}
			}

			break;
		case TutorialPhase::kPlay:

			DrawBackgroundSprite();
			/// 背景スプライト描画

			DrawObject();
			/// オブジェクト描画

			for (const auto& enemy : enemies_) {
				enemy->Draw(cameraManager_->GetActiveCamera()->GetViewProjection(),
					*directionalLight.get(),
					*pointLight.get(),
					*spotLight.get());
			}

			DrawForegroundSprite();
			/// 前景スプライト描画



			break;
		}

		/// 前景スプライト描画

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
		if (tutorialPhase_ == TutorialPhase::kPlay && lockOnSystem_) {
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

		break;
	}

	//========================================
	// パーティクルの描画
	ParticleManager::GetInstance()->Draw("Resources/circle.png");
}
