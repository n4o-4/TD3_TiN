#include "SceneFactory.h"

std::unique_ptr<BaseScene> SceneFactory::CreateScene(const std::string& sceneName)
{
    std::unique_ptr<BaseScene> newScene = nullptr;

    if (sceneName == "TITLE")
    {
        newScene = std::make_unique<TitleScene>();
    }
    else if (sceneName == "GAME")
    {
        newScene = std::make_unique<GameScene>();
    }
	


	/// 各個人のシーン
	else if (sceneName == "OKA")
	{
		newScene = std::make_unique<OkaScene>();
	}
	else if (sceneName == "PAKU")
	{
		newScene = std::make_unique<PakuScene>();
	}
	else if (sceneName == "MARU")
	{
		newScene = std::make_unique<MaruScene>();
	}
	else if (sceneName == "MIYA")
	{
		newScene = std::make_unique<MiyaScene>();
	}
	else if (sceneName == "CLEAR") 
	{
		newScene = std::make_unique<GameClear>();
	}
	else if (sceneName == "OVER") 
	{
		newScene = std::make_unique<GameOver>();
	}
	else if (sceneName == "TUTORIAL")
	{
		newScene = std::make_unique<TutorialScene>();
	}

    return newScene;
}
