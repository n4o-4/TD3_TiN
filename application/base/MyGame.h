#pragma once

#include <dxgidebug.h>

#include "Framework.h"

#include "SceneFactory.h"

// MagEngine
#include "LineManager.h"

// ゲーム全体
class MyGame : public Framework
{
public:
private:


public: //　メンバ関数
	// 初期化
	void Initialize() override;

	// 終了
	void Finalize() override;

	// 毎フレーム
	void Update() override;

	// 描画
	void Draw() override;

	// 終了フラグのチェック
	bool IsEndRequest() override { return endRequest_; }

private:

	std::unique_ptr<AbstaractSceneFactory> sceneFactory_ = nullptr;

};