#include "EnemySystem.h"
#include <cassert>
#include <cstdlib>
#include <random>

// コンストラクタ

EnemySystem::EnemySystem() {
}

// デストラクタ
EnemySystem::~EnemySystem() {}

// 初期化（必要であれば）
void EnemySystem::Initialize() {
    
    enemies_.clear();
    spawns_.clear();
    while (!delayedSpawnQueue_.empty()) delayedSpawnQueue_.pop();
    enemyPopCommands.clear();
    currentSpawnType_.clear();
    isWaiting_ = false;
    waitTimer_ = 0;
    waveReady_ = false;
}

// 難易度設定
void EnemySystem::SetDifficulty(bool easy, bool normal, bool hard) {
    easy_ = easy;
    normal_ = normal;
    hard_ = hard;
}

void EnemySystem::ClearAllEnemies() {
    enemies_.clear();
}

// ウェーブデータを読み込む
void EnemySystem::LoadWaveData(int index, std::istringstream data) {
    
    enemyPopCommands.str("");  
    enemyPopCommands.clear();
    enemyPopCommands << data.rdbuf();
    waveReady_ = false;
    
}

// フレーム毎の敵処理
void EnemySystem::Update() {
    int spawnedThisFrame = 0;

    if (!delayedSpawnQueue_.empty()) {
        while (!delayedSpawnQueue_.empty() && spawnedThisFrame < spawnPerFrame_) {
            DelayedSpawnData spawnData = delayedSpawnQueue_.front();
            delayedSpawnQueue_.pop();

            if (spawnData.type == "RC") {
                SpawnEnemyRC(spawnData.position, spawnData.hp);
            } else if (spawnData.type == "Iron") {
                SpawnEnemyIron(spawnData.position, spawnData.hp);
            } else if (spawnData.type == "Re") {
                SpawnEnemyRefrigerator(spawnData.position, spawnData.hp);
            } else if (spawnData.type == "CP") {
                SpawnEnemyCP(spawnData.position, spawnData.hp);
            } else if (spawnData.type == "WM") {
                SpawnEnemyWM(spawnData.position, spawnData.hp);
            }

            spawnedThisFrame++;
        }

       
        ParseEnemyPopCommands();
        return;
    }

    if (isWaiting_) {
        if (--waitTimer_ <= 0) {
            isWaiting_ = false;
        }
        return;
    }

    
    ParseEnemyPopCommands();
}

// 敵出現コマンド解析
void EnemySystem::ParseEnemyPopCommands() {
    std::string line;

    while (getline(enemyPopCommands, line)) {
        std::stringstream line_stream(line);
        std::string word;

        getline(line_stream, word, ',');
        if (word.find("//") == 0) continue;

        if (word == "POP") {
            float x, y, z;
            getline(line_stream, word, ','); x = (float)std::atof(word.c_str());
            getline(line_stream, word, ','); y = (float)std::atof(word.c_str());
            getline(line_stream, word, ','); z = (float)std::atof(word.c_str());

            SpawnSet(Vector3(x, y, z));

            int hp, num;
            getline(line_stream, word, ','); hp = std::stoi(word);
            getline(line_stream, word, ','); num = std::stoi(word);
            getline(line_stream, word, ','); currentSpawnType_ = word;

            std::uniform_real_distribution<float> randX(x - 10.0f, x + 10.0f);
            std::uniform_real_distribution<float> randY(y - 5.0f, y + 5.0f);
            std::uniform_real_distribution<float> randZ(z - 10.0f, z + 10.0f);

            for (int i = 0; i < num; ++i) {
                DelayedSpawnData data;
                data.hp = hp;
                data.type = currentSpawnType_;

                if (currentSpawnType_ == "BAT") {
                    data.position = Vector3(randX(gen), randY(gen), randZ(gen));
                } else {
                    data.position = Vector3(randX(gen), y, randZ(gen));
                }

                delayedSpawnQueue_.push(data);
            }
        } else if (word == "WAIT") {
            getline(line_stream, word, ',');
            waitTimer_ = std::atoi(word.c_str());
            isWaiting_ = true;
            break;
        }
    }

    if (enemyPopCommands.eof()) {
        waveReady_ = true;
    }
}

//敵生成関数
void EnemySystem::SpawnEnemy(const Vector3& pos) {
    auto enemy = std::make_unique<Enemy>();
    enemy->Initialize();
    enemy->SetPosition(pos);
    enemy->SetTarget(player_->GetWorldTransform());
    enemies_.push_back(std::move(enemy));
}

void EnemySystem::SpawnEnemyRC(const Vector3& pos, int hp) {
    std::unique_ptr<BaseEnemy> newEnemy = std::make_unique<GroundTypeEnemy>();
    if (auto* enemyNormal = dynamic_cast<GroundTypeEnemy*>(newEnemy.get())) {
        enemyNormal->Initialize();
        enemyNormal->SetPosition(pos);
        enemyNormal->SetTarget(player_->GetWorldTransform());
        enemyNormal->SetHp(hp);
    }

    enemies_.push_back(std::move(newEnemy));
}

void EnemySystem::SpawnEnemyIron(const Vector3& pos, int hp) {
    auto enemy = std::make_unique<SkyTypeEnemy>();
    enemy->Initialize();
    enemy->SetPosition(pos);
    enemy->SetTarget(player_->GetWorldTransform());
    enemy->SetHp(hp);
    enemies_.push_back(std::move(enemy));
}

void EnemySystem::SpawnEnemyRefrigerator(const Vector3& pos, int hp) {
    auto enemy = std::make_unique<GroundTypeEnemy2>();
    enemy->Initialize();
    enemy->SetPosition(pos);
    enemy->SetTarget(player_->GetWorldTransform());
    enemy->SetHp(hp);
    enemies_.push_back(std::move(enemy));
}

void EnemySystem::SpawnEnemyCP(const Vector3& pos, int hp) {
    auto enemy = std::make_unique<GroundTypeEnemy3>();
    enemy->Initialize();
    enemy->SetPosition(pos);
    enemy->SetTarget(player_->GetWorldTransform());
    enemy->SetHp(hp);
    enemies_.push_back(std::move(enemy));
}

void EnemySystem::SpawnEnemyWM(const Vector3& pos, int hp) {
    auto enemy = std::make_unique<GroundTypeEnemy4>();
    enemy->Initialize();
    enemy->SetPosition(pos);
    enemy->SetTarget(player_->GetWorldTransform());
    enemy->SetHp(hp);
    enemies_.push_back(std::move(enemy));
}

void EnemySystem::SpawnSet(const Vector3& pos) {
    auto spawn = std::make_unique<Spawn>();
    spawn->Initialize();
    spawn->SetPosition(pos);
    spawn->SetTarget(player_->GetWorldTransform());
    spawns_.push_back(std::move(spawn));
}

void EnemySystem::RemoveDeadSpawns(LockOn* lockOn, Player* player) {
    spawns_.erase(
        std::remove_if(spawns_.begin(), spawns_.end(),
            [&](const std::unique_ptr<BaseEnemy>& spawn) {
                if (spawn->GetSpawnHp() <= 0) {
                    if (lockOn) lockOn->RemoveLockedEnemy(spawn.get());
                    for (auto& bullet : player->GetBullets()) {
                        if (bullet->GetTarget() == spawn.get()) bullet->SetTarget(nullptr);
                    }
                    return true;
                }
                return false;
            }),
        spawns_.end());
}

void EnemySystem::RemoveDeadEnemies(LockOn* lockOn, Player* player) {
    enemies_.erase(
        std::remove_if(enemies_.begin(), enemies_.end(),
            [&](const std::unique_ptr<BaseEnemy>& enemy) {
                if (enemy->GetHp() <= 0) {
                    if (lockOn) lockOn->RemoveLockedEnemy(enemy.get());
                    for (auto& bullet : player->GetBullets()) {
                        if (bullet->GetTarget() == enemy.get()) bullet->SetTarget(nullptr);
                    }
                    return true;
                }
                return false;
            }),
        enemies_.end());
}

// 回避処理
void EnemySystem::AvoidOverlap(float avoidRadius) {
    std::vector<BaseEnemy*> all;
    for (auto& e : enemies_) all.push_back(e.get());

    for (auto* self : all) {
        Vector3 avoidance = { 0, 0, 0 };
        for (auto* other : all) {
            if (self == other) continue;
            Vector3 diff = self->GetPosition() - other->GetPosition();
            float dist = Length(diff);
            if (dist < avoidRadius && dist > 0.001f) {
                float weight = (avoidRadius - dist) / avoidRadius;
                avoidance += Normalize(diff) * weight;
            }
        }
        avoidance *= 0.05f;
        self->AddAvoidance(avoidance);

        if (dynamic_cast<GroundTypeEnemy*>(self) || dynamic_cast<GroundTypeEnemy2*>(self)) {
            self->SetVelocityY(0.0f);
        }
    }
}

// 敵リストを返す
const std::vector<std::unique_ptr<BaseEnemy>>& EnemySystem::GetEnemies() const {
    return enemies_;
}

const std::vector<std::unique_ptr<BaseEnemy>>& EnemySystem::GetSpawns() const {
    return spawns_;
}
void EnemySystem::SetPlayer(Player* player) {
    player_ = player;
}

void EnemySystem::SetLockOnSystem(LockOn* lockOnSystem) {
    lockOnSystem_ = lockOnSystem;
}

