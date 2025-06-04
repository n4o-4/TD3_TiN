#include "DirectionalLight.h"

void DirectionalLight::Initilaize()
{
	// bufferResourceの生成
	directionalLightResource_ = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(DirectionalLightData));

	// データをマップ
	directionalLightResource_.Get()->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData_));

	// 数値を初期化
	color_ = { 1.0f,1.0f,1.0f,1.0f };
	direction_ = { 1.0f,0.0f,0.0f };
	intensity_ = 1.0f;
	
	directionalLightData_->color = color_;
	directionalLightData_->direction = direction_;
	directionalLightData_->intensity = intensity_;
}

void DirectionalLight::Update()
{
	directionalLightData_->color = color_;
	directionalLightData_->direction = direction_;
	directionalLightData_->intensity = intensity_;
}
