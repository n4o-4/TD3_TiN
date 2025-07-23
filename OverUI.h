#pragma once
#include "ISceneUI.h"
class OverUI : public ISceneUI
{
public:
	~OverUI() = default;
	// 初期化
	void Initialize() override;
	// 更新
	void Update(BaseScene* scene) override;
	// 描画
	void Draw() override;
};

