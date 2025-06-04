#pragma once
#include "BaseCamera.h"

class Camera : public BaseCamera
{
private:

	static std::unique_ptr<Camera> instance;

	friend std::unique_ptr<Camera> std::make_unique<Camera>();
	friend std::default_delete<Camera>;

	Camera() = default;
	~Camera() = default;
	Camera(Camera&) = delete;
	Camera& operator=(Camera&) = delete;


public:

	static Camera* GetInstance();

	void Initialize();

	void Finalize();

	// 更新
	void Update();

	// setter
	void SetRotate(const Vector3& rotate) { viewProjection_->transform.rotate = rotate; }
	void SetTranslate(const Vector3& translate) { viewProjection_->transform.translate = translate; }
	void SetFovY(float fovY) { viewProjection_->fovY = fovY; }
	void SetAspectRation(float aspectRation) { viewProjection_->aspectRation = aspectRation; }

	void SetViewProjection(const ViewProjection& viewProjection) { *viewProjection_ = viewProjection; }

	// getter
	const Matrix4x4& GetWorldMatrix() const { return worldMatrix; }
	const Vector3& GetRotate() const { return viewProjection_->transform.rotate; }
	const Vector3& GetTranslate() const { return viewProjection_->transform.translate; }

	ViewProjection& GetViewProjection() { return *viewProjection_; }

private:
	

	/*float fovY;
	float aspectRation;
	float nearClip;
	float farClip;*/
};

