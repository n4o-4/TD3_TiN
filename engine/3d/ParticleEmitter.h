#pragma once
#include "ParticleManager.h"

class ParticleEmitter
{
private:

public:

    void Initialize(std::string name);

    void Emit();

	void RadialEmit();
    
    // 指定した数のパーティクルを放出するオーバーロードを追加
    void Emit(uint32_t count,Vector3 startColor,Vector3 finishColor);

    // エミッタの位置を設定するメソッドを追加
    void SetPosition(const Vector3& position);
    
    // パーティクル数を設定するメソッド
    void SetParticleCount(uint32_t count);
    
    // 放出頻度を設定するメソッド
    void SetFrequency(float frequency);

    void Update();

    void SetStartColorRange(ParticleManager::ColorRange colorRange) { particleStates.startColorRange = colorRange; }
    void SetFinishColorRange(ParticleManager::ColorRange colorRange) { particleStates.finishColorRange = colorRange; }

	void SetStartScaleRange(ParticleManager::Vec3Range scaleRange) { particleStates.startScaleRange = scaleRange; }
	void SetFinishScaleRange(ParticleManager::Vec3Range scaleRange) { particleStates.finishScaleRange = scaleRange; }

	void SetRotateRange(ParticleManager::Vec3Range rotateRange) { particleStates.rotateRange = rotateRange; }
	void SetTranslateRange(ParticleManager::Vec3Range translateRange) { particleStates.translateRange = translateRange; }

    void SetVelocityRange(ParticleManager::Vec3Range velocityRange) { particleStates.velocityRange = velocityRange; }
    void SetLifeTimeRange(ParticleManager::Range lifeTimeRange) { particleStates.lifeTimeRange = lifeTimeRange; }

private:

    struct Emitter {
        ParticleManager::Transform transform; // エミッタのTransform
        uint32_t count; // 発生数
        float frequency; // 発生頻度
        float frequencyTime; // 頻度用時刻
		float lifeTime; // 寿命
		Vector4 startColor; // 開始色
		Vector4 finishColor; // 終了色
    };

    Emitter emitter{};

    std::string name;

    std::unordered_map<std::string, ParticleManager::ParticleGroup> particleGroups;

    std::mt19937 randomEngine;

	ParticleManager::ParticleStates particleStates;

};