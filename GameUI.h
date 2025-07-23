#pragma once
#include "ISceneUI.h"
class GameUI : public ISceneUI
{
public:
	~GameUI() = default;
	// 初期化
	void Initialize() override;
	// 更新
	void Update(BaseScene* scene) override;
	// 描画
	void Draw() override;


private:

	std::unique_ptr<Sprite> backGround = nullptr;

	std::unique_ptr<Sprite> explanation = nullptr; // 説明のスプライト

	std::unique_ptr<Sprite> explanation1 = nullptr;

	std::unique_ptr<Sprite> contGame = nullptr;

	std::unique_ptr<Sprite> returnTitle = nullptr;

	std::unique_ptr<Sprite> menuBotton = nullptr;

	std::unique_ptr<Sprite> pose = nullptr;

	std::unique_ptr<Sprite> pointer = nullptr;

};

