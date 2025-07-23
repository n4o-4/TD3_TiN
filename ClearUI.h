#pragma once
#include "ISceneUI.h"
class ClearUI : public ISceneUI
{
public:
	~ClearUI() = default;
	// 初期化
	void Initialize() override;
	// 更新
	void Update(BaseScene* scene) override;
	// 描画
	void Draw() override;
};

