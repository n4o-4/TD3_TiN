#include "GameOver.h"

void GameOver::Initialize() {
	BaseScene::Initialize();
	//over
	TextureManager::GetInstance()->LoadTexture("Resources/scene/game_over.png");
	over_ = std::make_unique<Sprite>();
	over_->Initialize(SpriteCommon::GetInstance(), "Resources/scene/game_over.png");
	over_->SetTexSize({ 1280.0f,720.0f });
	over_->SetSize({ 1280.0f,720.0f });
	over_->SetPosition({ 0.0f,0.0f });

	//Button A
	TextureManager::GetInstance()->LoadTexture("Resources/scene/A1.png");
	buttonA_ = std::make_unique<Sprite>();
	buttonA_->Initialize(SpriteCommon::GetInstance(), "Resources/scene/A1.png");
	buttonA_->SetTexSize({ 220.0f,220.0f });
	buttonA_->SetSize({ 168.0f,172.0f });
	buttonA_->SetPosition({ 1100.0f,254.0f });


	AudioManager::GetInstance()->Initialize();
	AudioManager::GetInstance()->SoundLoadFile("Resources/bgm/gameOVER.mp3");
	AudioManager::GetInstance()->SoundLoadFile("Resources/se/tin.mp3");
	AudioManager::GetInstance()->SoundLoadFile("Resources/se/select.mp3");
	overBGM_ = std::make_unique<Audio>();
	overBGM_->Initialize();
	overBGM_->SoundPlay("Resources/bgm/gameOVER.mp3", 9999);
	overBGM_->SetVolume(0.6f);

	mwTinSE_ = std::make_unique<Audio>();
	mwTinSE_->Initialize();

	mwSE_ = std::make_unique<Audio>();
	mwSE_->Initialize();

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
	mvPointLight->position_ = { 0.7f,-3.6f,-0.2f };
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

	//model
	ModelManager::GetInstance()->LoadModel("scene/MWbody.obj");
	ModelManager::GetInstance()->LoadModel("scene/MWdoor.obj");
	ModelManager::GetInstance()->LoadModel("scene/dish.obj");
	ModelManager::GetInstance()->LoadModel("scene/MWScenen.obj");
	ModelManager::GetInstance()->LoadModel("scene/Over.obj");
	ModelManager::GetInstance()->LoadModel("scene/Title.obj");
	ModelManager::GetInstance()->LoadModel("enemy/RC/RC.obj");
	ModelManager::GetInstance()->LoadModel("enemy/Refrigerator/Refrigerator.obj");
	ModelManager::GetInstance()->LoadModel("enemy/WH/WH.obj");
	ModelManager::GetInstance()->LoadModel("enemy/Iron/Iron.obj");
	ModelManager::GetInstance()->LoadModel("enemy/cp/cp.obj");

	for (int i = 0; i < 4; ++i) {
		mvModels_[i] = std::make_unique<Object3d>();
		mvModels_[i]->Initialize(Object3dCommon::GetInstance());

		mvTransforms_[i] = std::make_unique<WorldTransform>();
		mvTransforms_[i]->Initialize();
		mvTransforms_[i]->transform.translate = { 0.0f, -4.1f , 0.3f };
		mvTransforms_[i]->transform.rotate = { 0.0f, 3.14f, 0.0f };
	}
	
	mvModels_[0]->SetModel("scene/MWbody.obj");
	mvModels_[1]->SetModel("scene/MWdoor.obj");
	mvModels_[2]->SetModel("scene/dish.obj");
	mvModels_[3]->SetModel("scene/MWScenen.obj");

	
	mvTransforms_[1]->transform.translate = { 1.36f, 0.0f , 0.95f };
	mvTransforms_[1]->transform.rotate = { 0.0f, 0.0f, 0.0f };
	mvTransforms_[2]->transform.translate = { 0.33f, 0.3f , 0.3f };
	mvTransforms_[2]->transform.scale = { 0.7f, 0.7f , 0.7f };
	mvTransforms_[3]->transform.translate = { -2.15f, 0.0f , 0.1f };
	mvTransforms_[3]->transform.rotate = { 0.0f, 3.14f, 0.0f };

	for (int i = 0; i < 5; ++i) {
		enemyModels_[i] = std::make_unique<Object3d>();
		enemyModels_[i]->Initialize(Object3dCommon::GetInstance());

		enemyTransforms_[i] = std::make_unique<WorldTransform>();
		enemyTransforms_[i]->Initialize();
		enemyTransforms_[i]->transform.rotate = { 0.0f, 3.14f , 0.0f };
	}

	enemyModels_[0]->SetModel("enemy/RC/RC.obj");
	enemyModels_[1]->SetModel("enemy/Refrigerator/Refrigerator.obj");
	enemyModels_[2]->SetModel("enemy/WH/WH.obj");
	enemyModels_[3]->SetModel("enemy/cp/cp.obj");
	enemyModels_[4]->SetModel("enemy/Iron/Iron.obj");

	enemyTransforms_[0]->transform.translate = { -9.7f, -1.0f ,-1.2f };
	enemyTransforms_[0]->transform.rotate = { 0.1f, -2.87f , -0.3f };
	enemyTransforms_[0]->transform.scale = { 1.24f, 1.2f , 1.16f };

	enemyTransforms_[1]->transform.translate = { -5.5f, 0.2f , -0.2f };
	enemyTransforms_[1]->transform.rotate = { -0.5f, -0.8f , 0.1f };
	enemyTransforms_[1]->transform.scale = { 1.1f, 1.07f , 1.07f };

	enemyTransforms_[2]->transform.translate = { 0.0f, 0.8f , 5.1f };
	enemyTransforms_[2]->transform.rotate = { 0.4f, 3.14f , 0.0f };
	enemyTransforms_[2]->transform.scale = { 1.19f, 1.29f , 1.0f };

	enemyTransforms_[3]->transform.translate = { 6.4f, 0.4f , 3.2f };
	enemyTransforms_[3]->transform.rotate = { 0.4f, 3.84f , 0.0f };
	enemyTransforms_[3]->transform.scale = { 1.2f, 1.13f , 1.15f };

	enemyTransforms_[4]->transform.translate = { 9.3f, -1.1f , -0.1f };
	enemyTransforms_[4]->transform.rotate = { -0.1f, 6.14f , 0.2f };
	enemyTransforms_[4]->transform.scale = { 1.62f, 1.82f , 1.07f };

	


	overModel_ = std::make_unique<Object3d>();
	overModel_->Initialize(Object3dCommon::GetInstance());
	overModel_->SetModel("scene/Over.obj");

	titleModel_ = std::make_unique<Object3d>();
	titleModel_->Initialize(Object3dCommon::GetInstance());
	titleModel_->SetModel("scene/Title.obj");

	textTransform_ = std::make_unique<WorldTransform>();
	textTransform_->Initialize();
	textTransform_->transform.translate = { -0.8f, 0.2f , -0.2f };
	textTransform_->transform.rotate = { 0.0f, 3.14f , 0.0f };
	textTransform_->transform.scale = { 0.55f, 0.55f , 1.0f };


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



	mvTransforms_[1]->SetParent(mvTransforms_[0].get());
	mvTransforms_[2]->SetParent(mvTransforms_[0].get());
	textTransform_->SetParent(mvTransforms_[2].get());
	mvTransforms_[3]->SetParent(mvTransforms_[1].get());

	/*cameraViewProjection_ = std::make_unique<ViewProjection>();
	cameraViewProjection_->Initialize();
	cameraViewProjection_->transform.translate = { 0.0f, 0.0f, -15.0f };*/

}

void GameOver::Finalize() {
	overBGM_->SoundStop("Resources/bgm/gameOVER.mp3");
}

void GameOver::Update() {

	BaseScene::Update();
	directionalLight->Update();
	pointLight->Update();
	mvPointLight->Update();
	spotLight->Update();
	

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

#ifdef _DEBUG
		ImGui::Begin("Model Transform Editor");

		if (ImGui::CollapsingHeader("MW model")) {
			for (int i = 0; i < 4; ++i) {
				std::string label = "Model " + std::to_string(i);
				ImGui::Text("%s", label.c_str());
				ImGui::DragFloat3(("Pos##" + label).c_str(), &mvTransforms_[i]->transform.translate.x, 0.1f);
				ImGui::DragFloat3(("Rot##" + label).c_str(), &mvTransforms_[i]->transform.rotate.x, 0.1f);
				ImGui::DragFloat3(("Scl##" + label).c_str(), &mvTransforms_[i]->transform.scale.x, 0.01f);
				ImGui::Separator();
			}
		}

		if (ImGui::CollapsingHeader("Enemy model")) {
			for (int i = 0; i < 5; ++i) {
				std::string label = "Model " + std::to_string(i);
				ImGui::Text("%s", label.c_str());
				ImGui::DragFloat3(("Pos##" + label).c_str(), &enemyTransforms_[i]->transform.translate.x, 0.1f);
				ImGui::DragFloat3(("Rot##" + label).c_str(), &enemyTransforms_[i]->transform.rotate.x, 0.1f);
				ImGui::DragFloat3(("Scl##" + label).c_str(), &enemyTransforms_[i]->transform.scale.x, 0.01f);
				ImGui::Separator();
			}
		}
		if (ImGui::CollapsingHeader("Clear")) {
			ImGui::DragFloat3("Position##Clear", &textTransform_->transform.translate.x, 0.1f);
			ImGui::DragFloat3("Rotation##Clear", &textTransform_->transform.rotate.x, 0.1f);
			ImGui::DragFloat3("Scale##Clear", &textTransform_->transform.scale.x, 0.01f);
		}
		
		if (ImGui::CollapsingHeader("Camera Transform")) {
			Vector3& camPos = cameraManager_->GetActiveCamera()->GetViewProjection().transform.translate;
			ImGui::DragFloat3("Camera Position", &camPos.x, 0.1f);
		}

		if (ImGui::CollapsingHeader("mvPointLight Light")) {
			ImGui::DragFloat3("mvPointLight Position", &mvPointLight->position_.x, 1.0f);
			ImGui::DragFloat("Intensity", &mvPointLight->intensity_, 0.1f, 0.0f, 100.0f);
			ImGui::DragFloat("Decay", &mvPointLight->decay_, 0.01f, 0.0f, 1.0f);
			ImGui::ColorEdit3("Color", &mvPointLight->color_.x);
		}

		/*if (ImGui::CollapsingHeader("smokeEmitter_")) {
			ImGui::DragFloat3("Position##MWBody", &smokeEmitter_->,1.f);
			
		}*/

		ImGui::End();
#endif

		//cameraViewProjection_->Update();

		if (!isMoving_) {
			cameraMove();
		}
		if (!isStartClose_) {
			OpenMwdoor();
		} else {
			CloseMwdoor();
		}

		buttonA_->Update();

		ParticleManager::GetInstance()->Update();

		for (int i = 0; i < 4; i++) {
			mvTransforms_[i]->UpdateMatrix();// 行列更新
			mvModels_[i]->SetLocalMatrix(MakeIdentity4x4());// ローカル行列を単位行列に
			mvModels_[i]->Update();// 更新
		}

		for (int i = 0; i < 5; i++) {
			enemyTransforms_[i]->UpdateMatrix();// 行列更新
			enemyModels_[i]->SetLocalMatrix(MakeIdentity4x4());// ローカル行列を単位行列に
			enemyModels_[i]->Update();// 更新

		}
		textTransform_->UpdateMatrix();// 行列更新
		overModel_->SetLocalMatrix(MakeIdentity4x4());// ローカル行列を単位行列に
		overModel_->Update();// 更新
		titleModel_->SetLocalMatrix(MakeIdentity4x4());// ローカル行列を単位行列に
		titleModel_->Update();// 更新

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
	over_->Update();

	BaseScene::Update();
}

void GameOver::Draw() {


	DrawBackgroundSprite();

	over_->Draw();

	DrawObject();

	if (!isText_) {
		overModel_->Draw(*textTransform_.get(), cameraManager_->GetActiveCamera()->GetViewProjection(),
			*directionalLight, *mvPointLight, *spotLight);
	} else {
		titleModel_->Draw(*textTransform_.get(), cameraManager_->GetActiveCamera()->GetViewProjection(),
			*directionalLight, *mvPointLight, *spotLight);
	}
	for (int i = 0; i < 4; ++i) {
		mvModels_[i]->Draw(*mvTransforms_[i], cameraManager_->GetActiveCamera()->GetViewProjection(),
				*directionalLight, *mvPointLight, *spotLight);
	}
	for (int i = 0; i < 5; ++i) {
		enemyModels_[i]->Draw(*enemyTransforms_[i], cameraManager_->GetActiveCamera()->GetViewProjection(),
			*directionalLight, *pointLight, *spotLight);
	}


	DrawForegroundSprite();
	if (showButtonA_) {
		buttonA_->Draw();
	}
	fade_->Draw();
	ParticleManager::GetInstance()->Draw("Resources/circle.png");
}

void GameOver::cameraMove() {

	cameraTimer_ += 1.0f / 60.0f;
	float t = cameraTimer_ / cameraDuration_;

	if (!iscameraMove_ && cameraTimer_ >= 0.3f) {
		iscameraMove_ = true;
	}
	if (iscameraMove_) {
		if (t >= 1.0f) {
			t = 1.0f;
			iscameraMove_ = false;
			isMoving_ = true;
		}
		Vector3 startPos = { 0.0f, 0.0f, -15.0f };
		Vector3 endPos = { 0.0f, -3.2f, -3.1f };
		Vector3 newPos = {
			startPos.x + (endPos.x - startPos.x) * t,
			startPos.y + (endPos.y - startPos.y) * t,
			startPos.z + (endPos.z - startPos.z) * t
		};
		cameraManager_->GetActiveCamera()->GetViewProjection().transform.translate = newPos;
	}


}

void GameOver::OpenMwdoor() {
	if (isOver_) {
		mvTransforms_[2]->transform.rotate.y += 0.05f;
		if (mvTransforms_[2]->transform.rotate.y >= 15.64f) {
			mvTransforms_[2]->transform.rotate.y = 15.64f;
			mwTinSE_->SoundPlay("Resources/se/tin.mp3", 0);
			mwTinSE_->SetVolume(1.3f);
			isOver_ = false;
		}
	}
	if (!isOver_) {
		if (isOpenDoor_) {
			openDoorTimer_ += 1.0f / 60.0f;
			float t = openDoorTimer_ / openDoorDuration_;

			Vector3 startRot = { 0.0f, 0.0f, 0.0f };
			Vector3 endRot = { 0.0f, 3.0f, 0.0f };
			Vector3 newRot = {
				startRot.x + (endRot.x - startRot.x) * t,
				startRot.y + (endRot.y - startRot.y) * t,
				startRot.z + (endRot.z - startRot.z) * t
			};
			mvTransforms_[1]->transform.rotate = newRot;

		}
		if (openDoorTimer_ >= openDoorDuration_) {
			isOpenDoor_ = false;
		}
		if (openDoorTimer_ >= 1.0f) {

			smokeEmitter_->SetPosition(mvTransforms_[2]->GetWorldPosition());
			smokeEmitter_->Update();
			const float amplitude = 0.08f;
			const float speed = 2.0f;
			static float clearTime = 0.0f;
			clearTime += 1.0f / 60.0f;

			textTransform_->transform.translate.y = 0.4f + amplitude * sinf(clearTime * speed);

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

void GameOver::CloseMwdoor() {
	if (!isCloseDoor_) {
		closeDoorTimer_ += 1.0f / 60.0f;
		float t = std::clamp(closeDoorTimer_ / closeDoorDuration_, 0.0f, 1.0f);

		Vector3 startRot = { 0.0f, 3.0f, 0.0f };
		Vector3 endRot = { 0.0f, 0.0f, 0.0f };
		Vector3 newRot = Lerp(endRot,startRot, t);
		mvTransforms_[1]->transform.rotate = newRot;

		isText_ = true;
		showButtonA_ = false;
		textTransform_->transform.translate = { -0.7f, 0.2f , -0.2f };

		if (t >= 1.0f) {
			isCloseDoor_ = true;
		}
	}
	if (isCloseDoor_) {

		mvTransforms_[2]->transform.rotate.y += 0.05f;
		cameraManager_->GetActiveCamera()->GetViewProjection().transform.translate.z -= 0.1f;
	}

	if (cameraManager_->GetActiveCamera()->GetViewProjection().transform.translate.z <= -10.0f) {
		isMove_ = true;
	}
	if (isMove_) {
		fade_->Start(Fade::Status::FadeOut, fadeTime_);
		phase_ = Phase::kFadeOut;
	}


}
