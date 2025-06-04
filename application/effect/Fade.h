#pragma once
#include <memory>
#include <algorithm>
//==============================
// engine
#include "TextureManager.h"
#include "SpriteCommon.h"
#include "Sprite.h"



class Fade
{
public:

	enum class Status
	{
		None,
		FadeIn,
		FadeOut
	};

public:
	// 初期化
	void Initialize();

	// 更新処理
	void Update();

	// 描画
	void Draw();

	// フェード開始
	void Start(Status status, float duration);

	// フェード終了判定
	bool IsFinished() const;

private:

	void UpdateFade();

	void UpdateFadeIn();

	void UpdateFadeOut();

private:

	// フェード用のスプライト
	std::unique_ptr<Sprite> curtain_ = nullptr;

	// フェードのステータス
	Status status_ = Status::None;

	// フェードの持続時間
	float duration_ = 0.0f;

	// 経過時間のカウンター		
	float counter_ = 0.0f;
};

