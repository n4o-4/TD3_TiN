#pragma once

#include "AbstractSceneFactory.h"
#include "TitleScene.h"
#include "GameScene.h"

#include "OkaScene.h"	
#include "PakuScene.h"
#include "MaruScene.h"
#include "MiyaScene.h"
#include "GameClear.h"
#include "GameOver.h"
#include "TutorialScene.h"

// このゲーム用のシーン工場
class SceneFactory : public AbstaractSceneFactory
{
public:


	std::unique_ptr<BaseScene> CreateScene(const std::string& sceneName) override;

};