#pragma once

#include <memory>

#include "WinApp.h"
#include "DirectXCommon.h"
#include "Input.h"
#include "Audio.h"
#include "SrvManager.h"
#include "TextureManager.h"
#include "SpriteCommon.h"
#include "Sprite.h"
#include "Object3dCommon.h"
#include "ModelCommon.h"
#include "Object3d.h"
#include "ModelManager.h"
#include "Camera.h"
#include "ParticleManager.h"
#include "ParticleEmitter.h"
#include "ImGuiManager.h"
#include "BaseScene.h"
#include "SceneManager.h"
#include "AbstractSceneFactory.h"

#include "LineDrawerBase.h"
#include "PostEffect.h"

#include "LeakChecker.h"

class Framework
{
public:

protected:

	std::unique_ptr<WinApp> winApp = nullptr;

	DirectXCommon* dxCommon_ = nullptr;

	std::unique_ptr<SrvManager> srvManager = nullptr;

	std::unique_ptr<ModelCommon> modelCommon = nullptr;

#ifdef _DEBUG

	std::unique_ptr<ImGuiManager> imGuiManager = nullptr;

#endif
	std::unique_ptr<AbstaractSceneFactory> sceneFactory_ = nullptr;

	
	bool endRequest_ = false;

	std::unique_ptr<LineDrawerBase> lineDrawer_ = nullptr;

	std::unique_ptr<PostEffect> postEffect_ = nullptr;

public:

	virtual~Framework() = default;

	// 初期化
	virtual void Initialize();

	// 終了
	virtual void Finalize();

	// 毎フレーム更新
	virtual void Update();

	// 描画
	virtual void Draw();

	virtual void DrawEffect();

	// 終了チェック
	virtual bool IsEndRequest() { return endRequest_; }

	// 実行
	void Run();

	
};
