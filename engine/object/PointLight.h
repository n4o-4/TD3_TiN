#pragma once
#include <d3d12.h>
#include <wrl.h>
#include "Structs.h"

#include "DirectXCommon.h"

struct PointLightData
{
	Vector4 color; // ライトの色
	Vector3 position; // ライトの位置
	float intensity; // 輝度
	float radius; // ライトの届く最大距離
	float decay; // 減衰率
	float padding[2];
};

class PointLight
{
public:

	void Initilize();

	void Update();

	const Microsoft::WRL::ComPtr<ID3D12Resource>& GetPointLightResource() { return pointLightResource_; }

public:

	Vector4 color_;
	Vector3 position_;
	float intensity_;
	float radius_;
	float decay_;

private:

	Microsoft::WRL::ComPtr<ID3D12Resource> pointLightResource_;

	PointLightData* pointLightData_ = nullptr;
};