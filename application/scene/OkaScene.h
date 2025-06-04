#pragma once
#include "BaseScene.h"
#include "Kouro.h"
#include "Player.h"

class OkaScene : public BaseScene
{
public:
	// 初期化
	void Initialize() override;
	// 終了
	void Finalize() override;
	// 毎フレーム更新
	void Update() override;
	// 描画
	void Draw() override;

private:
	std::unique_ptr<Player> player_ = nullptr;
	std::unique_ptr<FollowCamera> followCamera_ = nullptr;
	std::unique_ptr<DirectionalLight> directionalLight = nullptr;
	std::unique_ptr<PointLight> pointLight = nullptr;
	std::unique_ptr<SpotLight> spotLight = nullptr;
};
