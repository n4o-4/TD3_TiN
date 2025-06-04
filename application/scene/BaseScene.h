#pragma once

#include <memory>
#include "SrvManager.h"
#include "Camera.h"

#include "CameraManager.h"

#include "SpriteCommon.h"
#include "Object3dCommon.h"	
#include "LineDrawerBase.h"

#include "Fade.h"

#include "PostEffect.h"

class SceneManager;

// シーン基底クラス
class BaseScene
{
public:

	enum class Phase
	{
		kFadeIn,   // フェードイン

		kMain,     // メイン部

		kPlay,     // ゲームプレイ

		kPose,     // ポーズ

		kFadeOut,  // フェードアウト
	};

public:

	virtual ~BaseScene() = default;

	// 初期化
	virtual void Initialize();

	// 終了
	virtual void Finalize();

	// 更新
	virtual void Update();

	// 描画
	virtual void Draw();

	virtual void SetSceneManager(SceneManager* sceneManager) { sceneManager_ = sceneManager; }

	virtual void SetSrvManager(SrvManager* srvManager) { srvManager_ = srvManager; }

	virtual SrvManager* GetSrvManager() { return srvManager_; }

protected:

	virtual void LineDraw();

	virtual void DrawObject();

	virtual void DrawBackgroundSprite();

	virtual void DrawForegroundSprite();

	virtual void DrawFade();

protected:

	SceneManager* sceneManager_ = nullptr;

	SrvManager* srvManager_ = nullptr;

	std::unique_ptr<LineDrawerBase> lineDrawer_ = nullptr;

	std::unique_ptr<CameraManager> cameraManager_ = nullptr;

	std::unique_ptr<Fade> fade_ = nullptr;

	float fadeTime_ = 0.4f;

	Phase phase_ = Phase::kFadeIn;

	bool isContinue = true;


private:


};