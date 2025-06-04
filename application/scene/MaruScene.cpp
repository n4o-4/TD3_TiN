#include "MaruScene.h"

void MaruScene::Initialize()
{
	BaseScene::Initialize();
}

void MaruScene::Finalize()
{
}

void MaruScene::Update()
{
}

void MaruScene::Draw()
{
	DrawBackgroundSprite();
	/// 背景スプライト描画


	DrawObject();
	/// オブジェクト描画	


	DrawForegroundSprite();
	/// 前景スプライト描画	
}
