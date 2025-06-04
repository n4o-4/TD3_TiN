#include "GameClear.h"

void GameClear::Initialize() {
	BaseScene::Initialize();

	// ライト
	// 指向性
	directionalLight = std::make_unique<DirectionalLight>();
	directionalLight->Initilaize();
	directionalLight->intensity_ = 0.0f;
	// 点光源
	pointLight = std::make_unique<PointLight>();
	pointLight->Initilize();
	pointLight->intensity_ = 0.0f;

	mvPointLight = std::make_unique<PointLight>();
	mvPointLight->Initilize();
	mvPointLight->position_ = { -0.3f,2.0f,0.8f };
	mvPointLight->intensity_ = 2.7f;
	mvPointLight->decay_ = 1.0f;
	mvPointLight->color_ = { 0.925f, 0.498f, 0.118f, 1.0f };

	// スポットライト
	spotLight = std::make_unique<SpotLight>();
	spotLight->Initialize();
	spotLight->direction_ = { 0.0f,-1.0f,0.0f };
	spotLight->position_ = { 0.0f,3200.0f,0.0f };
	spotLight->intensity_ = 11.0f;
	spotLight->decay_ = 0.87f;
	spotLight->distance_ = 4800.0f;
	spotLight->cosAngle_ = 0.2f;
	spotLight->cosFalloffStart_;

	mvSpotLight = std::make_unique<SpotLight>();
	mvSpotLight->Initialize();
	mvSpotLight->direction_ = { 0.0f,-1.0f,0.0f };
	mvSpotLight->position_ = { 0.0f,3.0f,-1.0f };
	mvSpotLight->intensity_ = 11.0f;
	mvSpotLight->decay_ = 0.87f;
	mvSpotLight->distance_ = 4800.0f;
	mvSpotLight->cosAngle_ = 0.2f;
	mvSpotLight->cosFalloffStart_;
	mvSpotLight->color_ = { 0.925f, 0.498f, 0.118f, 1.0f };



	//clear
	TextureManager::GetInstance()->LoadTexture("Resources/scene/game_clear.png");
	clear_ = std::make_unique<Sprite>();
	clear_->Initialize(SpriteCommon::GetInstance(), "Resources/scene/game_clear.png");
	clear_->SetTexSize({ 1280.0f,720.0f });
	clear_->SetSize({ 1280.0f,720.0f });
	clear_->SetPosition({ 0.0f,0.0f });

	//BGM,SE
	AudioManager::GetInstance()->Initialize();
	AudioManager::GetInstance()->SoundLoadFile("Resources/bgm/gameCLEAR.mp3");
	AudioManager::GetInstance()->SoundLoadFile("Resources/se/tin.mp3");
	AudioManager::GetInstance()->SoundLoadFile("Resources/se/select.mp3");
	AudioManager::GetInstance()->SoundLoadFile("Resources/se/explosion.mp3");

	clearBGM_ = std::make_unique<Audio>();
	clearBGM_->Initialize();
	clearBGM_->SoundPlay("Resources/bgm/gameCLEAR.mp3", 9999);
	clearBGM_->SetVolume(0.6f);

	for (int i = 0; i < 4; ++i) {

		explosionSE_[i] = std::make_unique<Audio>();
		explosionSE_[i]->Initialize();
	}

	

	mwSE_ = std::make_unique<Audio>();
	mwSE_->Initialize();

	mwTinSE_ = std::make_unique<Audio>();
	mwTinSE_->Initialize();

	//Button A
	TextureManager::GetInstance()->LoadTexture("Resources/scene/A1.png");
	buttonA_ = std::make_unique<Sprite>();
	buttonA_->Initialize(SpriteCommon::GetInstance(), "Resources/scene/A1.png");
	buttonA_->SetTexSize({ 220.0f,220.0f });
	buttonA_->SetSize({ 168.0f,172.0f });
	buttonA_->SetPosition({ 1100.0f,254.0f });

	//model

	ModelManager::GetInstance()->LoadModel("scene/MWbody.obj");
	ModelManager::GetInstance()->LoadModel("scene/MWdoor.obj");
	ModelManager::GetInstance()->LoadModel("scene/dish.obj");
	ModelManager::GetInstance()->LoadModel("scene/Clear.obj");
	ModelManager::GetInstance()->LoadModel("Spawn/Spawn.obj");
	ModelManager::GetInstance()->LoadModel("missile.obj");
	ModelManager::GetInstance()->LoadModel("scene/MWScenen.obj");
	ModelManager::GetInstance()->LoadModel("scene/Title.obj");

	mwbody_ = std::make_unique<Object3d>();
	mwbody_->Initialize(Object3dCommon::GetInstance());
	mwbody_->SetModel("scene/MWbody.obj");

	mwdoor_ = std::make_unique<Object3d>();
	mwdoor_->Initialize(Object3dCommon::GetInstance());
	mwdoor_->SetModel("scene/MWdoor.obj");

	mwdish_ = std::make_unique<Object3d>();
	mwdish_->Initialize(Object3dCommon::GetInstance());
	mwdish_->SetModel("scene/dish.obj");

	clearModel_ = std::make_unique<Object3d>();
	clearModel_->Initialize(Object3dCommon::GetInstance());
	clearModel_->SetModel("scene/Clear.obj");

	

	mvSceneModel_ = std::make_unique<Object3d>();
	mvSceneModel_->Initialize(Object3dCommon::GetInstance());
	mvSceneModel_->SetModel("scene/MWScenen.obj");

	titleModel_ = std::make_unique<Object3d>();
	titleModel_->Initialize(Object3dCommon::GetInstance());
	titleModel_->SetModel("scene/Title.obj");

	// 初期位置を設定
	mwbodyTransform_ = std::make_unique<WorldTransform>();
	mwbodyTransform_->Initialize();
	mwbodyTransform_->transform.translate = { 0.0f, 1.0f , 1.0f };
	mwbodyTransform_->transform.rotate = { 0.0f, 3.14f, 0.0f };

	mwdoorTransform_ = std::make_unique<WorldTransform>();
	mwdoorTransform_->Initialize();
	mwdoorTransform_->transform.translate = { 1.37f, 0.0f , 0.94f };
	
	mwdishTransform_ = std::make_unique<WorldTransform>();
	mwdishTransform_->Initialize();
	mwdishTransform_->transform.translate = { 0.3f, 0.3f , 0.2f };
	mwdishTransform_->transform.scale = { 0.56f,0.7f,0.56f };

	clearTransform_ = std::make_unique<WorldTransform>();
	clearTransform_->Initialize();
	clearTransform_->transform.translate = { 0.9f, 0.4f , 0.2f };
	clearTransform_->transform.scale = { 0.5f, 0.5f , 1.0f };


	/*spawnTransform_ = std::make_unique<WorldTransform>();
	spawnTransform_->Initialize();
	spawnTransform_->transform.translate = { 23.5f, 10.6f , 38.3f };

	missileTransform_ = std::make_unique<WorldTransform>();
	missileTransform_->Initialize();
	missileTransform_->transform.translate = { 3.6f, 50.7f , 38.3f };
	missileTransform_->transform.rotate = { 0.1f, 1.6f, -1.0f };*/

	mvSceneTransform_ = std::make_unique<WorldTransform>();
	mvSceneTransform_->Initialize();
	mvSceneTransform_->transform.translate = { 0.0f, 0.0f , 0.0f };

	

	for (int i = 0; i < 4; ++i) {
		missileModels_[i] = std::make_unique<Object3d>();
		missileModels_[i]->Initialize(Object3dCommon::GetInstance());
		missileModels_[i]->SetModel("missile.obj");

		spawnModels_[i] = std::make_unique<Object3d>();
		spawnModels_[i]->Initialize(Object3dCommon::GetInstance());
		spawnModels_[i]->SetModel("Spawn/Spawn.obj");

		missileTransforms_[i] = std::make_unique<WorldTransform>();
		missileTransforms_[i]->Initialize();
		missileTransforms_[i]->transform.translate = { 0.6f, 60.7f , 50.3f };
		missileTransforms_[i]->transform.rotate = { 0.1f, 1.6f, -1.0f };
		missileStartPositions_[i] = missileTransforms_[i]->transform.translate;

		spawnTransforms_[i] = std::make_unique<WorldTransform>();
		spawnTransforms_[i]->Initialize();
		spawnTransforms_[i]->transform.translate = { 21.4f, 19.5f, 38.3f };

		std::string name = "explosion" + std::to_string(i);
		ParticleManager::GetInstance()->CreateParticleGroup(name, "Resources/circle.png", ParticleManager::ParticleType::Normal);
		explosionEmitter_[i] = std::make_unique<ParticleEmitter>();
		explosionEmitter_[i]->Initialize(name);
		explosionEmitter_[i]->SetParticleCount(500);
		explosionEmitter_[i]->SetFrequency(0.1f);
		explosionEmitter_[i]->SetLifeTimeRange({ 3.5f, 8.0f });
		explosionEmitter_[i]->SetVelocityRange(ParticleManager::Vec3Range({ -100.0f,-100.0f,-100.0f }, { 100.0f,100.0f,100.0f }));
		explosionEmitter_[i]->SetStartScaleRange(ParticleManager::Vec3Range({ 3.0f,3.0f,3.0f, }, { 5.0f,5.0f,5.0f }));
		explosionEmitter_[i]->SetFinishScaleRange(ParticleManager::Vec3Range({ 3.0f,3.0f,3.0f, }, { 5.0f,5.0f,5.0f }));
	}
	spawnTransforms_[0]->transform.translate = { 31.1f, 13.9f, 38.4f };
	spawnTransforms_[1]->transform.translate = { 15.2f, 3.5f, 47.5f };
	spawnTransforms_[2]->transform.translate = { -14.3f, -2.2f, 44.8f };
	spawnTransforms_[3]->transform.translate = { -30.4f, 11.3f, 43.6f };

	
	ParticleManager::GetInstance()->CreateParticleGroup("missileSmoke", "Resources/circle.png", ParticleManager::ParticleType::Normal);
	smokeEmitter_ = std::make_unique<ParticleEmitter>();
	smokeEmitter_->Initialize("missileSmoke");
	smokeEmitter_->SetParticleCount(4);
	smokeEmitter_->SetFrequency(0.1f);
	smokeEmitter_->SetLifeTimeRange({ 0.3f, 0.5f });
	smokeEmitter_->SetStartColorRange(
		ParticleManager::ColorRange(
			{ 0.3f,0.3f,0.3f,0.3f },
			{ 0.5f,0.5f,0.5f,0.5f }
		)
	);
	smokeEmitter_->SetFinishColorRange(
		ParticleManager::ColorRange(
			{ 0.6f,0.6f,0.6f,0.0f },
			{ 0.8f,0.8f,0.8f,0.3f }
		)
	);
	smokeEmitter_->SetVelocityRange(
		ParticleManager::Vec3Range(
			{ 0.0f,0.0f,-1.0f },
			{ 0.0f,6.0f,1.0f }
		)
	);
	smokeEmitter_->SetStartScaleRange(ParticleManager::Vec3Range({ 0.5f,0.5f,0.5f }, { 1.0f,1.0f,1.0f }));
	smokeEmitter_->SetFinishScaleRange(ParticleManager::Vec3Range({ 0.5f,0.5f,0.5f }, { 1.0f,1.0f,1.0f }));

	mwdoorTransform_->SetParent(mwbodyTransform_.get());
	mwdishTransform_->SetParent(mwbodyTransform_.get());
	clearTransform_->SetParent(mwdishTransform_.get());
	mvSceneTransform_->SetParent(mwdoorTransform_.get());
	



}


void GameClear::Finalize() {
}

void GameClear::Update() {

	BaseScene::Update();
	directionalLight->Update();
	pointLight->Update();
	mvPointLight->Update();
	spotLight->Update();
	mvSpotLight->Update();

	switch (phase_)
	{
	case Phase::kFadeIn:

		if (fade_->IsFinished())
		{
			Input::GetInstance()->SetIsReception(true);
			phase_ = Phase::kMain;
		}

		break;
	case Phase::kMain:

		if (isMoving_) {
			UpdateMoveMwbody();
		}

		if (!isStartClose_) {
			UpdateMoveClear();
		} else {
			UpdateCloseDoor(); 
		}
		ParticleManager::GetInstance()->Update();

		UpdateMissileFlight();

		buttonA_->Update();

		mwbodyTransform_->UpdateMatrix();// 行列更新
		mwbody_->SetLocalMatrix(MakeIdentity4x4());// ローカル行列を単位行列に
		mwbody_->Update();// 更新

		mwdoorTransform_->UpdateMatrix();// 行列更新
		mwdoor_->SetLocalMatrix(MakeIdentity4x4());// ローカル行列を単位行列に
		mwdoor_->Update();// 更新

		mwdishTransform_->UpdateMatrix();// 行列更新
		mwdish_->SetLocalMatrix(MakeIdentity4x4());// ローカル行列を単位行列に
		mwdish_->Update();// 更新

		clearTransform_->UpdateMatrix();// 行列更新
		clearModel_->SetLocalMatrix(MakeIdentity4x4());// ローカル行列を単位行列に
		clearModel_->Update();// 更新
		titleModel_->SetLocalMatrix(MakeIdentity4x4());// ローカル行列を単位行列に
		titleModel_->Update();// 更新
		for (int i = 0; i < 4; i++) {
			missileTransforms_[i]->UpdateMatrix();// 行列更新
			missileModels_[i]->SetLocalMatrix(MakeIdentity4x4());// ローカル行列を単位行列に
			missileModels_[i]->Update();// 更新

			spawnTransforms_[i]->UpdateMatrix();// 行列更新
			spawnModels_[i]->SetLocalMatrix(MakeIdentity4x4());// ローカル行列を単位行列に
			spawnModels_[i]->Update();// 更新
		}
		//spawnTransform_->UpdateMatrix();// 行列更新
		//spawnModel_->SetLocalMatrix(MakeIdentity4x4());// ローカル行列を単位行列に
		//spawnModel_->Update();// 更新

		//missileTransform_->UpdateMatrix();// 行列更新
		//missileModel_->SetLocalMatrix(MakeIdentity4x4());// ローカル行列を単位行列に
		//missileModel_->Update();// 更新

		mvSceneTransform_->UpdateMatrix();// 行列更新
		mvSceneModel_->SetLocalMatrix(MakeIdentity4x4());// ローカル行列を単位行列に
		mvSceneModel_->Update();// 更新


		

#ifdef _DEBUG
		ImGui::Begin("Model Transform Editor");

		
		if (ImGui::CollapsingHeader("MW Body")) {
			ImGui::DragFloat3("Position##MWBody", &mwbodyTransform_->transform.translate.x, 0.1f);
			ImGui::DragFloat3("Rotation##MWBody", &mwbodyTransform_->transform.rotate.x, 0.1f);
			ImGui::DragFloat3("Scale##MWBody", &mwbodyTransform_->transform.scale.x, 0.01f);
		}


		if (ImGui::CollapsingHeader("MW Door")) {
			ImGui::DragFloat3("Position##MWDoor", &mwdoorTransform_->transform.translate.x, 0.1f);
			ImGui::DragFloat3("Rotation##MWDoor", &mwdoorTransform_->transform.rotate.x, 0.1f);
			ImGui::DragFloat3("Scale##MWDoor", &mwdoorTransform_->transform.scale.x, 0.01f);
		}

		if (ImGui::CollapsingHeader("Dish")) {
			ImGui::DragFloat3("Position##Dish", &mwdishTransform_->transform.translate.x, 0.1f);
			ImGui::DragFloat3("Rotation##Dish", &mwdishTransform_->transform.rotate.x, 0.1f);
			ImGui::DragFloat3("Scale##Dish", &mwdishTransform_->transform.scale.x, 0.01f);
		}

		if (ImGui::CollapsingHeader("Clear")) {
			ImGui::DragFloat3("Position##Clear", &clearTransform_->transform.translate.x, 0.1f);
			ImGui::DragFloat3("Rotation##Clear", &clearTransform_->transform.rotate.x, 0.1f);
			ImGui::DragFloat3("Scale##Clear", &clearTransform_->transform.scale.x, 0.01f);
		}

		if (ImGui::CollapsingHeader("Spawn1")) {
			ImGui::DragFloat3("Position##Spawn", &spawnTransforms_[0]->transform.translate.x, 0.1f);
			ImGui::DragFloat3("Rotation##Spawn", &spawnTransforms_[0]->transform.rotate.x, 0.1f);
			ImGui::DragFloat3("Scale##Spawn", &spawnTransforms_[0]->transform.scale.x, 0.01f);
		}
		if (ImGui::CollapsingHeader("Spawn2")) {
			ImGui::DragFloat3("Position##Spawn", &spawnTransforms_[1]->transform.translate.x, 0.1f);
			ImGui::DragFloat3("Rotation##Spawn", &spawnTransforms_[1]->transform.rotate.x, 0.1f);
			ImGui::DragFloat3("Scale##Spawn", &spawnTransforms_[1]->transform.scale.x, 0.01f);
		}
		if (ImGui::CollapsingHeader("Spawn3")) {
			ImGui::DragFloat3("Position##Spawn", &spawnTransforms_[2]->transform.translate.x, 0.1f);
			ImGui::DragFloat3("Rotation##Spawn", &spawnTransforms_[2]->transform.rotate.x, 0.1f);
			ImGui::DragFloat3("Scale##Spawn", &spawnTransforms_[2]->transform.scale.x, 0.01f);
		}
		if (ImGui::CollapsingHeader("Spawn4")) {
			ImGui::DragFloat3("Position##Spawn", &spawnTransforms_[3]->transform.translate.x, 0.1f);
			ImGui::DragFloat3("Rotation##Spawn", &spawnTransforms_[3]->transform.rotate.x, 0.1f);
			ImGui::DragFloat3("Scale##Spawn", &spawnTransforms_[3]->transform.scale.x, 0.01f);
		}

		if (ImGui::CollapsingHeader("missile1")) {
			ImGui::DragFloat3("Position##missile", &missileTransforms_[0]->transform.translate.x, 0.1f);
			ImGui::DragFloat3("Rotation##missile", &missileTransforms_[0]->transform.rotate.x, 0.1f);
			ImGui::DragFloat3("Scale##missile", &missileTransforms_[0]->transform.scale.x, 0.01f);
		}
		if (ImGui::CollapsingHeader("missile2")) {
			ImGui::DragFloat3("Position##missile", &missileTransforms_[1]->transform.translate.x, 0.1f);
			ImGui::DragFloat3("Rotation##missile", &missileTransforms_[1]->transform.rotate.x, 0.1f);
			ImGui::DragFloat3("Scale##missile", &missileTransforms_[1]->transform.scale.x, 0.01f);
		}
		if (ImGui::CollapsingHeader("missile3")) {
			ImGui::DragFloat3("Position##missile", &missileTransforms_[2]->transform.translate.x, 0.1f);
			ImGui::DragFloat3("Rotation##missile", &missileTransforms_[2]->transform.rotate.x, 0.1f);
			ImGui::DragFloat3("Scale##missile", &missileTransforms_[2]->transform.scale.x, 0.01f);
		}
		if (ImGui::CollapsingHeader("missile4")) {
			ImGui::DragFloat3("Position##missile", &missileTransforms_[3]->transform.translate.x, 0.1f);
			ImGui::DragFloat3("Rotation##missile", &missileTransforms_[3]->transform.rotate.x, 0.1f);
			ImGui::DragFloat3("Scale##missile", &missileTransforms_[3]->transform.scale.x, 0.01f);
		}

		if (ImGui::CollapsingHeader("MW Scene")) {
			ImGui::DragFloat3("Position##MWScenen", &mvSceneTransform_->transform.translate.x, 0.1f);
			ImGui::DragFloat3("Rotation##MWScenen", &mvSceneTransform_->transform.rotate.x, 0.1f);
			ImGui::DragFloat3("Scale##MWScenen", &mvSceneTransform_->transform.scale.x, 0.01f);
		}

		if (ImGui::CollapsingHeader("mvSpotLight Light")) {
			ImGui::DragFloat3("mvSpotLight Position", &mvSpotLight->position_.x, 1.0f);
			ImGui::DragFloat3("mvSpotLight Direction", &mvSpotLight->direction_.x, 0.05f);
			ImGui::DragFloat("Intensity", &mvSpotLight->intensity_, 0.1f, 0.0f, 100.0f);
			ImGui::DragFloat("Decay", &mvSpotLight->decay_, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Distance", &mvSpotLight->distance_, 1.0f, 0.0f, 10000.0f);
			ImGui::DragFloat("CosAngle", &mvSpotLight->cosAngle_, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("FalloffStart", &mvSpotLight->cosFalloffStart_, 0.01f, 0.0f, 1.0f);
			ImGui::ColorEdit3("Color", &mvSpotLight->color_.x);
		}
		if (ImGui::CollapsingHeader("mvPointLight Light")) {
			ImGui::DragFloat3("mvPointLight Position", &mvPointLight->position_.x, 1.0f);
			ImGui::DragFloat("Intensity", &mvPointLight->intensity_, 0.1f, 0.0f, 100.0f);
			ImGui::DragFloat("Decay", &mvPointLight->decay_, 0.01f, 0.0f, 1.0f);
			ImGui::ColorEdit3("Color", &mvPointLight->color_.x);
		}
		if (ImGui::CollapsingHeader("Button A")) {
			Vector2 buttonPos = buttonA_->GetPosition();
			Vector2 buttonSize = buttonA_->GetSize();

			ImGui::DragFloat2("Position##ButtonA", &buttonPos.x, 1.0f);
			ImGui::DragFloat2("Size##ButtonA", &buttonSize.x, 1.0f);

			buttonA_->SetPosition(buttonPos);
			buttonA_->SetSize(buttonSize);
		}

		

		ImGui::End();
#endif
		if (!isStartClose_ && showButtonA_ &&
			(Input::GetInstance()->Triggerkey(DIK_RETURN) || Input::GetInstance()->TriggerGamePadButton(Input::GamePadButton::A))) {
			mwSE_->SoundPlay("Resources/se/select.mp3", 0);
			mwSE_->SetVolume(0.5f);
			isStartClose_ = true;
		}

		break;
	case Phase::kFadeOut:

		if (fade_->IsFinished())
		{
			SceneManager::GetInstance()->ChangeScene("TITLE");

			return;
		}

		break;
	
	case Phase::kPlay:
		break;
	case Phase::kPose:
		break;
	}

	

	//title
	clear_->Update();

	BaseScene::Update();
}

void GameClear::Draw() {
	DrawBackgroundSprite();

	clear_->Draw();


	DrawObject();

	mwbody_->Draw(*mwbodyTransform_.get(), cameraManager_->GetActiveCamera()->GetViewProjection(),
		*directionalLight, *mvPointLight, *spotLight);
	mwdoor_->Draw(*mwdoorTransform_.get(), cameraManager_->GetActiveCamera()->GetViewProjection(),
		*directionalLight, *pointLight, *spotLight);
	mwdish_->Draw(*mwdishTransform_.get(), cameraManager_->GetActiveCamera()->GetViewProjection(),
		*directionalLight, *mvPointLight, *spotLight);
	if (!isText_) {
	clearModel_->Draw(*clearTransform_.get(), cameraManager_->GetActiveCamera()->GetViewProjection(),
		*directionalLight, *mvPointLight, *spotLight);
	} else {
	titleModel_->Draw(*clearTransform_.get(), cameraManager_->GetActiveCamera()->GetViewProjection(),
		*directionalLight, *mvPointLight, *spotLight);
	}
	for (int i = 0; i < 4; ++i) {
		if (isVisible_[i]) {
			spawnModels_[i]->Draw(*spawnTransforms_[i], cameraManager_->GetActiveCamera()->GetViewProjection(),
				*directionalLight, *mvPointLight, *spotLight);
			missileModels_[i]->Draw(*missileTransforms_[i], cameraManager_->GetActiveCamera()->GetViewProjection(),
				*directionalLight, *mvPointLight, *spotLight);
		}
	}
	mvSceneModel_->Draw(*mvSceneTransform_.get(), cameraManager_->GetActiveCamera()->GetViewProjection(),
		*directionalLight, *pointLight, *mvSpotLight);

	DrawForegroundSprite();

	if (showButtonA_) {
		buttonA_->Draw();
	}

	fade_->Draw();

	ParticleManager::GetInstance()->Draw("Resources/circle.png");
}

void GameClear::UpdateMoveMwbody() {
	if (!isMoving_) return;

	moveTimer_ += 1.0f / 60.0f;
	float t = moveTimer_ / moveDuration_;
	if (t >= 1.0f) {
		t = 1.0f;
		isMoving_ = false;
	}

	// LERP 
	Vector3 startPos = { 0.0f, 1.0f, 1.0f };
	Vector3 endPos = { -0.1f, -0.9f, -11.9f };
	Vector3 newPos = {
		startPos.x + (endPos.x - startPos.x) * t,
		startPos.y + (endPos.y - startPos.y) * t,
		startPos.z + (endPos.z - startPos.z) * t
	};
	mwbodyTransform_->transform.translate = newPos;

	Vector3 startPointPos = { -0.3f,2.0f,0.8f };
	Vector3 endPointPos = { -0.1f, -0.0f, -11.0f };
	Vector3 newPointPos = {
		startPointPos.x + (endPointPos.x - startPointPos.x) * t,
		startPointPos.y + (endPointPos.y - startPointPos.y) * t,
		startPointPos.z + (endPointPos.z - startPointPos.z) * t
	};
	mvPointLight->position_ = newPointPos;

	
}

void GameClear::UpdateMissileFlight() {

	for (int i = 0; i < 4; ++i) {
		if (!isMissileFlying_[i]) continue;

		
		missileTimers_[i] += 1.0f / 60.0f;
		float t = std::min(missileTimers_[i] / missileDurations_[i], 1.0f);

		
		Vector3 startPos = missileStartPositions_[i];
		Vector3 endPos = missileEndPositions_[i];
		Vector3 newPos = Lerp(endPos, startPos, t);
		missileTransforms_[i]->transform.translate = newPos;

		
		Vector3 startRot = { 0.3f, 1.6f, -1.0f };
		Vector3 endRotList[4] = {
			{ 0.4f, 1.6f, -1.0f },
			{ 0.5f, 1.6f, -1.0f },
			{ 0.6f, 1.6f, -1.0f },
			{ 0.3f, 1.6f, -1.0f }
		};
		Vector3 newRot = Lerp(endRotList[i], startRot, t);
		missileTransforms_[i]->transform.rotate = newRot;

		
		if (t >= 1.0f) {
			isMissileFlying_[i] = false;
			isVisible_[i] = false;
			explosionEmitter_[i]->SetPosition(endPos);
			explosionEmitter_[i]->Emit();
			explosionSE_[i]->SoundPlay("Resources/se/explosion.mp3", 0);
			explosionSE_[i]->SetVolume(0.5f);
		}
	}

}

void GameClear::UpdateMoveClear() {
	if (isClear_) {
	mwdishTransform_->transform.rotate.y += 0.05f;
	if (mwdishTransform_->transform.rotate.y >= 12.45) {
		mwdishTransform_->transform.rotate.y = 12.45f;
		mwTinSE_->SoundPlay("Resources/se/tin.mp3", 0);
		mwTinSE_->SetVolume(1.3f);
		isClear_ = false;
		}
	}
	if (!isClear_) {
		if (isDoor_) {
		doorTimer_ += 1.0f / 60.0f;
		float t = doorTimer_ / doorDuration_;

		Vector3 startRot = { 0.0f, 0.0f, 0.0f };
		Vector3 endRot = { 0.0f, 3.0f, 0.0f };
		Vector3 newRot = {
			startRot.x + (endRot.x - startRot.x) * t,
			startRot.y + (endRot.y - startRot.y) * t,
			startRot.z + (endRot.z - startRot.z) * t
			};
		mwdoorTransform_->transform.rotate = newRot;
		
		/*smokeEmitter_->Emit();*/
		
		}
		if (doorTimer_ >= doorDuration_) {
			isDoor_ = false;
		}
		if (doorTimer_ >= 1.0f) {
			
			smokeEmitter_->SetPosition(mwdishTransform_->GetWorldPosition());
			smokeEmitter_->Update();
			const float amplitude = 0.08f;
			const float speed = 2.0f;
			static float clearTime = 0.0f;
			clearTime += 1.0f / 60.0f;

			clearTransform_->transform.translate.y = 0.4f + amplitude * sinf(clearTime * speed);

			buttonABlinkTimer_ += 1.0f / 60.0f;
			const float blinkInterval = 0.5f; 

			if (fmod(buttonABlinkTimer_, blinkInterval * 2.0f) < blinkInterval) {
				showButtonA_ = true;
			} else {
				showButtonA_ = false;
			}

		}

	}
}

void GameClear::UpdateCloseDoor() {
	if (!isClose_) {
	closeTimer_ += 1.0f / 60.0f;
	float t = closeTimer_ / closeDuration_;

	Vector3 startRot = { 0.0f, 3.0f, 0.0f };
	Vector3 endRot = { 0.0f, 0.0f, 0.0f };
	Vector3 newRot = {
		startRot.x + (endRot.x - startRot.x) * t,
		startRot.y + (endRot.y - startRot.y) * t,
		startRot.z + (endRot.z - startRot.z) * t
	};
	mwdoorTransform_->transform.rotate = newRot;
	isText_ = true;
	clearTransform_->transform.translate = { 0.5f, 0.4f , 0.2f };
	showButtonA_ = false;
	}

	if (closeTimer_ >= closeDuration_) {
		isClose_ = true;
	}
	if (isClose_) {

		mwdishTransform_->transform.rotate.y += 0.05f;
		mwbodyTransform_->transform.translate.z += 0.05f;
	}
	
	if (mwbodyTransform_->transform.translate.z >= -0.6) {
		isMove_ = true;
	}
	if (isMove_) {
		
		fade_->Start(Fade::Status::FadeOut, fadeTime_);
		phase_ = Phase::kFadeOut;
	}
}

