#include "SpotLight.h"

void SpotLight::Initialize()
{
	// bufferResourceの生成
	spotLightResource_ = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(SpotLightData));

	// データをマップ
	spotLightResource_.Get()->Map(0, nullptr, reinterpret_cast<void**>(&spotLightData_));

	color_ = { 1.0f,1.0f,1.0f,1.0f };
	position_ = { 2.0f,1.25f,0.0f };
	intensity_ = 1.0f;
	direction_ = Normalize({ -1.0f,-1.0f,0.0f });
	distance_ = 7.0f;
	decay_ = 2.0f;
	cosAngle_ = std::cos(std::numbers::pi_v<float> / 3.0f);
	cosFalloffStart_ = 1.0f;

	spotLightData_->color = color_;
	spotLightData_->position = position_;
	spotLightData_->intensity = intensity_;
	spotLightData_->direction = direction_;
	spotLightData_->distance = distance_;
	spotLightData_->decay = decay_;
	spotLightData_->cosAngle = cosAngle_;
	spotLightData_->cosFalloffStart = cosFalloffStart_;
}

void SpotLight::Update()
{
	spotLightData_->color = color_;
	spotLightData_->position = position_;
	spotLightData_->intensity = intensity_;
	spotLightData_->direction = direction_;
	spotLightData_->distance = distance_;
	spotLightData_->decay = decay_;
	spotLightData_->cosAngle = cosAngle_;
	spotLightData_->cosFalloffStart = cosFalloffStart_;
}
