#pragma once
#include <memory>
#include "BaseScene.h"
#include "AbstractSceneFactory.h"
#include "Camera.h"
#include "SrvManager.h"
#include "PostEffect.h"

struct SceneTransitionData {
	bool easy = false;
	bool nomal = false;
	bool hard = false;
};

// シーン管理
class SceneManager
{
public:

	static SceneManager* GetInstance();

	// 初期化
	void Initialize(DirectXCommon* dxCommon,SrvManager* srvManager,Camera* camera);

	void Finalize();

	// 更新　
	void Update();

	// 描画
	void Draw();

	void ChangeScene(const std::string& sceneName);
	
	void SetSceneFactory(AbstaractSceneFactory& sceneFactory) { sceneFactory_ = &sceneFactory; }

	void SetPostEffect(PostEffect* postEffect) { postEffect_ = postEffect; }

	DirectXCommon* GetDxCommon() { return dxCommon_; }	

	SrvManager* GetSrvManager() { return srvManager_; }

	Camera* GetCamera() { return camera_; }

	PostEffect* GetPostEffect() { return postEffect_; }

	// データ配信用の構造体アクセサ
	SceneTransitionData& GetTransitionData() { return transitionData_; }


private:

	static std::unique_ptr<SceneManager> instance;

	friend std::unique_ptr<SceneManager> std::make_unique<SceneManager>();
	friend std::default_delete<SceneManager>;

	~SceneManager() = default;
	SceneManager() = default;
	SceneManager(SceneManager&) = delete;
	SceneManager& operator=(SceneManager&) = delete;

	
	// 今のシーン
	std::unique_ptr<BaseScene> scene_ = nullptr;

	// 次のシーン
	std::unique_ptr<BaseScene> nextScene_ = nullptr;

	AbstaractSceneFactory* sceneFactory_ = nullptr;

	SrvManager* srvManager_ = nullptr;

	DirectXCommon* dxCommon_ = nullptr;

	Camera* camera_ = nullptr;

	PostEffect* postEffect_ = nullptr;
	
	SceneTransitionData transitionData_;
};