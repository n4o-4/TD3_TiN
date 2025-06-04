#include "TutorialEnemy.h"
#include "ModelManager.h"
#include "PlayerMissile.h"
#include "PlayerMachineGun.h"

void TutorialEnemy::Initialize(Model* model) {
	ModelManager::GetInstance()->LoadModel("enemy/kumo/kumo.obj");
	BaseEnemy::Initialize(ModelManager::GetInstance()->FindModel("enemy/kumo/kumo.obj"));
	SetVelocity({ 0.0f, 0.0f, 0.0f }); // 動かない
	SetModelScale({ 5.0f, 5.0f, 5.0f }); // 任意の大きさ
}

void TutorialEnemy::Update() {
	BaseEnemy::Update(); // 描画と当たり判定処理は継続


}

void TutorialEnemy::Draw(ViewProjection viewProjection,
	DirectionalLight directionalLight,
	PointLight pointLight,
	SpotLight spotLight) {
	BaseEnemy::Draw(viewProjection, directionalLight, pointLight, spotLight);
}


void TutorialEnemy::OnCollisionEnter(BaseObject* other) {
	if (dynamic_cast<PlayerMissile*>(other)) {
		//se1_->SoundPlay("Resources/se/爆発1.mp3", 0);
		//--hp_;
		//HitJump();
		particleEmitter_->Emit();
	}
	if (dynamic_cast<PlayerMachineGun*>(other)) {
		//--hp_;
		//HitJump();
		particleEmitter_->Emit();
	}
}
