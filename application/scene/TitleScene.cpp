#include "TitleScene.h"


void TitleScene::Initialize()
{
	BaseScene::Initialize();

	TextureManager::GetInstance()->LoadTexture("Resources/monsterBall.png");
	TextureManager::GetInstance()->LoadTexture("Resources/uvChecker.png");
	TextureManager::GetInstance()->LoadTexture("Resources/fruit_suika_red.png");


	ModelManager::GetInstance()->LoadModel("axis.obj");

	//audio = std::make_unique<Audio>();
	//audio->Initialize();
	//audio->SoundPlay("Resources/Spinning_World.mp3",999);

	// ライト
	// 指向性
	directionalLight = std::make_unique<DirectionalLight>();
	directionalLight->Initilaize();
	directionalLight->intensity_ = 0.0f;
	// 点光源
	pointLight = std::make_unique<PointLight>();
	pointLight->Initilize();
	pointLight->intensity_ = 0.0f;
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

	mvPointLight = std::make_unique<PointLight>();
	mvPointLight->Initilize();
	mvPointLight->position_ = { -0.3f,0.0f,-10.5f };
	mvPointLight->intensity_ = 2.7f;
	mvPointLight->decay_ = 1.0f;
	mvPointLight->color_ = { 0.925f, 0.498f, 0.118f, 1.0f };

	//start
	TextureManager::GetInstance()->LoadTexture("Resources/scene/A1.png");
	start1_ = std::make_unique<Sprite>();
	start1_->Initialize(SpriteCommon::GetInstance(), "Resources/scene/A1.png");
	start1_->SetTexSize({ 220.0f,220.0f });
	start1_->SetSize({ 220.0f,220.0f });
	start1_->SetPosition({ 235.0f,330.0f });
	TextureManager::GetInstance()->LoadTexture("Resources/scene/A2.png");
	start2_ = std::make_unique<Sprite>();
	start2_->Initialize(SpriteCommon::GetInstance(), "Resources/scene/A2.png");
	start2_->SetTexSize({ 230.0f,230.0f });
	start2_->SetSize({ 230.0f,230.0f });
	start2_->SetPosition({ 230.0f,325.0f });

	//select

	const std::array<std::string, 5> selectTextures = {
	"Resources/scene/TiN.png",
	"Resources/scene/Tutorial.png",
	"Resources/scene/Easy.png",
	"Resources/scene/Nomal.png",
	"Resources/scene/Hard.png"
	};

	for (int i = 0; i < 5; ++i) {
		TextureManager::GetInstance()->LoadTexture(selectTextures[i]);
		selectSprites_[i] = std::make_unique<Sprite>();
		selectSprites_[i]->Initialize(SpriteCommon::GetInstance(), selectTextures[i]);
		selectSprites_[i]->SetTexSize({ 110.0f, 81.0f });
		selectSprites_[i]->SetSize({ 110.0f, 81.0f });
		selectSprites_[i]->SetPosition({ 808.0f, 300.0f });
	}

	// 天球
	skyDome_ = std::make_unique<SkyDome>();
	skyDome_->Initialize();

	//model
	ModelManager::GetInstance()->LoadModel("titleM/wvBody.obj");
	ModelManager::GetInstance()->LoadModel("scene/MWdoor.obj");
	ModelManager::GetInstance()->LoadModel("scene/dish.obj");
	ModelManager::GetInstance()->LoadModel("scene/MWScenen.obj");
	ModelManager::GetInstance()->LoadModel("titleM/wvb.obj");
	ModelManager::GetInstance()->LoadModel("titleM/wvr.obj");
	ModelManager::GetInstance()->LoadModel("titleM/easy.obj");
	ModelManager::GetInstance()->LoadModel("titleM/normal.obj");
	ModelManager::GetInstance()->LoadModel("titleM/hard.obj");
	ModelManager::GetInstance()->LoadModel("titleM/tutorial.obj");
	ModelManager::GetInstance()->LoadModel("titleM/Tin.obj");

	for (int i = 0; i < 6; ++i) {
		mvModels_[i] = std::make_unique<Object3d>();
		mvModels_[i]->Initialize(Object3dCommon::GetInstance());

		mvTransforms_[i] = std::make_unique<WorldTransform>();
		mvTransforms_[i]->Initialize();
		
	}

	mvModels_[0]->SetModel("titleM/wvBody.obj");
	mvModels_[1]->SetModel("scene/MWdoor.obj");
	mvModels_[2]->SetModel("scene/dish.obj");
	mvModels_[3]->SetModel("scene/MWScenen.obj");
	mvModels_[4]->SetModel("titleM/wvb.obj");
	mvModels_[5]->SetModel("titleM/wvr.obj");

	mvTransforms_[0]->transform.translate = { 0.0f,-1.3f,-10.9f };
	mvTransforms_[0]->transform.rotate = { -0.05f,3.14f,0.0f };
	mvTransforms_[1]->transform.translate = { 1.37f,0.0f,0.939f };
	mvTransforms_[2]->transform.translate = { 0.3f,0.2f,0.0f };
	mvTransforms_[2]->transform.scale = { 0.79f,0.83f,0.82f };
	mvTransforms_[4]->transform.translate = { 0.03f,-0.7f,0.07f };
	mvTransforms_[5]->transform.translate = { -0.86f,0.5f,1.14f };

	for (int i = 0; i < 4; ++i) {
		textModels_[i] = std::make_unique<Object3d>();
		textModels_[i]->Initialize(Object3dCommon::GetInstance());
	}

	textModels_[0]->SetModel("titleM/tutorial.obj");
	textModels_[1]->SetModel("titleM/easy.obj");
	textModels_[2]->SetModel("titleM/normal.obj");
	textModels_[3]->SetModel("titleM/hard.obj");

	textTransform_ = std::make_unique<WorldTransform>();
	textTransform_->Initialize();
	textTransform_->transform.translate = { 1.0f, 0.1f , 0.0f };
	textTransform_->transform.scale = {0.38f,0.66f,1.0f};

	tinModel_ = std::make_unique<Object3d>();
	tinModel_->Initialize(Object3dCommon::GetInstance());
	tinModel_->SetModel("titleM/Tin.obj");

	tinTransform_ = std::make_unique<WorldTransform>();
	tinTransform_->Initialize();
	tinTransform_->transform.translate = { 0.9f, 0.2f , 0.0f };

	easy = false;
	nomal = false;
	hard = false;

	AudioManager::GetInstance()->Initialize();
	AudioManager::GetInstance()->SoundLoadFile("Resources/bgm/title.mp3");
	AudioManager::GetInstance()->SoundLoadFile("Resources/se/tin.mp3");
	AudioManager::GetInstance()->SoundLoadFile("Resources/se/select.mp3");
	AudioManager::GetInstance()->SoundLoadFile("Resources/se/電子レンジを開ける.mp3");
	AudioManager::GetInstance()->SoundLoadFile("Resources/se/電子レンジを閉める.mp3");

	bgm_ = std::make_unique<Audio>();
	se1_ = std::make_unique<Audio>();
	se2_ = std::make_unique<Audio>();
	se3_ = std::make_unique<Audio>();
	tin_ = std::make_unique<Audio>();

	bgm_->Initialize();
	se1_->Initialize();
	se2_->Initialize();
	se3_->Initialize();
	tin_->Initialize();

	

	bgm_->SoundPlay("Resources/bgm/title.mp3", 10000);
	bgm_->SetVolume(0.6f);

	cameraManager_->GetActiveCamera()->GetViewProjection().Initialize();





	mvTransforms_[1]->SetParent(mvTransforms_[0].get());
	mvTransforms_[2]->SetParent(mvTransforms_[0].get());
	mvTransforms_[3]->SetParent(mvTransforms_[1].get());
	mvTransforms_[4]->SetParent(mvTransforms_[0].get());
	mvTransforms_[5]->SetParent(mvTransforms_[0].get());
	tinTransform_->SetParent(mvTransforms_[2].get());
	textTransform_->SetParent(mvTransforms_[2].get());



}

void TitleScene::Finalize() {
	BaseScene::Finalize();

	bgm_->SoundStop("Resources/bgm/title.mp3");
}

void TitleScene::Update() {

	BaseScene::Update();
	directionalLight->Update();
	pointLight->Update();
	mvPointLight->Update();
	spotLight->Update();



	switch (phase_) {
	case Phase::kFadeIn:

		if (fade_->IsFinished()) {
			Input::GetInstance()->SetIsReception(true);
			phase_ = Phase::kMain;
		}

		break;
	case Phase::kMain:
#ifdef _DEBUG
		if (ImGui::TreeNode("directionalLight")) {
			ImGui::ColorEdit4("directionalLight.color", &directionalLight->color_.x, ImGuiColorEditFlags_None);
			if (ImGui::DragFloat3("directionalLight.direction", &directionalLight->direction_.x, 0.01f)) {
				directionalLight->direction_ = Normalize(directionalLight->direction_);
			}
			ImGui::DragFloat("directionalLight.intensity", &directionalLight->intensity_, 0.01f);
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("pointLight")) {
			ImGui::ColorEdit4("pointLight.color", &pointLight->color_.x, ImGuiColorEditFlags_None);
			ImGui::DragFloat3("pointLight.position", &pointLight->position_.x, 0.01f);
			ImGui::DragFloat("pointLight.decay", &pointLight->decay_, 0.01f);
			ImGui::DragFloat("pointLight.radius", &pointLight->radius_, 0.01f);
			ImGui::DragFloat("pointLight.intensity", &pointLight->intensity_, 0.01f);
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("spotLight")) {
			ImGui::ColorEdit4("spotlLight.color", &spotLight->color_.x, ImGuiColorEditFlags_None);
			if (ImGui::DragFloat3("spotLight.direction", &spotLight->direction_.x, 0.01f)) {
				spotLight->direction_ = Normalize(spotLight->direction_);
			}
			ImGui::DragFloat3("spotLight.position", &spotLight->position_.x, 0.01f);
			ImGui::DragFloat("spotLight.decay", &spotLight->decay_, 0.01f);
			ImGui::DragFloat("spotLight.intensity", &spotLight->intensity_, 0.01f);
			ImGui::DragFloat("spotLight.distance", &spotLight->distance_, 0.01f);
			ImGui::DragFloat("spotLight.cosAngle", &spotLight->cosAngle_, 0.01f);
			ImGui::DragFloat("spotLight.cosFalloffStart", &spotLight->cosFalloffStart_, 0.01f);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("condition")) {
			ImGui::Text("selectNum: %d", selectNum);

			Vector2 pos1 = selectSprites_[1]->GetPosition();
			float posArr1[2] = { pos1.x, pos1.y };
			if (ImGui::DragFloat2("select1_.position", posArr1, 1.0f)) {
				selectSprites_[1]->SetPosition(Vector2(posArr1[0], posArr1[1]));
			}
			ImGui::TreePop();
		}
		ImGui::Begin("Model Transform Editor");

		if (ImGui::CollapsingHeader("MW model")) {
			for (int i = 0; i < 6; ++i) {
				std::string label = "Model " + std::to_string(i);
				ImGui::Text("%s", label.c_str());
				ImGui::DragFloat3(("Pos##" + label).c_str(), &mvTransforms_[i]->transform.translate.x, 0.1f);
				ImGui::DragFloat3(("Rot##" + label).c_str(), &mvTransforms_[i]->transform.rotate.x, 0.1f);
				ImGui::DragFloat3(("Scl##" + label).c_str(), &mvTransforms_[i]->transform.scale.x, 0.01f);
				ImGui::Separator();
			}
		}
		if (ImGui::CollapsingHeader("Tin")) {
			ImGui::DragFloat3("Position##Clear", &tinTransform_->transform.translate.x, 0.1f);
			ImGui::DragFloat3("Rotation##Clear", &tinTransform_->transform.rotate.x, 0.1f);
			ImGui::DragFloat3("Scale##Clear", &tinTransform_->transform.scale.x, 0.01f);
		}
		if (ImGui::CollapsingHeader("mvPointLight Light")) {
			ImGui::DragFloat3("mvPointLight Position", &mvPointLight->position_.x, 1.0f);
			ImGui::DragFloat("Intensity", &mvPointLight->intensity_, 0.1f, 0.0f, 100.0f);
			ImGui::DragFloat("Decay", &mvPointLight->decay_, 0.01f, 0.0f, 1.0f);
			ImGui::ColorEdit3("Color", &mvPointLight->color_.x);
		}
		if (ImGui::CollapsingHeader("spotLight Light")) {
			ImGui::DragFloat3("spotLight Position", &spotLight->position_.x, 1.0f);
			ImGui::DragFloat("Intensity", &spotLight->intensity_, 0.1f, 0.0f, 100.0f);
			ImGui::DragFloat("Decay", &spotLight->decay_, 0.01f, 0.0f, 1.0f);
			ImGui::ColorEdit3("Color", &spotLight->color_.x);
		}
		if (ImGui::CollapsingHeader("Text")) {
			ImGui::DragFloat3("Position##Clear", &textTransform_->transform.translate.x, 0.1f);
			ImGui::DragFloat3("Rotation##Clear", &textTransform_->transform.rotate.x, 0.1f);
			ImGui::DragFloat3("Scale##Clear", &textTransform_->transform.scale.x, 0.01f);
		}
		ImGui::End();

#endif

		UpdateSequence();

		

		tinTransform_->UpdateMatrix();// 行列更新
		tinModel_->SetLocalMatrix(MakeIdentity4x4());// ローカル行列を単位行列に
		tinModel_->Update();// 更新

		textTransform_->UpdateMatrix();// 行列更新
		for (int i = 0; i < 4; ++i) {
			textModels_[i]->SetLocalMatrix(MakeIdentity4x4());// ローカル行列を単位行列に
			textModels_[i]->Update();// 更新
		}

		for (int i = 0; i < 6; i++) {
			mvTransforms_[i]->UpdateMatrix();// 行列更新
			mvModels_[i]->SetLocalMatrix(MakeIdentity4x4());// ローカル行列を単位行列に
			mvModels_[i]->Update();// 更新
		}

		
		//select
		for (auto& sprite : selectSprites_) {
			sprite->Update();
		}
		

			if (sequenceState_ == SequenceState::Done && Input::GetInstance()->TriggerGamePadButton(Input::GamePadButton::A)) {
				if (selectNum != 0) {

				
				}

				fade_->Start(Fade::Status::FadeOut, fadeTime_);
				phase_ = Phase::kFadeOut;
			}
		
		

		break;
	case Phase::kFadeOut:

		if (fade_->IsFinished()) {

			if (selectNum == 1) {

				SceneManager::GetInstance()->ChangeScene("TUTORIAL");

			}
			else
			{
				SceneManager::GetInstance()->ChangeScene("GAME");
			}


			return;
		}

		break;

	case Phase::kPlay:
		break;
	case Phase::kPose:
		break;
	}

	skyDome_->Update();

	
}

void TitleScene::Draw()
{
	BaseScene::Draw();

	DrawBackgroundSprite();
	/// 背景スプライト描画

	
	DrawObject();
	/// オブジェクト描画	

	skyDome_->Draw(cameraManager_->GetActiveCamera()->GetViewProjection(),
		*directionalLight.get(),
		*pointLight.get(),
		*spotLight.get());

	tinModel_->Draw(*tinTransform_.get(), cameraManager_->GetActiveCamera()->GetViewProjection(),
		*directionalLight, *mvPointLight, *spotLight);


	if (selectNum == 1) {
		textModels_[0]->Draw(*textTransform_, cameraManager_->GetActiveCamera()->GetViewProjection(),
			*directionalLight, *mvPointLight, *spotLight);
	} else if (selectNum == 2) {
		textModels_[1]->Draw(*textTransform_, cameraManager_->GetActiveCamera()->GetViewProjection(),
			*directionalLight, *mvPointLight, *spotLight);
	} else if (selectNum == 3) {
		textModels_[2]->Draw(*textTransform_, cameraManager_->GetActiveCamera()->GetViewProjection(),
			*directionalLight, *mvPointLight, *spotLight);
	} else if (selectNum == 4) {
		textModels_[3]->Draw(*textTransform_, cameraManager_->GetActiveCamera()->GetViewProjection(),
			*directionalLight, *mvPointLight, *spotLight);
	}

	for (int i = 0; i < 6; ++i) {
		mvModels_[i]->Draw(*mvTransforms_[i], cameraManager_->GetActiveCamera()->GetViewProjection(),
			*directionalLight, *mvPointLight, *spotLight);
	}




	DrawForegroundSprite();	
	/// 前景スプライト描画	

	
	if (selectNum == 0) {
		selectSprites_[0]->Draw();
	} else if (selectNum == 1) {
		selectSprites_[1]->Draw();
	} else if (selectNum == 2) {
		selectSprites_[2]->Draw();
	} else if (selectNum == 3) {
		selectSprites_[3]->Draw();
	} else if (selectNum == 4) {
		selectSprites_[4]->Draw();
	}
	
	fade_->Draw();

}

void TitleScene::select() {

	

	Vector2 stickInput = Input::GetInstance()->GetLeftStick();

		if (stickInput.x > 0.5f && stickReleased_) {
			se1_->SoundPlay("Resources/se/select.mp3", 0);
			se1_->SetVolume(0.5f);
			selectNum++;
			stickReleased_ = false;
			mvTransforms_[5]->transform.rotate.z += 0.4f;
		}

		else if (stickInput.x < -0.5f && stickReleased_) {
			se1_->SoundPlay("Resources/se/select.mp3", 0);
			se1_->SetVolume(0.5f);
			selectNum--;
			stickReleased_ = false;
			mvTransforms_[5]->transform.rotate.z -= 0.4f;
		}

		if (std::abs(stickInput.x) < 0.3f) {
			stickReleased_ = true;
		}
	//}
	if (selectNum >= 5) {
		selectNum = 1;
	}
	if (selectNum <= 0) {
		selectNum = 4;
	}
	if (selectNum == 1) {
			textTransform_->transform.translate = { 1.0f, 0.1f , 0.0f };
		if (Input::GetInstance()->TriggerGamePadButton(Input::GamePadButton::A)) {
			se1_->SoundPlay("Resources/se/select.mp3", 0);
			se1_->SetVolume(0.5f);
			mvTransforms_[4]->transform.translate.z -= 0.04f;
			tutorial = true;
			SceneManager::GetInstance()->GetTransitionData().easy = false;
			SceneManager::GetInstance()->GetTransitionData().nomal = false;
			SceneManager::GetInstance()->GetTransitionData().hard = false;
		}
	}
	if (selectNum == 2) {
			textTransform_->transform.translate = { 1.0f, 0.2f , 0.0f };
		if (Input::GetInstance()->TriggerGamePadButton(Input::GamePadButton::A)) {
			se1_->SoundPlay("Resources/se/select.mp3", 0);
			se1_->SetVolume(0.5f);
			mvTransforms_[4]->transform.translate.z -= 0.04f;
			easy = true;
			SceneManager::GetInstance()->GetTransitionData().easy = true;
			SceneManager::GetInstance()->GetTransitionData().nomal = false;
			SceneManager::GetInstance()->GetTransitionData().hard = false;
		}
	}
	if (selectNum == 3) {
			textTransform_->transform.translate = { 1.0f, 0.2f , 0.0f };
		if (Input::GetInstance()->TriggerGamePadButton(Input::GamePadButton::A)) {
			se1_->SoundPlay("Resources/se/select.mp3", 0);
			se1_->SetVolume(0.5f);
			mvTransforms_[4]->transform.translate.z -= 0.04f;
			nomal = true;
			SceneManager::GetInstance()->GetTransitionData().easy = false;
			SceneManager::GetInstance()->GetTransitionData().nomal = true;
			SceneManager::GetInstance()->GetTransitionData().hard = false;
		}
	}
	if (selectNum == 4) {
			textTransform_->transform.translate = { 1.0f, 0.2f , 0.0f };
		if (Input::GetInstance()->TriggerGamePadButton(Input::GamePadButton::A)) {
			se1_->SoundPlay("Resources/se/select.mp3", 0);
			se1_->SetVolume(0.5f);
			mvTransforms_[4]->transform.translate.z -= 0.04f;
			hard = true;
			SceneManager::GetInstance()->GetTransitionData().easy = false;
			SceneManager::GetInstance()->GetTransitionData().nomal = false;
			SceneManager::GetInstance()->GetTransitionData().hard = true;
		}
		
	}
}

void TitleScene::UpdateSequence() {

	switch (sequenceState_) {
	case SequenceState::RotateDish:
		mvTransforms_[2]->transform.rotate.y += 0.05f;
		dishAngle_ += 0.05f;
		if (dishAngle_ >= 6.28f) {
			mvTransforms_[2]->transform.rotate.y = 6.28f;
			tin_->SoundPlay("Resources/se/tin.mp3", 0);
			tin_->SetVolume(1.3f);
			sequenceState_ = SequenceState::RotateDoor;
		}
		break;

	case SequenceState::RotateDoor:
		
		if (mvTransforms_[1]->transform.rotate.y < 2.7f) {
			
			mvTransforms_[1]->transform.rotate.y += 0.05f;
		} else {
			mvTransforms_[1]->transform.rotate.y = 2.7f;
			sequenceState_ = SequenceState::MoveTinToFirst;
		}
		break;

	case SequenceState::MoveTinToFirst: {
		Vector3 target = { 0.7f, 0.5f, 2.1f };
		Vector3& pos = tinTransform_->transform.translate;
		Vector3 delta = target - pos;
		float distance = Length(delta);
		float speed = 0.03f;

		if (distance <= speed) {
			pos = target;
			sequenceState_ = SequenceState::MoveTinToSecond;
		} else {
			pos = pos + Normalize(delta) * speed;
		}
		break;
	}

	case SequenceState::MoveTinToSecond: {
		Vector3 target = { 0.7f, 1.9f, 1.9f };
		Vector3& pos = tinTransform_->transform.translate;
		Vector3 delta = target - pos;
		float distance = Length(delta);
		float speed = 0.03f;

		if (distance <= speed) {
			pos = target;
			sequenceState_ = SequenceState::ResetDoorRotation;
		} else {
			pos = pos + Normalize(delta) * speed;
		}
		break;
	}

	case SequenceState::ResetDoorRotation:
		if (mvTransforms_[1]->transform.rotate.y > 0.05f) {
			mvTransforms_[1]->transform.rotate.y -= 0.05f;
		} else {
			mvTransforms_[1]->transform.rotate.y = 0.0f;
			selectNum = 1;          
			stickReleased_ = false;
			sequenceState_ = SequenceState::Done;
		}
		break;

	case SequenceState::Done:
	
		select();
		const float amplitude = 0.08f;     
		const float speed = 2.0f;          
		selectFloatTimer_ += 1.0f / 60.0f;
		textTransform_->transform.translate.y = 0.2f + amplitude * sinf(selectFloatTimer_ * speed);
		break;
	}

}
