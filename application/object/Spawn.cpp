#include "Spawn.h"
#include "PlayerMissile.h"
#include "PlayerMachineGun.h"
void Spawn::Initialize() {

	ModelManager::GetInstance()->LoadModel("Spawn/Spawn.obj");
	BaseEnemy::Initialize(ModelManager::GetInstance()->FindModel("Spawn/Spawn.obj"));

	worldTransform_->transform.scale = Vector3(2.5f, 2.5f, 2.5f);

	//spawnModel_ = std::make_unique<Object3d>();
	//spawnModel_->Initialize(Object3dCommon::GetInstance());
	//// 
	//ModelManager::GetInstance()->LoadModel("Spawn/Spawn.obj");
	//spawnModel_->SetModel(ModelManager::GetInstance()->FindModel("Spawn/Spawn.obj"));
	//
	//spawnWorldTransform_ = std::make_unique<WorldTransform>();
	//spawnWorldTransform_->Initialize();

}

void Spawn::Update() {

	BaseEnemy::Update();
	
	//if (spawnHp_ > 0) {
	//spawnWorldTransform_->UpdateMatrix();
	//spawnModel_->
	// (MakeIdentity4x4());
	//spawnModel_->Update();
}

void Spawn::Draw(ViewProjection viewProjection, DirectionalLight directionalLight, PointLight pointLight, SpotLight spotLight) {
	//spawnModel_->Draw(*spawnWorldTransform_.get(), viewProjection, directionalLight, pointLight, spotLight);
	
		BaseEnemy::Draw(viewProjection, directionalLight, pointLight, spotLight);
	
}
	

void Spawn::OnCollisionEnter(BaseObject* other) {
	if (dynamic_cast<PlayerMissile*>(other)) {
		--spawnHp_;
		particleEmitter_->Emit();
	}
	if (dynamic_cast<PlayerMachineGun*>(other)) {
		--spawnHp_;
		particleEmitter_->Emit();
	}
}

void Spawn::OnCollisionStay(BaseObject* other) {
}

void Spawn::OnCollisionExit(BaseObject* other) {
}
