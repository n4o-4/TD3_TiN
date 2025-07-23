#pragma once

#include "BaseScene.h"

class ISceneUI
{
public:
	virtual ~ISceneUI() = default;
	// 初期化
	virtual void Initialize() = 0;
	// 更新
	virtual void Update(BaseScene* scene) = 0;
	// 描画
	virtual void Draw() = 0;

protected:

	BaseScene* scene_ = nullptr; // シーンへのポインタ

};

