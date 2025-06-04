#include "OkaScene.h"

void OkaScene::Initialize()
{
	BaseScene::Initialize();

	directionalLight = std::make_unique<DirectionalLight>();
	directionalLight->Initilaize();

	pointLight = std::make_unique<PointLight>();
	pointLight->Initilize();

	spotLight = std::make_unique<SpotLight>();
	spotLight->Initialize();

	// プレイヤーを生成
	player_ = std::make_unique<Player>();
	player_->Initialize();

}

void OkaScene::Finalize()
{
	player_.reset();
}

void OkaScene::Update()
{
	BaseScene::Update();

	// プレイヤーの更新
	player_->Update();

}

void OkaScene::Draw()
{
	DrawBackgroundSprite();
	/// 背景スプライト描画


	DrawObject();
	/// オブジェクト描画	

	// プレイヤーの描画
	player_->Draw(Camera::GetInstance()->GetViewProjection(),
		*directionalLight.get(),
		*pointLight.get(),
		*spotLight.get());

	DrawForegroundSprite();
	/// 前景スプライト描画	

	
}
