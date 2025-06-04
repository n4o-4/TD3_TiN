#pragma once
#include <d3d12.h>
#include <wrl.h>
#include "Structs.h"

#include "DirectXCommon.h"

#include "MyMath.h"

//struct ConstBufferDataViewProjection
//{
//	Vector3 worldPosition;
//	float padding[1];
//	Matrix4x4 matView;
//	Matrix4x4 matProjection;
//};

struct ConstBufferDataViewProjection
{
	Vector3 worldPosition;
	float padding[1];
	Matrix4x4 matView;
	Matrix4x4 matProjection;
};

class ViewProjection
{
public:

	void Initialize();

	void Update();

	Microsoft::WRL::ComPtr<ID3D12Resource> &GetViewProjectionResource() { return viewProjectionResource_; }

public:

	Transform transform;

	Vector3 worldPosition_;
	Matrix4x4 matWorld_;
	Matrix4x4 matView_;
	Matrix4x4 matProjection_;

	float fovY;
	float aspectRation;
	float nearClip;
	float farClip;

private:

	Microsoft::WRL::ComPtr<ID3D12Resource> viewProjectionResource_;

	ConstBufferDataViewProjection* viewProjectionData_;
};
