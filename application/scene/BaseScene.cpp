#include "BaseScene.h"
#include "AudioManager.h"
#include "SceneManager.h"
#include "ParticleManager.h"
#include "LineManager.h"

void BaseScene::Initialize()
{
	lineDrawer_ = std::make_unique<LineDrawerBase>();
	lineDrawer_->Initialize(sceneManager_->GetDxCommon(),srvManager_);

	cameraManager_ = std::make_unique<CameraManager>();	
	cameraManager_->Initialize();

	fade_ = std::make_unique<Fade>();
	fade_->Initialize();

	phase_ = Phase::kFadeIn;

	fade_->Start(Fade::Status::FadeIn, fadeTime_);

	Input::GetInstance()->SetIsReception(false);

	sceneManager_->GetPostEffect()->SetCameraManager(cameraManager_.get());

	ParticleManager::GetInstance()->Initialize(sceneManager_->GetDxCommon(), srvManager_);

	ParticleManager::GetInstance()->SetCameraManager(cameraManager_.get());

	// MagEngine
	LineManager::GetInstance()->SetDefaultCamera(&cameraManager_->GetActiveCamera()->GetViewProjection());
}

void BaseScene::Finalize()
{
	AudioManager::GetInstance()->Finalize();

	ParticleManager::GetInstance()->Finalize();
}

void BaseScene::Update()
{

	LineManager::GetInstance()->SetDefaultCamera(&cameraManager_->GetActiveCamera()->GetViewProjection());

	lineDrawer_->Update();

	cameraManager_->Update();

	fade_->Update();
}

void BaseScene::Draw()
{
}

void BaseScene::LineDraw()
{
	lineDrawer_->Draw(Camera::GetInstance()->GetViewProjection());
}

void BaseScene::DrawObject()
{

	Object3dCommon::GetInstance()->SetView();
	
}

void BaseScene::DrawBackgroundSprite()
{
	SpriteCommon::GetInstance()->SetBackgroundView();

	SpriteCommon::GetInstance()->DrawBackground();
}

void BaseScene::DrawForegroundSprite()
{
	SpriteCommon::GetInstance()->SetForegroundView();

	SpriteCommon::GetInstance()->DrawForeground();
}

void BaseScene::DrawFade()
{
	fade_->Draw();
}
