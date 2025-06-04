#pragma once
#include "Kouro.h"
#include "BaseObject.h"
#include "BaseEnemy.h"
#include "LockOnMarker.h"
#include "Spawn.h"

class LockOn {
public:
    // ロックオンレベルを表す列挙型
    enum class LockLevel {
        None,           // ロックオンなし
        SimpleLock,     // 簡易ロックオン
        PreciseLock     // 精密ロックオン
    };
    
    // 敵ごとのロックオン情報を管理する構造体
	struct LockedEnemyInfo {
        BaseEnemy* enemy;                 // ロックオン対象の敵
    	float lockTimer;              // ロックオン継続時間
	    LockLevel lockLevel;          // 現在のロックオンレベル
	    float viewAlignmentFactor;    // 視点との整列度（0.0～1.0）
	    Vector3 initialPosition;      // 初回ロックオン時の敵の位置（簡易ロックオン用）
	};

    void Initialize(); 
    void Update(const std::vector<std::unique_ptr<BaseEnemy>>& enemies);
    void UpdateRaw(const std::vector<BaseEnemy*>& enemies);
    void Draw(ViewProjection viewProjection, DirectionalLight directionalLight, PointLight pointLight, SpotLight spotLight);

    void SetPosition(const Vector3& position) { lockOnWorldTransform_->transform.translate = position; }
    // LockOn 範囲内の敵を検知する関数
    void DetectEnemies(const std::vector<std::unique_ptr<BaseEnemy>>& enemies);
    void DetectEnemiesRaw(const std::vector<BaseEnemy*>& enemies);
    //検出された敵のリストを取得する関数
    size_t GetLockedEnemyCount() const { return lockedEnemies_.size(); }

    // 視点方向を設定
    void SetViewDirection(const Vector3& viewDir) { viewDirection_ = viewDir; }
    
    // ロックオン対象の敵を削除
    void RemoveLockedEnemy(BaseEnemy* enemy);
    
    // 特定の敵のロックオンレベルを取得
    LockLevel GetLockLevel(BaseEnemy* enemy) const;
    
    // 特定の敵のロックオン情報を取得
    LockedEnemyInfo* GetLockedEnemyInfo(BaseEnemy* enemy);
    
    // ロックオン対象リストを取得
    const std::vector<BaseEnemy*>& GetLockedEnemies() const { return lockedEnemies_; }
    
    // 精密ロックオン移行時間の取得
    float GetTimeToPreciseLock() const { return timeToPreciseLock_; }\
    
    // この敵はロックオンされているか
    bool IsLocked(BaseEnemy* enemy) {
        return std::find(lockedEnemies_.begin(), lockedEnemies_.end(), enemy) != lockedEnemies_.end();
    }

	// 特定の敵の追尾位置を取得（簡易ロックオンでは初期位置を返す）
Vector3 GetLockedEnemyPosition(BaseEnemy* enemy) const;

private:
    //lockOn model
    //std::unique_ptr<Object3d> lockOn_ = nullptr;
    //ワールド変換
    std::unique_ptr<WorldTransform> lockOnWorldTransform_ = nullptr;
    
    // 検出された敵のリスト
    std::vector<BaseEnemy*> lockedEnemies_;
    
    // 敵ごとのロックオン情報リスト
    std::vector<LockedEnemyInfo> lockedEnemiesInfo_;
    
    //検出されたEnemyの前に表示されるオブジェクト
    std::vector<std::unique_ptr<LockOnMarker>> lockOnMarkers_;
    
    //敵
    BaseEnemy* enemy_ = nullptr;
    
    // 視点方向ベクトル
    Vector3 viewDirection_ = { 0.0f, 0.0f, 1.0f }; // デフォルトは前方向
    
    // ロックオン範囲
    float detectionRange_ = 400.0f;      // ロックオン検出距離
    float viewAngleThreshold_ = 0.99f;   // 視点方向判定（約8度）
    const size_t maxLockCount_ = 8;    // 最大ロックオン数
    
    // ロックオンレベルの遷移タイミング
    float timeToSimpleLock_ = 0.0f;      // 簡易ロックオンまでの時間（即時）
    float timeToPreciseLock_ = 2.0f;     // 精密ロックオンまでの時間（秒）
    
    // 視点中心との整列度の計算
    float CalculateViewAlignmentFactor(float dotProduct);
};