#include "Framework.h"

#include "LeakChecker.h"

void Framework::Initialize()
{

	/*-----------------------------------
	* WinAppの生成と初期化
	-----------------------------------*/

	winApp = std::make_unique<WinApp>();
	winApp->Initialize();

	/*-----------------------------------
	* DirectXCommonの生成と初期化
	-----------------------------------*/

	DirectXCommon::GetInstance()->Initialize(winApp.get());
	dxCommon_ = DirectXCommon::GetInstance();

	/*-----------------------------------
	* Audioの初期化
	-----------------------------------*/

	

	/*-----------------------------------
	* Inputの生成と初期化
	-----------------------------------*/

	Input::GetInstance()->Initialize(winApp.get());

	/*-----------------------------------
	* SrvManagerの生成と初期化
	-----------------------------------*/


	srvManager = std::make_unique<SrvManager>();
	srvManager->Initialize(dxCommon_);

	/*-----------------------------------
	* TextureManagerの初期化
	-----------------------------------*/

	TextureManager::GetInstance()->Initialize(dxCommon_, srvManager.get());

	/*-----------------------------------
	* SpriteCommonの生成と初期化
	*
	*  スプライト共通部
	-----------------------------------*/

	SpriteCommon::GetInstance()->Initialize(dxCommon_);

	/*-----------------------------------
	* Object3dCommonの生成と初期化
	*
	*  3Dオブジェクト共通部
	-----------------------------------*/


	Object3dCommon::GetInstance()->Initialize(dxCommon_);

	/*-----------------------------------
	* ModelCommonの生成と初期化
	*
	*  モデル共通部
	-----------------------------------*/

	modelCommon = std::make_unique<ModelCommon>();
	modelCommon->Initialize(dxCommon_);


	/*-----------------------------------
	* ModelManagerの初期化
	-----------------------------------*/

	ModelManager::GetInstance()->Initialize(dxCommon_);

	/*-----------------------------------
	* Cameraの初期化
	-----------------------------------*/

	Camera::GetInstance()->Initialize();

	/*-----------------------------------
	* ParticleManagerの初期化
	-----------------------------------*/

	ParticleManager::GetInstance()->Initialize(dxCommon_, srvManager.get());
	//ParticleManager::GetInstance()->CreateParticleGroup("particle", "Resources/circle.png");

	/*-----------------------------------
	* ParticleEmitterの生成と初期化
	-----------------------------------*/


	/*particleEmitter = std::make_unique<ParticleEmitter>();
	particleEmitter->Initialize("particle");
	particleEmitter->Emit();*/

	SceneManager::GetInstance()->Initialize(dxCommon_,srvManager.get(),Camera::GetInstance());

	lineDrawer_ = std::make_unique<LineDrawerBase>();
	lineDrawer_->Initialize(dxCommon_,srvManager.get());

	postEffect_ = std::make_unique<PostEffect>();
	postEffect_->Initialize(dxCommon_,srvManager.get());

	SceneManager::GetInstance()->SetPostEffect(postEffect_.get());
}

void Framework::Finalize()
{
	SceneManager::GetInstance()->Finalize(); ///

	postEffect_->Finalize();

	ParticleManager::GetInstance()->Finalize(); ///

	Camera::GetInstance()->Finalize();

	ModelManager::GetInstance()->Finalize(); ///

	Object3dCommon::GetInstance()->Finalize(); ///

	SpriteCommon::GetInstance()->Finalize(); ///

	TextureManager::GetInstance()->Finalize(); ///

	Input::GetInstance()->Finalize(); //

	DirectXCommon::GetInstance()->Finalize();

	winApp->Finalize();

}

void Framework::Update()
{

	Camera::GetInstance()->Update();

	if (winApp->ProcessMessage()) {
		endRequest_ = true;
	}

	Input::GetInstance()->Update();

	SceneManager::GetInstance()->Update();

}

void Framework::Draw()
{

	SceneManager::GetInstance()->Draw();

}

void Framework::DrawEffect()
{
	postEffect_->Draw();
}



void Framework::Run()
{

	// ゲームの初期化
	Initialize();

	while (true)
	{

		// 毎フレーム更新
		Update();

		if (IsEndRequest())
		{
			break;
		}

		Draw();
	}

	// ゲームの終了
	Finalize();
}
