#include "UIManager.h"

void UIManager::Initialize()
{
}

void UIManager::Update(BaseScene* scene)
{
	sceneUI_->Update(scene);
}

void UIManager::Draw()
{
	sceneUI_->Draw();
}

void UIManager::ChengeScene(const std::string& sceneName)
{
	std::unique_ptr<ISceneUI> newUI = nullptr;

	if (sceneName == "TITLE")
	{
		newUI = std::make_unique<TitleUI>();
	}
	else if (sceneName == "GAME")
	{
		newUI = std::make_unique<GameUI>();
	}
	else if (sceneName == "CLEAR")
	{
		newUI = std::make_unique<ClearUI>();
	}
	else if (sceneName == "OVER")
	{
		newUI = std::make_unique<OverUI>();
	}
	else if (sceneName == "TUTORIAL")
	{
		newUI = std::make_unique<TutorialUI>();
	}
	
	newUI->Initialize();

	sceneUI_ = std::move(newUI);
}
