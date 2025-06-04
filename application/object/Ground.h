#pragma once
#include "Kouro.h"
class Ground {


public:
	void Initialize();
	void Update();
	void Draw(ViewProjection viewProjection, DirectionalLight directionalLight, PointLight pointLight, SpotLight spotLight);

private:

	std::unique_ptr<Object3d> model_ = nullptr;

	std::unique_ptr<WorldTransform> worldTransform_ = nullptr;



};

