#pragma once  
#include "BaseScene.h"  
#include "Kouro.h"  
#include "SkyDome.h"  
#include "Ground.h"  
#include "Enemy.h"  
#include <sstream>  
#include "Player.h"
#include "FollowCamera.h"
#include "LockOn.h"

class PakuScene : public BaseScene  
{  
public: // メンバ関数  
   // 初期化  
   void Initialize() override;  
   // 終了  
   void Finalize() override;  
   // 毎フレーム更新  
   void Update() override;  
   // 描画  
   void Draw() override;  
   void LoadEnemyPopData();  
   void UpdateEnemyPopCommands();  
   void SpawnEnemy(const Vector3& position);

private:  
   std::unique_ptr<SkyDome> skyDome_ = nullptr;  
   std::unique_ptr<Ground> ground_ = nullptr;  
   std::unique_ptr<Enemy> enemy_ = nullptr;  
   std::unique_ptr<Camera> camera = nullptr;  
   std::unique_ptr<DirectionalLight> directionalLight = nullptr;  
   std::unique_ptr<PointLight> pointLight = nullptr;  
   std::unique_ptr<SpotLight> spotLight = nullptr;  
  
   /// <summary>
   /// Eneny 出現
   /// </summary>
   std::stringstream enemyPopCommands;

   std::vector<std::unique_ptr<Enemy>> enemies_;

   bool isWaiting_ = false;
   int32_t waitTimer_ = 0;

   // プレイヤー
   std::unique_ptr<Player> player_ = nullptr;
   // 追従カメラ
   std::unique_ptr<FollowCamera> followCamera_ = nullptr;

   //LockOn
 
   std::unique_ptr<LockOn> lockOn_ = nullptr;

};
