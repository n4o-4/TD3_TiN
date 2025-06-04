#pragma once
#include <algorithm>

#include "Input.h"
#include "BaseCamera.h"
#include "WorldTransform.h"

class DebugCamera : public BaseCamera
{
public:

	// 初期化
	void Initialize() override;

	// 更新
	void Update() override;

	ViewProjection& GetViewProjection() { return *viewProjection_; }

private:
		
	std::unique_ptr<WorldTransform> targetTransform_ = nullptr;

	// ターゲットからのオフセット 
	Vector3 offset = { 0.0f,0.0f,-10.0f };
};

