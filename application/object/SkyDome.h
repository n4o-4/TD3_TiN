#pragma once

#include "Kouro.h"
class SkyDome {

public:
	void Initialize();
	void Update();
	void Draw(ViewProjection viewProjection, DirectionalLight directionalLight, PointLight pointLight, SpotLight spotLight);

private:
	//基本スカイドーム
	std::unique_ptr<Object3d> model_ = nullptr;
	std::unique_ptr<WorldTransform> worldTransform_ = nullptr;

	// スカイドームの背景
	std::unique_ptr<Object3d> modelSkydomeBg_ = nullptr;
	std::unique_ptr<WorldTransform> worldTransformSkydomeBg_ = nullptr;
	// スカイドームの雲
	std::unique_ptr<Object3d> modelSkydomeCloud_ = nullptr;
	std::unique_ptr<WorldTransform> worldTransformSkydomeCloud_ = nullptr;
	//cube
	std::unique_ptr<Object3d> cube_ = nullptr;
	std::unique_ptr<WorldTransform> worldTransformCube_ = nullptr;

};

