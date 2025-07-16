#pragma once

#include "BaseScene.h"
#include "Kouro.h"
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
#include "SkyDome.h" 

enum class SequenceState {
	RotateDish,       
	RotateDoor,       
	MoveTinToFirst,   
	MoveTinToSecond,  
	ResetDoorRotation,
	Done              
};


class TitleScene : public BaseScene
{
private:

	std::unique_ptr<Sprite> sprite = nullptr;

	std::unique_ptr<ParticleEmitter> particleEmitter_1 = nullptr;

	// 天球
	std::unique_ptr<SkyDome> skyDome_ = nullptr;

	// ライトクラス
	std::unique_ptr<DirectionalLight> directionalLight = nullptr;
	std::unique_ptr<PointLight> pointLight = nullptr;
	std::unique_ptr<SpotLight> spotLight = nullptr;
	std::unique_ptr<PointLight> mvPointLight = nullptr;

	//model
	std::array<std::unique_ptr<Object3d>, 6> mvModels_;
	std::array<std::unique_ptr<WorldTransform>, 6> mvTransforms_;

	std::array<std::unique_ptr<Object3d>, 4> textModels_;
	std::unique_ptr<WorldTransform> textTransform_;

	std::unique_ptr<Object3d> tinModel_ = nullptr;
	std::unique_ptr<WorldTransform> tinTransform_;
	//
	std::unique_ptr<Sprite> title_ = nullptr;
	std::unique_ptr<Sprite> start1_ = nullptr;
	std::unique_ptr<Sprite> start2_ = nullptr;

	std::array<std::unique_ptr<Sprite>, 5> selectSprites_;

	std::unique_ptr<Sprite> select1_ = nullptr;
	std::unique_ptr<Sprite> select2_ = nullptr;
	std::unique_ptr<Sprite> select3_ = nullptr;
	std::unique_ptr<Sprite> select4_ = nullptr;




	bool tutorial = false;
	bool easy = false;
	bool nomal = false;
	bool hard = false;

	int selectNum = 0;
	bool stickReleased_ = true;


	//
	bool music = false;
	std::unique_ptr<Audio> se1_ = nullptr;
	std::unique_ptr<Audio> se2_ = nullptr;
	std::unique_ptr<Audio> se3_ = nullptr;
	std::unique_ptr<Audio> tin_ = nullptr;
	std::unique_ptr<Audio> bgm_ = nullptr;

	std::unique_ptr<Audio> audio = nullptr;


	SequenceState sequenceState_ = SequenceState::RotateDish;
	float dishAngle_ = 0.0f;

	float selectFloatTimer_ = 0.0f;

private:
public: // メンバ関数

	// 初期化
	void Initialize() override;

	// 終了
	void Finalize() override;

	// 毎フレーム更新
	void Update() override;

	// 描画
	void Draw() override;

	void select();

	void UpdateSequence();

	bool GetEasy() const { return easy; }
	bool GetNomal() const { return nomal; }
	bool GetHard() const { return hard; }
};