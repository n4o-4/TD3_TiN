#pragma once
#include <random>

#include "DirectXCommon.h"
#include "TextureManager.h"
#include "CameraManager.h"

///==============================
// エフェクト
#include "BaseEffect.h"
#include "Grayscale.h"
#include "Vignette.h"
#include "BoxFilter.h"
#include "GaussianFilter.h"
#include "LuminanceBasedOutline.h"
#include "DepthBasedOutline.h"
#include "RadialBlur.h"
#include "Dissolve.h"
#include "Random.h"
#include "LinearFog.h"
#include "MotionBlur.h"
#include "Gritch.h"

class PostEffect
{
public:
	//==============================
	// エフェクトの種類
	enum class EffectType
	{
		// モノクロ
		Grayscale,

		// 周辺減光効果
		Vignette,

		// ボックスフィルターによるぼかし
		BoxFilter,

		// ガウス関数によるぼかし
		GaussianFilter,

		// 輝度検出でのアウトライン
		LuminanceBasedOutline,

		// 深度検出でのアウトライン
		DepthBasedOutline,

		// RadialBlur
		RadialBlur,

		// Dissolve
		Dissolve,

		// Random
		Random,

		// LinearFog
		LinearFog,

		// MotionBlur
		MotionBlur,

		// Gritch
		Gritch,

		// ↑↑↑追加↑↑↑

		EffectCount,
	};

private:

	struct Pipeline
	{
		Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;
	};

	struct ActiveEffect
	{
		std::unique_ptr<BaseEffect> effect;
		EffectType type;
	};

public:

	// 初期化
	void Initialize(DirectXCommon* dxCommon,SrvManager* srvManager);

	// 終了
	void Finalize();

	// 更新
	void Update();

	// 描画
	void Draw();

	// 指定のエフェクトをる
	void ApplyEffect(std::string name,EffectType type);

	// カメラマネージャを設定
	void SetCameraManager(CameraManager* cameraManager) { cameraManager_ = cameraManager; }

	void ResetActiveEffect();

    BaseEffect* GetEffectData(std::string name) { 
       auto it = activeEffects_.find(name); 
       if (it != activeEffects_.end()) { 
           return it->second->effect.get(); 
       } 
       return nullptr; 
    }
private:

private: // メンバ変数
	
	DirectXCommon* dxCommon_ = nullptr;

	SrvManager* srvManager_ = nullptr;

	CameraManager* cameraManager_ = nullptr;

	//std::list<std::unique_ptr<ActiveEffect>> activeEffects_;

	std::unordered_map<std::string, std::unique_ptr<ActiveEffect>> activeEffects_;
};

