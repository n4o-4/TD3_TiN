#pragma once

#include "ISceneUI.h"
#include "TitleUI.h"
#include "GameUI.h"
#include "ClearUI.h"
#include "OverUI.h"
#include "TutorialUI.h"

class UIManager
{
public:
	// 初期化
	void Initialize();
	// 更新
	void Update(BaseScene* scene);
	// 描画
	void Draw();

	void ChengeScene(const std::string& sceneName);

private:

	std::unique_ptr<ISceneUI> sceneUI_ = nullptr;
};

