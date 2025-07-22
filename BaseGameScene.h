#pragma once
#include "BaseScene.h"
#include "SkyDome.h"
#include "Ground.h"
#include "Player.h"
#include "LockOn.h"
#include "Hud.h"
#include "CollisionManager.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Sprite.h"

class BaseGameScene : public BaseScene {
public:
	void Initialize() override;
	void Finalize() override;
	void Update() override;
	void Draw() override;

protected:
	std::unique_ptr<SkyDome> skyDome_;
	std::unique_ptr<Ground> ground_;
	std::unique_ptr<Player> player_;
	std::unique_ptr<LockOn> lockOnSystem_;
	std::unique_ptr<Hud> hud_;
	std::unique_ptr<CollisionManager> collisionManager_;

	std::unique_ptr<DirectionalLight> directionalLight;
	std::unique_ptr<PointLight> pointLight;
	std::unique_ptr<SpotLight> spotLight;

	std::unique_ptr<Sprite> backGround_;
	std::unique_ptr<Sprite> explanation_;

	// 共通の描画・更新処理
	void UpdateCommon();
	void DrawCommon();
};
