#pragma once

#include "Vectors.h"
#include "Matrixs.h"
#include "MyMath.h"	
#include "ViewProjection.h"	

class BaseCamera
{
public:
	// 初期化
	virtual void Initialize();

	// 更新処理
	virtual void Update();

	virtual ViewProjection& GetViewProjection() { return *viewProjection_; }

protected:

	std::unique_ptr<ViewProjection> viewProjection_ = nullptr;

	Matrix4x4 worldMatrix = MakeIdentity4x4();

};

