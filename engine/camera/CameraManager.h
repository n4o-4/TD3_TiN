#pragma once
#include "BaseCamera.h"
#include "Camera.h"
#include "DebugCamera.h"
#include "FollowCamera.h"

class CameraManager
{
public: /// メンバ関数	

	// 初期化
	void Initialize();

	// 更新
	void Update();

	// アクティブカメラの取得
	BaseCamera* GetActiveCamera() { return activeCamera_; }

	void SetFollowCameraTarget(WorldTransform* target) { followCamera_->SetTarget(target); }

	FollowCamera* GetFollowCamera() { return followCamera_.get(); }

private:

	void ChangeActiveCamera();

public:

	bool useDebugCamera_ = false;

	bool useFollowCamera_ = false;	

	bool useDefaultCamera_ = false;

private:
	std::unique_ptr<DebugCamera> debugCamera_ = nullptr;

	std::unique_ptr<FollowCamera> followCamera_ = nullptr;

	std::list<BaseCamera*> cameras_;

	BaseCamera* activeCamera_ = nullptr;
};

