#pragma once

#include "BaseScene.h"
#include"Kouro.h"
#include "audio.h"
#include "Input.h"
#include "Sprite.h"
#include "SpriteCommon.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "Object3d.h"
#include "Camera.h"
#include "ParticleManager.h"
#include "ParticleEmitter.h"
#include "SceneManager.h"
#include "Audio.h"

class GameClear : public BaseScene {

public:

	// 初期化
	void Initialize() override;

	// 終了
	void Finalize() override;

	// 毎フレーム更新
	void Update() override;

	// 描画
	void Draw() override;
	
	void UpdateMoveMwbody();

	void UpdateMissileFlight();

	void UpdateMoveClear();

	void UpdateCloseDoor();

private:

	std::unique_ptr<Audio> audio = nullptr;

	std::unique_ptr<Sprite> clear_ = nullptr;

	std::unique_ptr<Audio> clearBGM_ = nullptr;
	std::unique_ptr<Audio> mwSE_ = nullptr;
	std::unique_ptr<Audio> mwTinSE_ = nullptr;
	std::array<std::unique_ptr<Audio>, 4> explosionSE_;

	// ライトクラス
	std::unique_ptr<DirectionalLight> directionalLight = nullptr;
	std::unique_ptr<PointLight> pointLight = nullptr;
	std::unique_ptr<SpotLight> spotLight = nullptr;
	std::unique_ptr<PointLight> mvPointLight = nullptr;
	std::unique_ptr<SpotLight> mvSpotLight = nullptr;

	// model
	std::unique_ptr<Object3d> mwbody_;
	std::unique_ptr<Object3d> mwdoor_;
	std::unique_ptr<Object3d> mwdish_;
	std::unique_ptr<Object3d> clearModel_;
	std::unique_ptr<Object3d> mvSceneModel_;
	std::unique_ptr<Object3d> titleModel_;
	std::array<std::unique_ptr<Object3d>, 4> missileModels_;
	std::array<std::unique_ptr<Object3d>, 4> spawnModels_;

	// ワールド変換
	std::unique_ptr<WorldTransform> mwbodyTransform_;
	std::unique_ptr<WorldTransform> mwdoorTransform_;
	std::unique_ptr<WorldTransform> mwdishTransform_;
	std::unique_ptr<WorldTransform> clearTransform_;
	std::unique_ptr<WorldTransform> mvSceneTransform_;
	std::array<std::unique_ptr<WorldTransform>, 4> missileTransforms_;
	std::array<std::unique_ptr<WorldTransform>, 4> spawnTransforms_;

	// パーティクル
	std::unique_ptr<ParticleEmitter> smokeEmitter_ = nullptr;
	std::array<std::unique_ptr<ParticleEmitter>, 4> explosionEmitter_;

	//
	std::unique_ptr<Sprite> buttonA_ = nullptr;

	//
	float moveTimer_ = 0.0f;
	const float moveDuration_ = 3.2f;
	bool isMoving_ = true;

	//
	std::array<bool, 4> isMissileFlying_ = { true, true, true, true };
	std::array<bool, 4> isVisible_ = { true, true, true, true };
	std::array<float, 4> missileTimers_ = { 0.0f, 0.0f, 0.0f, 0.0f };
	std::array<float, 4> missileDurations_ = { 1.2f, 1.0f, 1.4f, 0.9f };
	std::array<Vector3, 4> missileStartPositions_;
	std::array<Vector3, 4> missileEndPositions_ = {
		Vector3{31.4f, 22.5f, 38.5f},
		Vector3{15.2f, 13.8f, 47.5f},
		Vector3{-13.7f, 7.1f, 44.8f},
		Vector3{-29.5f, 20.7f, 43.6f}
	};
	int currentMissileIndex_ = 0;
	float missileFireInterval_ = 1.5f;
	float missileFireTimer_ = 0.0f;

	float missileTimer_ = 0.0f;
	const float missileDuration_ = 1.0f;
	bool isMissileMoving_ = true;
	bool isMissileVisible_ = true;

	bool isClear_ = true;
	bool isDoor_ = true;
	float doorTimer_ = 0.0f;
	const float doorDuration_ = 2.0f;

	//
	bool showButtonA_ = false;
	float buttonABlinkTimer_ = 0.0f;

	//
	bool isClose_ = false;
	bool isStartClose_ = false;
	bool isMove_ = false;
	bool isText_ = false;
	float closeTimer_ = 0.0f;
	const float closeDuration_ = 2.4f;

};

