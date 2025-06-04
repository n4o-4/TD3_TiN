#pragma once

#include "BaseScene.h"

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
#include "Kouro.h"

class GameOver : public BaseScene {


public: 

	// 初期化
	void Initialize() override;
	// 終了
	void Finalize() override;
	// 毎フレーム更新
	void Update() override;
	// 描画
	void Draw() override;

	void cameraMove();
	void OpenMwdoor();
	void CloseMwdoor();

private:

	std::unique_ptr<Audio> audio = nullptr;

	std::unique_ptr<Sprite> over_ = nullptr;
	std::unique_ptr<Sprite> buttonA_ = nullptr;

	std::unique_ptr<Audio> overBGM_ = nullptr;
	std::unique_ptr<Audio> mwTinSE_ = nullptr;
	std::unique_ptr<Audio> mwSE_ = nullptr;

	// light class
	std::unique_ptr<DirectionalLight> directionalLight = nullptr;
	std::unique_ptr<PointLight> pointLight = nullptr;
	std::unique_ptr<SpotLight> spotLight = nullptr;
	std::unique_ptr<PointLight> mvPointLight = nullptr;
	
	//particle
	std::unique_ptr<ParticleEmitter> smokeEmitter_ = nullptr;
	
	//model
	std::unique_ptr<Object3d> overModel_;
	std::unique_ptr<Object3d> titleModel_;
	std::array<std::unique_ptr<Object3d>, 4> mvModels_;
	std::array<std::unique_ptr<Object3d>, 5> enemyModels_;
	//WorldTransform
	std::unique_ptr<WorldTransform> textTransform_;
	std::array<std::unique_ptr<WorldTransform>, 4> mvTransforms_;
	std::array<std::unique_ptr<WorldTransform>, 5> enemyTransforms_;


	//camera
	bool iscameraMove_ = false;
	bool isMoving_ = false;
	float cameraTimer_ = 0.0f;
	const float cameraDuration_ = 2.2f;

	//text
	bool isOver_ = true;
	bool isText_ = false;
	/*float dishTimer_ = 0.0f;
	const float dishDuration_ = 3.0f;*/

	//door
	bool isOpenDoor_ = true;
	bool isCloseDoor_ = false;
	float openDoorTimer_ = 0.0f;
	const float openDoorDuration_ = 2.2f;
	float closeDoorTimer_ = 0.0f;
	const float closeDoorDuration_ = 2.2f;

	//button
	bool showButtonA_ = false;
	float buttonABlinkTimer_ = 0.0f;

	//
	bool isMove_ = false;
	bool isStartClose_ = false;

	std::unique_ptr<ViewProjection> cameraViewProjection_ = nullptr;
};

