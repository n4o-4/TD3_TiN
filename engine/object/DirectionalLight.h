#pragma once
#include <d3d12.h>
#include <wrl.h>
#include "Structs.h"

#include "DirectXCommon.h"

class DirectionalLight
{
private:

	struct DirectionalLightData {
		Vector4 color;
		Vector3 direction;
		float intensity;
	};

public:

	void Initilaize();

	void Update();

	const Microsoft::WRL::ComPtr<ID3D12Resource>& GetDirectionalLightResource() { return directionalLightResource_; }
public:

	Vector4 color_;
	Vector3 direction_;
	float intensity_;

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource_;

	DirectionalLightData *directionalLightData_ = nullptr;
};