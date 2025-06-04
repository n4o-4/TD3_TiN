#pragma once

#include "BaseScene.h"
#include <string>

/// <sumary>
/// シーン工場(概念)
/// </summary>

class AbstaractSceneFactory
{
public:
	/// 仮想デストラクタ
	virtual ~AbstaractSceneFactory() = default;

	/// シーン生成
	virtual std::unique_ptr<BaseScene> CreateScene(const std::string& sceneName) = 0;
};