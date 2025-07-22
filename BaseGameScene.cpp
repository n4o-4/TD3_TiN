#include "BaseGameScene.h"

void BaseGameScene::Initialize() {
	BaseScene::Initialize();

	// ライト
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

	skyDome_ = std::make_unique<SkyDome>();
	skyDome_->Initialize();

	ground_ = std::make_unique<Ground>();
	ground_->Initialize();

	player_ = std::make_unique<Player>();
	player_->Initialize();

	lockOnSystem_ = std::make_unique<LockOn>();
	lockOnSystem_->Initialize();
	player_->SetLockOnSystem(lockOnSystem_.get());

	collisionManager_ = std::make_unique<CollisionManager>();
	collisionManager_->Initialize(32.0f);

	hud_ = std::make_unique<Hud>();

	// スプライト共通（背景・説明）
	TextureManager::GetInstance()->LoadTexture("Resources/white.png");
	TextureManager::GetInstance()->LoadTexture("Resources/operation2.png");

	backGround_ = std::make_unique<Sprite>();
	backGround_->Initialize(SpriteCommon::GetInstance(), "Resources/white.png");
	backGround_->SetSize({ 1280, 720 });
	backGround_->SetColor({ 1, 1, 1, 0.5f });

	explanation_ = std::make_unique<Sprite>();
	explanation_->Initialize(SpriteCommon::GetInstance(), "Resources/operation2.png");
	explanation_->SetSize({ 1280, 720 });
}

void BaseGameScene::Finalize() {
	BaseScene::Finalize();
	skyDome_.reset();
	ground_.reset();
	player_->Finalize();
}

void BaseGameScene::Update()
{
}

void BaseGameScene::Draw()
{
}

void BaseGameScene::UpdateCommon() {
	skyDome_->Update();
	ground_->Update();
	player_->Update();
	directionalLight->Update();
	pointLight->Update();
	spotLight->Update();
}

void BaseGameScene::DrawCommon() {
	DrawBackgroundSprite();
	DrawObject();

	skyDome_->Draw(cameraManager_->GetActiveCamera()->GetViewProjection(),
		*directionalLight.get(), *pointLight.get(), *spotLight.get());

	ground_->Draw(cameraManager_->GetActiveCamera()->GetViewProjection(),
		*directionalLight.get(), *pointLight.get(), *spotLight.get());

	player_->Draw(cameraManager_->GetActiveCamera()->GetViewProjection(),
		*directionalLight.get(), *pointLight.get(), *spotLight.get());

	DrawForegroundSprite();
}
