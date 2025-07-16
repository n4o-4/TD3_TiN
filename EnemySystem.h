#pragma once
#include <vector>
#include <queue>
#include <string>
#include <sstream>
#include <memory>
#include <random>
#include <fstream>

#include "BaseEnemy.h"
#include "Spawn.h"
#include "Player.h"
#include "GroundTypeEnemy.h"
#include "GroundTypeEnemy2.h"
#include "GroundTypeEnemy3.h"
#include "GroundTypeEnemy4.h"
#include "SkyTypeEnemy.h"
#include "LockOn.h"

// フレーム分散召喚用の新しいメンバ変数
struct DelayedSpawnData {
    Vector3 position;
    int hp;
    std::string type;
};

class EnemySystem {
public:
    // コンストラクタ・デストラクタ
    EnemySystem();
    ~EnemySystem();

    // 初期化
    void Initialize();

    // ウェーブCSVデータを読み込む
    void LoadWaveData(int index, std::istringstream data);

    // 敵の出現処理（毎フレーム）
    void Update();

    // 全敵リスト取得（GameSceneと連携用）
    const std::vector<std::unique_ptr<BaseEnemy>>& GetEnemies() const;
    const std::vector<std::unique_ptr<BaseEnemy>>& GetSpawns() const;

    // 敵回避処理
    void AvoidOverlap(float avoidRadius);

    // 難易度設定
    void SetDifficulty(bool easy, bool normal, bool hard);

    void RemoveDeadSpawns(LockOn* lockOn, Player* player);
    void RemoveDeadEnemies(LockOn* lockOn, Player* player);
    bool IsWaveReady() const { return waveReady_; }
    void SetPlayer(Player* player);               
    void SetLockOnSystem(LockOn* lockOnSystem);   
private:
    // 敵生成関数群
    void SpawnEnemy(const Vector3& pos);
    void SpawnEnemyRC(const Vector3& pos, int hp);
    void SpawnEnemyIron(const Vector3& pos, int hp);
    void SpawnEnemyRefrigerator(const Vector3& pos, int hp);
    void SpawnEnemyChair(const Vector3& pos, int hp);
    void SpawnEnemyWM(const Vector3& pos, int hp);
    void SpawnSet(const Vector3& pos);


    // 出現指令を解析する
    void ParseEnemyPopCommands();

    // データメンバー
    Player* player_ = nullptr;
    LockOn* lockOnSystem_ = nullptr;

    std::vector<std::unique_ptr<BaseEnemy>> enemies_;
    std::vector<std::unique_ptr<BaseEnemy>> spawns_;

    std::queue<DelayedSpawnData> delayedSpawnQueue_;
    std::stringstream enemyPopCommands;

    std::string currentSpawnType_;
    bool isWaiting_ = false;
    int waitTimer_ = 0;
    bool waveReady_ = false;

    int spawnPerFrame_ = 5;

    std::vector<std::string> waveCsvPaths_;
    bool easy_ = false, normal_ = false, hard_ = false;

    std::mt19937 gen;
};

