#pragma once
#include "BaseEnemy.h"

class TutorialEnemy : public BaseEnemy {
public:
	TutorialEnemy() : BaseEnemy() {}

	void Initialize(Model* model) override;
	void Update() override;

	void Draw(ViewProjection viewProjection,
		DirectionalLight directionalLight,
		PointLight pointLight,
		SpotLight spotLight);

	void OnCollisionEnter(BaseObject* other) override;
	void OnCollisionStay(BaseObject* other) override {}
	void OnCollisionExit(BaseObject* other) override {}

	void SetPosition(const Vector3& position) {
		worldTransform_->transform.translate = position;
		spawnPosition_ = position;
	}

	void SetTarget(WorldTransform* target) {
		target_ = target;
	}
};
