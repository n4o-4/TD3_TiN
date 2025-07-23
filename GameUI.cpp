#include "GameUI.h"

#include "SceneManager.h"

void GameUI::Initialize()
{
	// テクスチャ読み込み

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

void GameUI::Update(BaseScene* scene)
{
	scene_ = scene; // シーンへのポインタを設定

	BaseScene::Phase phase = scene->GetPhase();

	switch (phase)
	{
	case BaseScene::Phase::kFadeIn:
		break;

	case BaseScene::Phase::kMain:
		break;

	case BaseScene::Phase::kPlay:

		if (Input::GetInstance()->TriggerGamePadButton(Input::GamePadButton::START)) 
		{
			pointer->SetPosition({ 640.0f, 360.0f }); // ポインターの位置を中央にリセット
		}

		break;

	case BaseScene::Phase::kFadeOut:
		break;

	case BaseScene::Phase::kPose:

		Vector2 pointerPos = pointer->GetPosition();

		Vector2 vect = Input::GetInstance()->GetLeftStick();

		pointerPos = { pointerPos.x + vect.x * 2.0f , pointerPos.y + -vect.y * 2.0f };

		pointer->SetPosition(pointerPos);

		if (pointerPos.x < 640) 
		{
			if (pointerPos.y < 360) {
				contGame->SetSize({ 665.6f, 166.4f });
				returnTitle->SetSize({ 512.0f, 128.0f });

				if (Input::GetInstance()->TriggerGamePadButton(Input::GamePadButton::A)) 
				{
					scene->SetPhase(BaseScene::Phase::kPlay); // ゲームプレイに戻る
					scene->GetCameraManager()->GetFollowCamera()->enableUpdate_ = true; // カメラの更新を有効化
				}
			}
			else if (pointerPos.y > 360) {
				returnTitle->SetSize({ 665.6f, 166.4f });
				contGame->SetSize({ 512.0f, 128.0f });

				if (Input::GetInstance()->TriggerGamePadButton(Input::GamePadButton::A)) 
				{
					scene_->GetSceneManager()->ChangeScene("TITLE"); // タイトルシーンに戻る

					break;
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
}

void GameUI::Draw()
{
	BaseScene::Phase phase = scene_->GetPhase();

	switch (phase)
	{
	case BaseScene::Phase::kFadeIn:
		break;

	case BaseScene::Phase::kMain:
		break;

	case BaseScene::Phase::kPlay:

		menuBotton->Draw();

		pose->Draw();

		break;

    case BaseScene::Phase::kFadeOut:
		break;

	case BaseScene::Phase::kPose:

		backGround->Draw();

		explanation->Draw();

		contGame->Draw();

		returnTitle->Draw();

		pointer->Draw();

		break;
	}
}
