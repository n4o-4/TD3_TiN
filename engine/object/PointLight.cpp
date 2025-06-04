#include "PointLight.h"

void PointLight::Initilize()
{
	// bufferResourceの生成
	pointLightResource_ = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(PointLightData));

	// データをマップ
	pointLightResource_.Get()->Map(0, nullptr, reinterpret_cast<void**>(&pointLightData_));

	// 数値を初期化
	color_ = { 1.0f,1.0f,1.0f,1.0f};
	position_ = { 0.0f,2.0f,0.0f };
	intensity_ = 1.0f;
	radius_ = 2.8f;
	decay_ = 0.95f;

	// 数値をデータに代入
	pointLightData_->color = color_;
	pointLightData_->position = position_;
	pointLightData_->intensity = intensity_;
	pointLightData_->radius = radius_;
	pointLightData_->decay = decay_;
}

void PointLight::Update()
{

	// 数値をデータに代入
	pointLightData_->color = color_;
	pointLightData_->position = position_;
	pointLightData_->intensity = intensity_;
	pointLightData_->radius = radius_;
	pointLightData_->decay = decay_;

}
