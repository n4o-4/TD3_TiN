#include "LockOn.h"
#include "Player.h"
#include <iostream>
#include <cmath> // 数学関数用

///=============================================================================
///                        初期化
void LockOn::Initialize() {
	ModelManager::GetInstance()->LoadModel("lockOn/Lock_on1.obj");	

	lockOnWorldTransform_ = std::make_unique<WorldTransform>();
	lockOnWorldTransform_->Initialize();
	lockOnWorldTransform_->transform.translate = { 0.0f, 0.0f, 0.0f };
	lockOnWorldTransform_->transform.scale = { 3.5f, 3.5f, 3.5f };

	// LockOnMarker 
	lockOnMarkers_.clear();
    
    // 最初に数個のマーカーを用意しておく（必要に応じて増える）
    for (int i = 0; i < 5; ++i) {
        std::unique_ptr<LockOnMarker> marker = std::make_unique<LockOnMarker>();
        marker->Initialize();
        marker->Hide(); // 最初は非表示
        lockOnMarkers_.push_back(std::move(marker));
    }

}

///=============================================================================
///                        更新処理
void LockOn::Update(const std::vector<std::unique_ptr<BaseEnemy>>& enemies) {
    // 既存のデバッグ表示
#ifdef _DEBUG
    ImGui::Begin("LockOn Debug");
    ImGui::Text("Locked Enemies: %d", static_cast<int>(lockedEnemies_.size()));
    ImGui::Text("View Direction: (%.2f, %.2f, %.2f)", viewDirection_.x, viewDirection_.y, viewDirection_.z);
    //それぞれの敵の情報を表示
    for (size_t i = 0; i < lockedEnemiesInfo_.size(); ++i) {
        auto& info = lockedEnemiesInfo_[i];
        ImGui::Text("Enemy %d: Timer %.2f, Level %d", i, info.lockTimer, static_cast<int>(info.lockLevel));
    }
    ImGui::End();
#endif

    DetectEnemies(enemies);
    
    // プレイヤーの位置（ロックオンシステムの位置）を取得
    Vector3 playerPos = lockOnWorldTransform_->transform.translate;
    
    // カメラの視点方向に基づいてメインロックオンを配置
    float distanceInFront = 20.0f;
    
    if (Length(viewDirection_) > 0.001f) {
        // 既存の配置コード
        Vector3 normalizedDirection = Normalize(viewDirection_);
        
        Vector3 lockOnPosition = {
            playerPos.x + normalizedDirection.x * distanceInFront,
            playerPos.y + normalizedDirection.y * distanceInFront,
            playerPos.z + normalizedDirection.z * distanceInFront
        };
        
        lockOnWorldTransform_->transform.translate = lockOnPosition;
        
        Vector3 toCamera = {
            -normalizedDirection.x,
            -normalizedDirection.y,
            -normalizedDirection.z
        };
        
        float yaw = std::atan2(toCamera.x, toCamera.z);
        float pitch = std::atan2(-toCamera.y, std::sqrt(toCamera.x * toCamera.x + toCamera.z * toCamera.z));
        
        lockOnWorldTransform_->transform.rotate = { pitch, yaw, 0.0f };
    }
    
    // 行列を更新
    lockOnWorldTransform_->UpdateMatrix();
    
    // 各ロックオン対象のタイマーと状態を更新
    float deltaTime = 1.0f / 60.0f; // 60FPSを想定
    
    for (size_t i = 0; i < lockedEnemiesInfo_.size(); ++i) {
        auto& info = lockedEnemiesInfo_[i];
        
        // タイマーを更新（視点との整列度に応じてボーナス）
        float alignmentBonus = info.viewAlignmentFactor * 0.5f; // 最大50%のボーナス
        info.lockTimer += deltaTime * (1.0f + alignmentBonus);
        
        // 前回のロックレベルを保存
        LockLevel previousLevel = info.lockLevel;
        
        // ロックオンレベルを更新
        if (info.lockTimer >= timeToSimpleLock_) {
            info.lockLevel = LockLevel::SimpleLock;
        }
        
        if (info.lockTimer >= timeToPreciseLock_) {
            info.lockLevel = LockLevel::PreciseLock;
        }
        
        // ロックレベルが変わった場合、マーカーを更新
        if (previousLevel != info.lockLevel && i < lockOnMarkers_.size()) {
            lockOnMarkers_[i]->SetLockLevel(static_cast<int>(info.lockLevel));
            
            // 精密ロックオン達成時に特別なエフェクト表示
            if (info.lockLevel == LockLevel::PreciseLock && previousLevel != LockLevel::PreciseLock) {
                // ここにエフェクト処理を追加（将来的に）
                // 例：ParticleManager::GetInstance()->Emit("preciselock", info.enemy->GetPosition(), 10);
                
                // マーカーを一度ハイライト表示
                lockOnMarkers_[i]->Show(); // アニメーションをリスタート
            }
        }
        
        // マーカーの位置を更新
        if (i < lockOnMarkers_.size() && info.enemy) {
            Vector3 enemyPos = info.enemy->GetPosition();
            lockOnMarkers_[i]->Update(playerPos, enemyPos);
        }
    }
}
void LockOn::UpdateRaw(const std::vector<BaseEnemy*>& enemies) {
    DetectEnemiesRaw(enemies); 

    Vector3 playerPos = lockOnWorldTransform_->transform.translate;
    float distanceInFront = 20.0f;

    if (Length(viewDirection_) > 0.001f) {
        Vector3 normalizedDirection = Normalize(viewDirection_);
        Vector3 lockOnPosition = {
            playerPos.x + normalizedDirection.x * distanceInFront,
            playerPos.y + normalizedDirection.y * distanceInFront,
            playerPos.z + normalizedDirection.z * distanceInFront
        };

        lockOnWorldTransform_->transform.translate = lockOnPosition;

        Vector3 toCamera = -normalizedDirection;

        float yaw = std::atan2(toCamera.x, toCamera.z);
        float pitch = std::atan2(-toCamera.y, std::sqrt(toCamera.x * toCamera.x + toCamera.z * toCamera.z));
        lockOnWorldTransform_->transform.rotate = { pitch, yaw, 0.0f };
    }

    lockOnWorldTransform_->UpdateMatrix();
    //lockOn_->SetLocalMatrix(MakeIdentity4x4());
    //lockOn_->Update();

    float deltaTime = 1.0f / 60.0f;

    for (size_t i = 0; i < lockedEnemiesInfo_.size(); ++i) {
        auto& info = lockedEnemiesInfo_[i];
        float alignmentBonus = info.viewAlignmentFactor * 0.5f;
        info.lockTimer += deltaTime * (1.0f + alignmentBonus);

        LockLevel previousLevel = info.lockLevel;
        if (info.lockTimer >= timeToSimpleLock_) info.lockLevel = LockLevel::SimpleLock;
        if (info.lockTimer >= timeToPreciseLock_) info.lockLevel = LockLevel::PreciseLock;

        if (previousLevel != info.lockLevel && i < lockOnMarkers_.size()) {
            lockOnMarkers_[i]->SetLockLevel(static_cast<int>(info.lockLevel));
            if (info.lockLevel == LockLevel::PreciseLock) {
                lockOnMarkers_[i]->Show();
            }
        }

        if (i < lockOnMarkers_.size() && info.enemy) {
            Vector3 enemyPos = info.enemy->GetPosition();
            lockOnMarkers_[i]->Update(playerPos, enemyPos);
        }
    }
}
///=============================================================================
///                        描画処理
void LockOn::Draw(ViewProjection viewProjection, DirectionalLight directionalLight, PointLight pointLight, SpotLight spotLight) {
}

///=============================================================================
///                        ロックオン範囲内の敵を検知する関数
void LockOn::DetectEnemies(const std::vector<std::unique_ptr<BaseEnemy>>& enemies) {
    Vector3 lockOnPos = lockOnWorldTransform_->transform.translate;
    if (Length(viewDirection_) < 0.001f) {
        viewDirection_ = {0.0f, 0.0f, 1.0f};
    }
    
    struct EnemyWithPriority {
        BaseEnemy* enemy;
        float priority; // 視点との一致度
        float distance; // プレイヤーからの距離
    };
    std::vector<EnemyWithPriority> prioritizedEnemies;
    
    // 各敵について優先度を計算
    for (const auto& enemy : enemies) {
        Vector3 enemyPos = enemy->GetPosition();
        Vector3 toEnemy = enemyPos - lockOnPos;
        float distanceToEnemy = Length(toEnemy);
        
        if (distanceToEnemy > 0.001f && distanceToEnemy < detectionRange_) {
            Vector3 normalizedToEnemy = {
                toEnemy.x / distanceToEnemy,
                toEnemy.y / distanceToEnemy,
                toEnemy.z / distanceToEnemy
            };
            
            float dotProduct = Dot(viewDirection_, normalizedToEnemy);
            
            if (dotProduct > viewAngleThreshold_) {
                prioritizedEnemies.push_back({
                    enemy.get(), 
                    dotProduct,
                    distanceToEnemy
                });
            }
        }
    }
    
    // 視点との一致度でソート
    std::sort(prioritizedEnemies.begin(), prioritizedEnemies.end(), 
        [](const EnemyWithPriority& a, const EnemyWithPriority& b) {
            if (std::abs(a.priority - b.priority) < 0.01f) {
                return a.distance < b.distance;
            }
            return a.priority > b.priority;
        });
    
    // 現在ロックオン中の敵を一時保存（タイマーを継続するため）
    std::vector<LockedEnemyInfo> previousLockedInfo = lockedEnemiesInfo_;
    lockedEnemiesInfo_.clear();
    
    // マーカー管理用の配列（false=使用していない）
    std::vector<bool> markerUsed(lockOnMarkers_.size(), false);
    
    // ロックオン対象を更新
    for (size_t i = 0; i < std::min(prioritizedEnemies.size(), maxLockCount_); ++i) {
        BaseEnemy* enemy = prioritizedEnemies[i].enemy;
        float viewAlignment = prioritizedEnemies[i].priority;
        
        // 既存のロックオン情報があれば引き継ぐ
        auto it = std::find_if(previousLockedInfo.begin(), previousLockedInfo.end(),
            [enemy](const LockedEnemyInfo& info) { return info.enemy == enemy; });
        
        if (it != previousLockedInfo.end()) {
            // 既存のロックオン情報を更新して追加
            lockedEnemiesInfo_.push_back(*it);
            
            // マーカーの対応するインデックスを正しく設定
            int markerIndex = static_cast<int>(lockedEnemiesInfo_.size() - 1);
            if (markerIndex < static_cast<int>(lockOnMarkers_.size())) {
                lockOnMarkers_[markerIndex]->SetLockLevel(static_cast<int>(it->lockLevel));
                lockOnMarkers_[markerIndex]->Show();
                markerUsed[markerIndex] = true;
            }
        } else {
            // 新しいロックオン情報を作成
            LockedEnemyInfo newInfo;
            newInfo.enemy = enemy;
            newInfo.lockTimer = 0.0f;
            newInfo.lockLevel = LockLevel::None;
            newInfo.viewAlignmentFactor = CalculateViewAlignmentFactor(viewAlignment);
            // 新しい静的位置情報を記録（簡易ロックオン用）
            newInfo.initialPosition = enemy->GetPosition();
            lockedEnemiesInfo_.push_back(newInfo);
            
            // 新しいマーカーを準備
            int markerIndex = static_cast<int>(lockedEnemiesInfo_.size() - 1);
            if (markerIndex < static_cast<int>(lockOnMarkers_.size())) {
                lockOnMarkers_[markerIndex]->SetLockLevel(static_cast<int>(newInfo.lockLevel));
                lockOnMarkers_[markerIndex]->Show();
                markerUsed[markerIndex] = true;
            } else {
                // 必要に応じて新しいマーカーを作成
                std::unique_ptr<LockOnMarker> newMarker = std::make_unique<LockOnMarker>();
                newMarker->Initialize();
                newMarker->SetLockLevel(static_cast<int>(newInfo.lockLevel));
                newMarker->Show();
                lockOnMarkers_.push_back(std::move(newMarker));
                markerUsed.push_back(true);
            }
        }
    }
    
    // 使用されていないマーカーを非表示にする
    for (size_t i = 0; i < lockOnMarkers_.size(); ++i) {
        if (!markerUsed[i]) {
            lockOnMarkers_[i]->Hide();
        }
    }
    
    // ロックオン対象リストを更新
    lockedEnemies_.clear();
    for (const auto& info : lockedEnemiesInfo_) {
        lockedEnemies_.push_back(info.enemy);
    }
}
void LockOn::DetectEnemiesRaw(const std::vector<BaseEnemy*>& enemies) {
    Vector3 lockOnPos = lockOnWorldTransform_->transform.translate;
    if (Length(viewDirection_) < 0.001f) {
        viewDirection_ = { 0.0f, 0.0f, 1.0f };
    }

    struct EnemyWithPriority {
        BaseEnemy* enemy;
        float priority; // 視点との一致度
        float distance; // プレイヤーからの距離
    };
    std::vector<EnemyWithPriority> prioritizedEnemies;

    // 各敵について優先度を計算
    for (const auto& enemy : enemies) {
        Vector3 enemyPos = enemy->GetPosition();
        Vector3 toEnemy = enemyPos - lockOnPos;
        float distanceToEnemy = Length(toEnemy);

        if (distanceToEnemy > 0.001f && distanceToEnemy < detectionRange_) {
            Vector3 normalizedToEnemy = {
                toEnemy.x / distanceToEnemy,
                toEnemy.y / distanceToEnemy,
                toEnemy.z / distanceToEnemy
            };

            float dotProduct = Dot(viewDirection_, normalizedToEnemy);

            if (dotProduct > viewAngleThreshold_) {
                prioritizedEnemies.push_back({
                    enemy,
                    dotProduct,
                    distanceToEnemy
                    });
            }
        }
    }

    // 視点との一致度でソート
    std::sort(prioritizedEnemies.begin(), prioritizedEnemies.end(),
        [](const EnemyWithPriority& a, const EnemyWithPriority& b) {
            if (std::abs(a.priority - b.priority) < 0.01f) {
                return a.distance < b.distance;
            }
            return a.priority > b.priority;
        });

    // 現在ロックオン中の敵を一時保存（タイマーを継続するため）
    std::vector<LockedEnemyInfo> previousLockedInfo = lockedEnemiesInfo_;
    lockedEnemiesInfo_.clear();

    // マーカー管理用の配列（false=使用していない）
    std::vector<bool> markerUsed(lockOnMarkers_.size(), false);

    // ロックオン対象を更新
    for (size_t i = 0; i < std::min(prioritizedEnemies.size(), maxLockCount_); ++i) {
        BaseEnemy* enemy = prioritizedEnemies[i].enemy;
        float viewAlignment = prioritizedEnemies[i].priority;

        // 既存のロックオン情報があれば引き継ぐ
        auto it = std::find_if(previousLockedInfo.begin(), previousLockedInfo.end(),
            [enemy](const LockedEnemyInfo& info) { return info.enemy == enemy; });

        if (it != previousLockedInfo.end()) {
            // 既存のロックオン情報を更新して追加
            lockedEnemiesInfo_.push_back(*it);

            // マーカーの対応するインデックスを正しく設定
            int markerIndex = static_cast<int>(lockedEnemiesInfo_.size() - 1);
            if (markerIndex < static_cast<int>(lockOnMarkers_.size())) {
                lockOnMarkers_[markerIndex]->SetLockLevel(static_cast<int>(it->lockLevel));
                lockOnMarkers_[markerIndex]->Show();
                markerUsed[markerIndex] = true;
            }
        } else {
            // 新しいロックオン情報を作成
            LockedEnemyInfo newInfo;
            newInfo.enemy = enemy;
            newInfo.lockTimer = 0.0f;
            newInfo.lockLevel = LockLevel::None;
            newInfo.viewAlignmentFactor = CalculateViewAlignmentFactor(viewAlignment);
            // 新しい静的位置情報を記録（簡易ロックオン用）
            newInfo.initialPosition = enemy->GetPosition();
            lockedEnemiesInfo_.push_back(newInfo);

            // 新しいマーカーを準備
            int markerIndex = static_cast<int>(lockedEnemiesInfo_.size() - 1);
            if (markerIndex < static_cast<int>(lockOnMarkers_.size())) {
                lockOnMarkers_[markerIndex]->SetLockLevel(static_cast<int>(newInfo.lockLevel));
                lockOnMarkers_[markerIndex]->Show();
                markerUsed[markerIndex] = true;
            } else {
                // 必要に応じて新しいマーカーを作成
                std::unique_ptr<LockOnMarker> newMarker = std::make_unique<LockOnMarker>();
                newMarker->Initialize();
                newMarker->SetLockLevel(static_cast<int>(newInfo.lockLevel));
                newMarker->Show();
                lockOnMarkers_.push_back(std::move(newMarker));
                markerUsed.push_back(true);
            }
        }
    }

    // 使用されていないマーカーを非表示にする
    for (size_t i = 0; i < lockOnMarkers_.size(); ++i) {
        if (!markerUsed[i]) {
            lockOnMarkers_[i]->Hide();
        }
    }

    // ロックオン対象リストを更新
    lockedEnemies_.clear();
    for (const auto& info : lockedEnemiesInfo_) {
        lockedEnemies_.push_back(info.enemy);
    }
}
///=============================================================================
///                        ロックオンされている敵を削除
void LockOn::RemoveLockedEnemy(BaseEnemy* enemy)
{
    // 1. lockedEnemies_ から削除し、その要素を nullptr にする
    for (auto& lockedEnemy : lockedEnemies_) {
        if (lockedEnemy == enemy) {
            lockedEnemy = nullptr;  // enemy に対応する要素を nullptr に設定
            //break;  // 見つけたら処理を終了
        }
    }

    // 2. lockedEnemiesInfo_ から削除し、その enemy を nullptr にする
    for (auto& info : lockedEnemiesInfo_) {
        if (info.enemy == enemy) {
            info.enemy = nullptr;  // info.enemy を nullptr に設定
            //break;  // 見つけたら処理を終了
        }
    }

    // lockedEnemies_ から削除
    auto it = std::remove(lockedEnemies_.begin(), lockedEnemies_.end(), enemy);
    lockedEnemies_.erase(it, lockedEnemies_.end());
    
    // lockedEnemiesInfo_ から削除
    auto infoIt = std::remove_if(lockedEnemiesInfo_.begin(), lockedEnemiesInfo_.end(), 
        [enemy](const LockedEnemyInfo& info) { return info.enemy == enemy; });
    lockedEnemiesInfo_.erase(infoIt, lockedEnemiesInfo_.end());

    // マーカーの表示状態を調整
    for (size_t i = 0; i < lockOnMarkers_.size(); ++i) {
        if (i < lockedEnemiesInfo_.size()) {
            // 表示するマーカーはロックレベルを更新
            lockOnMarkers_[i]->SetLockLevel(static_cast<int>(lockedEnemiesInfo_[i].lockLevel));
            lockOnMarkers_[i]->Show();
        } else {
            // 余分なマーカーは非表示
            lockOnMarkers_[i]->Hide();
        }
    }
}

// 視点中心との整列度の計算
float LockOn::CalculateViewAlignmentFactor(float dotProduct) {
    // dotProductは-1～1の範囲で、1が完全一致
    // 0.9～1.0の範囲を0～1にマッピング（視野角の約25度以内）
    const float threshold = 0.9f;
    if (dotProduct < threshold) {
        return 0.0f;
    }
    
    // 0.9～1.0を0～1.0に正規化
    return (dotProduct - threshold) / (1.0f - threshold);
}

// LockLevelを取得するメソッドを追加
LockOn::LockLevel LockOn::GetLockLevel(BaseEnemy* enemy) const {
    // 指定された敵のロックオン情報を探す
    for (const auto& info : lockedEnemiesInfo_) {
        if (info.enemy == enemy) {
            return info.lockLevel;
        }
    }
    
    // 見つからなければ None を返す
    return LockLevel::None;
}

// ロックオン情報を取得するメソッドを追加
LockOn::LockedEnemyInfo* LockOn::GetLockedEnemyInfo(BaseEnemy* enemy) {
    for (auto& info : lockedEnemiesInfo_) {
        if (info.enemy == enemy) {
            return &info;
        }
    }
    return nullptr;
}

// 特定の敵の追尾位置を取得するメソッド（簡易ロックオンでは初期位置を返す）
Vector3 LockOn::GetLockedEnemyPosition(BaseEnemy* enemy) const {
    for (const auto& info : lockedEnemiesInfo_) {
        if (info.enemy == enemy) {
            // 簡易ロックオンの場合は初期位置を返す
            if (info.lockLevel == LockLevel::SimpleLock) {
                return info.initialPosition;
            }
            // それ以外は現在の敵の位置を返す
            return info.enemy->GetPosition();
        }
    }
    // 見つからなければ敵の現在位置を返す
    return enemy ? enemy->GetPosition() : Vector3{0, 0, 0};
}

