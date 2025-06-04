#include "Object3d.h"
#include "MyMath.h"
#include "TextureManager.h"
#include "ModelDatas.h"
#include "imgui.h"

void Object3d::Initialize(Object3dCommon* object3dCommon)
{
	this->object3dCommon = object3dCommon;

	transformationResource = this->object3dCommon->GetDxCommon()->CreateBufferResource(sizeof(TransformationMatrix));

	// データを書き込むためのアドレスを取得
	transformationResource.Get()->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));

	//　単位行列を書き込んでおく
	transformationMatrixData->WVP = MakeIdentity4x4();

	this->camera = object3dCommon->GetDefaultCamera();

	transform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	materialResource_ = object3dCommon->GetDxCommon()->CreateBufferResource(sizeof(Material));

	// 書き込むためのアドレスを取得
	materialResource_.Get()->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));

	materialData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialData_->enableLighting = true;
	materialData_->uvTransform = MakeIdentity4x4();

	materialData_->shininess = 48.3f;

	materialData_->specularColor = { 1.0f,1.0f,1.0f };
}

void Object3d::Update()
{
	Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	Matrix4x4 worldViewProjectionMatrix;
	
#ifdef _DEBUG

	//ImGui::DragFloat3("light.Direction", &directionLightData->direction.x);

	//if (ImGui::DragFloat3("light.Direction", &directionLightData->direction.x,0.01f)) {
	//	// Normalize the direction vector
	//	Vector3& dir = directionLightData->direction;
	//	float length = std::sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
	//	if (length > 0.0f) {
	//		dir.x /= length;
	//		dir.y /= length;
	//		dir.z /= length;
	//	}
	//}

	//ImGui::DragFloat("light.intensity", &directionLightData->intensity, 0.01f);


#endif

	

	if (camera) {
		

	}
	else {

		worldViewProjectionMatrix = worldMatrix;
	}
	//transformationMatrixData->WVP = worldViewProjectionMatrix;
}

void Object3d::Draw(WorldTransform worldTransform,ViewProjection viewProjection,DirectionalLight directionalLight ,PointLight pointLight,SpotLight spotLight)
{
	worldTransform.matWorld_ = localMatrix * worldTransform.matWorld_;

	worldTransform.TransferMatrix();

	// depthtest書き込みする場合
	if (writeDepthTest_) {
		object3dCommon->GetDxCommon()->GetCommandList()->SetPipelineState(object3dCommon->GetGraphicsPipelineState().Get());
	}
	else {
		object3dCommon->GetDxCommon()->GetCommandList()->SetPipelineState(object3dCommon->GetDisableDepthTestPipelineState().Get());
	}

	object3dCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource_.Get()->GetGPUVirtualAddress());

	object3dCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, viewProjection.GetViewProjectionResource()->GetGPUVirtualAddress());
	
	object3dCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLight.GetDirectionalLightResource()->GetGPUVirtualAddress());

	object3dCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(5, pointLight.GetPointLightResource()->GetGPUVirtualAddress());

	object3dCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(6, spotLight.GetSpotLightResource()->GetGPUVirtualAddress());

	if (model) {
		model->Draw(worldTransform);
	}
}

void Object3d::SetModel(const std::string& filePath)
{
	model = ModelManager::GetInstance()->FindModel(filePath);
}


