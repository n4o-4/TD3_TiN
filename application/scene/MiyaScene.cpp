#include "MiyaScene.h"

void MiyaScene::Initialize()
{
	BaseScene::Initialize();	
}

void MiyaScene::Finalize()
{
}

void MiyaScene::Update()
{
}

void MiyaScene::Draw()
{
	DrawBackgroundSprite();
	/// 背景スプライト描画


	DrawObject();
	/// オブジェクト描画	


	DrawForegroundSprite();
	/// 前景スプライト描画	
}
