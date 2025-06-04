#define _USE_MATH_DEFINES
#define NOMINMAX
#include <algorithm>
#include "Player.h"
#include <cmath>
#include "MyMath.h"
#include "imgui.h"
#include "BaseEnemy.h"
#include "EnemyBullet.h"
#include "Vectors.h"
#include "SkyTypeEnemy.h"
#include "GroundTypeEnemy.h"
#include "GroundTypeEnemy2.h"

void Player::Initialize() {
	// Object3d を初期化
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(Object3dCommon::GetInstance());
	
	door_ = std::make_unique<Object3d>();
	door_->Initialize(Object3dCommon::GetInstance());

	// マシンガンBody
	machineGunBody_ = std::make_unique<Object3d>();
	machineGunBody_->Initialize(Object3dCommon::GetInstance());
	// マシンガンHead
	machineGunHead_ = std::make_unique<Object3d>();
	machineGunHead_->Initialize(Object3dCommon::GetInstance());

	// モデルを設定
	ModelManager::GetInstance()->LoadModel("player/Microwave_body.obj");
	object3d_->SetModel("player/Microwave_body.obj");
	// モデルのスケールを設定
	ModelManager::GetInstance()->LoadModel("player/Microwave_door.obj");
	door_->SetModel("player/Microwave_door.obj");
	// モデルのスケールを設定
	ModelManager::GetInstance()->LoadModel("mg_body.obj");
	machineGunBody_->SetModel("mg_body.obj");
	// モデルのスケールを設定
	ModelManager::GetInstance()->LoadModel("mg_head.obj");
	machineGunHead_->SetModel("mg_head.obj");

	// 初期位置を設定
	objectTransform_ = std::make_unique<WorldTransform>();
	objectTransform_->Initialize();
	objectTransform_->transform.translate = { 0.0f, initialY_ , kInitialZ_ };
	

	doorObjectTransform_ = std::make_unique<WorldTransform>();
	doorObjectTransform_->Initialize();
	doorObjectTransform_->transform.translate = { 1.36f, -0.01f , 0.94f };
	//doorObjectTransform_->transform.scale = Vector3(2.0f, 2.0f, 2.0f);

	// マシンガンBody
	machineGunBodyTransform_ = std::make_unique<WorldTransform>();
	machineGunBodyTransform_->Initialize();
	machineGunBodyTransform_->transform.translate = { 0.0f, 0.0f , 0.0f };
	// マシンガンHead
	machineGunHeadTransform_ = std::make_unique<WorldTransform>();
	machineGunHeadTransform_->Initialize();
	machineGunHeadTransform_->transform.translate = { -2.2f, 1.31f , 4.49f };

	doorObjectTransform_->SetParent(objectTransform_.get());
	machineGunBodyTransform_->SetParent(objectTransform_.get());
	machineGunHeadTransform_->SetParent(machineGunBodyTransform_.get());
	
	ParticleManager::GetInstance()->CreateParticleGroup("missileSmoke", "Resources/circle.png", ParticleManager::ParticleType::Normal);

	ParticleManager::GetInstance()->CreateParticleGroup("OverHeat", "Resources/circle.png", ParticleManager::ParticleType::Normal);

	ParticleManager::GetInstance()->GetParticleGroup("OverHeat")->enableBillboard = true;

	explosionEmitter_ = std::make_unique<ParticleEmitter>();
	explosionEmitter_->Initialize("missileSmoke");
	explosionEmitter_->SetParticleCount(kExplosionParticleCount_);
	explosionEmitter_->SetFrequency(kExplosionFrequency_);
	explosionEmitter_->SetLifeTimeRange({1.0f,1.0f});

	smokeEmitter_ = std::make_unique<ParticleEmitter>();
	smokeEmitter_->Initialize("OverHeat");
	smokeEmitter_->SetParticleCount(4);
	smokeEmitter_->SetFrequency(0.1f);
	smokeEmitter_->SetLifeTimeRange( {0.3f, 0.5f} );
	smokeEmitter_->SetStartColorRange(
		ParticleManager::ColorRange(
			{1.0f,1.0f,1.0f,0.3f},
			{1.0f,1.0f,1.0f,0.5f}
		)
	);
	smokeEmitter_->SetFinishColorRange(
		ParticleManager::ColorRange(
			{ 0.6f,0.6f,0.6f,0.0f },
			{ 0.8f,0.8f,0.8f,0.3f }
		)
	);
	smokeEmitter_->SetVelocityRange(
		ParticleManager::Vec3Range(
			{ 0.0f,0.0f,-1.0f },
			{ 0.0f,6.0f,1.0f }
		)
	);

	BaseObject::Initialize(objectTransform_->transform.translate, 1.0f);

	AudioManager::GetInstance()->SoundLoadFile("Resources/se/missile.mp3");
	AudioManager::GetInstance()->SoundLoadFile("Resources/se/高速移動.mp3");
	AudioManager::GetInstance()->SoundLoadFile("Resources/se/999DCD4E5D2A205C05.mp3");

	se1_ = std::make_unique<Audio>();
	se2_ = std::make_unique<Audio>();
	se3_ = std::make_unique<Audio>();
	se1_->Initialize();
	se2_->Initialize();
	se3_->Initialize();
}
///=============================================================================
///                        更新処理
void Player::Update() {
	// 移動入力の取得
	Vector3 inputDirection = GetMovementInput();

	// ブースト処理
	HandleBoost();

	// 移動処理
	UpdateMove(inputDirection);

	// 反動の適用
	ApplyRecoil();

	// 揺れの適用
	ApplyShake();

	// 弾の処理
	UpdateBullets();

	// ジャンプ処理
	UpdateJump();

	// playerの揺れ処理
	ApplyFloating();

	// ドア
	OpenDoor();

	// 無敵時間の処理
	if (isInvincible_) {
		invincibleTimer_--;

		// 点滅（フレームごとに表示・非表示切り替え）
		if (invincibleTimer_ % kInvincibleBlinkInterval_ == 0) { // 10フレームごとに切り替え
			isVisible_ = !isVisible_;
		}

		// 無敵時間終了
		if (invincibleTimer_ <= 0) {
			isInvincible_ = false;
			isVisible_ = true;
		}
	}

	objectTransform_->UpdateMatrix();// 行列更新
	object3d_->SetLocalMatrix(MakeIdentity4x4());// ローカル行列を単位行列に
	object3d_->Update();// 更新

	//ドア
	doorObjectTransform_->UpdateMatrix();// 行列更新
	door_->SetLocalMatrix(MakeIdentity4x4());// ローカル行列を単位行列に
	door_->Update();

	// マシンガンの更新
	machineGunBodyTransform_->UpdateMatrix();// 行列更新
	machineGunBody_->SetLocalMatrix(MakeIdentity4x4());// ローカル行列を単位行列に
	machineGunBody_->Update();// 更新
	machineGunHeadTransform_->UpdateMatrix();// 行列更新
	machineGunHead_->SetLocalMatrix(MakeIdentity4x4());// ローカル行列を単位行列に
	machineGunHead_->Update();// 更新

	//========================================
	// 当たり判定との同期
	BaseObject::Update(objectTransform_->transform.translate);
	// ImGui描画

	DrawImGui();

	CameraShake();
}
///=============================================================================
///                        描画
void Player::Draw(ViewProjection viewProjection, DirectionalLight directionalLight, PointLight pointLight, SpotLight spotLight) {
	// 無敵時間中は点滅
	if (!isInvincible_ || isVisible_) {
		object3d_->Draw(*objectTransform_.get(), viewProjection, directionalLight, pointLight, spotLight);
		door_->Draw(*doorObjectTransform_.get(), viewProjection, directionalLight, pointLight, spotLight);
		machineGunBody_->Draw(*machineGunBodyTransform_.get(), viewProjection, directionalLight, pointLight, spotLight);
		machineGunHead_->Draw(*machineGunHeadTransform_.get(), viewProjection, directionalLight, pointLight, spotLight);
	}

	// 弾の描画
	for (auto& bullet : bullets_) {
		bullet->Draw(viewProjection, directionalLight, pointLight, spotLight);
	}

	//マシンガンの描画
	for (auto& bullet : machineGunBullets_) {
		bullet->Draw(viewProjection, directionalLight, pointLight, spotLight);
	}
}
///=============================================================================
///                        終了処理
void Player::Finalize() {
	lockOnSystem_ = nullptr;
}
///=============================================================================
///                        Imgui描画
void Player::DrawImGui() {

#ifdef _DEBUG

	ImGui::Begin("Player Status");

	Vector3 pos = objectTransform_->transform.translate;
	Vector3 rot = objectTransform_->transform.rotate;
	Vector3 vel = velocity_;
	ImGui::Text("HP: %d", hp_);
	ImGui::Text("Position: (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
	ImGui::Text("Rotation: (%.2f, %.2f, %.2f)", rot.x, rot.y, rot.z);
	ImGui::Text("Velocity: (%.2f, %.2f, %.2f)", vel.x, vel.y, vel.z);

	ImGui::Text("Speed: %.2f", Length(vel));
	ImGui::Text("Boost: %.1f / %.1f", currentBoostTime_, maxBoostTime_);
	ImGui::Text("Is Jumping: %s", isJumping_ ? "Yes" : "No");
	ImGui::Text("Is Boosting: %s", isBoosting_ ? "Yes" : "No");
	ImGui::Text("In Vincible: %s", isInvincible_ ? "Yes" : "No");

	ImGui::Text("QuickBoost Used: %d / %d", quickBoostUsedCount_, maxQuickBoostUses_);
	ImGui::Text("QuickBoost Cooldown: %d", quickBoostChargeCooldown_);

	ImGui::Text("Missile Cooldown: %d", missileCooldown_);

	ImGui::Text("Machine Gun Heat: %.1f / %.1f", heatLevel_, maxHeat_);
	ImGui::ProgressBar(heatLevel_ / maxHeat_, ImVec2(0.0f, 0.0f));
	if (isOverheated_) {
		ImGui::TextColored(ImVec4(1, 0, 0, 1), "OVERHEATED!");
	}
	//マシンガンの情報


	ImGui::DragFloat3("door Translate", &doorObjectTransform_->transform.translate.x, 0.01f);
	ImGui::DragFloat3("MG_BODY Translate", &machineGunHeadTransform_->transform.translate.x, 0.01f);

	static Vector3 prevPos;

	Vector3 diff = objectTransform_->transform.translate - prevPos;

	float length = Length(diff);

	ImGui::Text("Distance Moved: %.2f", length);

	// アナログスティック入力を合成
	Vector2 stickInput = Input::GetInstance()->GetLeftStick();

	
	ImGui::Text("X : %.2f :: Y : %.2f", stickInput.x, stickInput.y);

	prevPos = objectTransform_->transform.translate;

	ImGui::End();

	// 弾の情報も表示
	ImGui::Begin("Player Bullets");
	int index = 0;
	for (const auto& bullet : bullets_) {
		if (bullet->IsActive()) {
			Vector3 pos = bullet->GetCollider()->GetPosition();
			ImGui::Text("Bullet %d: (%.2f, %.2f, %.2f)", index, pos.x, pos.y, pos.z);
		}
		index++;
	}
	ImGui::End();

#endif
}
///=============================================================================
///                        静的メンバ関数
///--------------------------------------------------------------
///                        移動入力の取得
Vector3 Player::GetMovementInput() {
    Vector3 rawInputDirection = { 0.0f, 0.0f, 0.0f };
    // キーボード入力
    if (Input::GetInstance()->PushKey(DIK_W)) { rawInputDirection.z += 1.0f; }
    if (Input::GetInstance()->PushKey(DIK_S)) { rawInputDirection.z -= 1.0f; }
    if (Input::GetInstance()->PushKey(DIK_A)) { rawInputDirection.x -= 1.0f; }
    if (Input::GetInstance()->PushKey(DIK_D)) { rawInputDirection.x += 1.0f; }

    // アナログスティック入力を合成
    Vector2 stickInput = Input::GetInstance()->GetLeftStick();
    rawInputDirection.x += stickInput.x;
    rawInputDirection.z += stickInput.y; // スティックのY軸をZ軸移動にマッピング

    // 入力の大きさを計算し、メンバ変数に保存 (0.0 ~ 1.0 にクランプ)
    currentInputMagnitude_ = Length(rawInputDirection);
    if (currentInputMagnitude_ > 1.0f) {
        currentInputMagnitude_ = 1.0f;
    } else if (currentInputMagnitude_ < kVelocityStopThreshold_) { // ごくわずかな入力は0として扱う
        currentInputMagnitude_ = 0.0f;
    }
    
    Vector3 worldDirection = { 0.0f, 0.0f, 0.0f };
    if (currentInputMagnitude_ > 0.0f) { // 入力がある場合のみ方向を正規化
        worldDirection = Normalize(rawInputDirection);
    }

    // カメラの向きに合わせて入力方向を回転
    if (followCamera_) {
        Matrix4x4 rotateMatrix = MakeRotateMatrix(followCamera_->GetViewProjection().transform.rotate);
        worldDirection = TransformNormal(worldDirection, rotateMatrix);
    }
    worldDirection.y = 0.0f; // 上下方向の移動は無効化

    return worldDirection; // 正規化されたワールド空間での目標移動方向
}
///--------------------------------------------------------------
///                        移動
void Player::UpdateMove(Vector3 direction) { // direction は正規化された目標移動方向
    // 入力方向と大きさに応じて加速度を決定
    float currentAccelerationRate = accelerationRate_;
    if (isBoosting_ && !isQuickBoosting_) { // 通常ブースト中は加速度もアップ
        currentAccelerationRate *= boostAccelerationFactor_;
    }

    if (currentInputMagnitude_ > kVelocityStopThreshold_) {
        float controlFactor = isJumping_ ? airControlFactor_ : 1.0f;
        acceleration_ = direction * (currentAccelerationRate * controlFactor * currentInputMagnitude_);
    } else {
        acceleration_ = { 0.0f, 0.0f, 0.0f };
    }

    velocity_ = velocity_ + acceleration_;

    float targetSpeedLimit;
    if (isQuickBoosting_) {
        targetSpeedLimit = maxSpeed_ * kQuickBoostSpeedMultiplier_;
    } else if (isBoosting_) {
        // 通常ブースト時は、基本のブースト速度と、入力の大きさを考慮した速度の大きい方を採用し、
        // さらに限界突破用の最大速度を設定
        float baseBoostSpeed = maxSpeed_ * boostFactor_;
        float inputScaledBoostSpeed = maxSpeed_ * currentInputMagnitude_ * boostFactor_; // スティック入力も少し影響
        targetSpeedLimit = std::max(baseBoostSpeed, inputScaledBoostSpeed);
        targetSpeedLimit = std::min(targetSpeedLimit, maxSpeed_ * boostMaxSpeedFactor_); // 限界突破上限
    } else {
        targetSpeedLimit = maxSpeed_ * currentInputMagnitude_;
    }

    float currentSpeed = Length(velocity_);

    if (currentSpeed > targetSpeedLimit) {
        if (isQuickBoosting_ || isBoosting_) {
            velocity_ = Normalize(velocity_) * targetSpeedLimit;
        } else {
            float slowedSpeed = currentSpeed * kPostBoostSlowdownRate_;
            velocity_ = Normalize(velocity_) * std::max(targetSpeedLimit, slowedSpeed);
        }
    }

    // 移動が入力されているときだけ向きを変更
    if (Length(velocity_) > kVelocityStopThreshold_) {
        distinationRotateY_ = std::atan2(velocity_.x, velocity_.z);
    }

    objectTransform_->transform.rotate.y = LerpShortAngle(objectTransform_->transform.rotate.y, distinationRotateY_, kRotationLerpFactor_);

    // 高速移動中は進行方向に傾ける
    float targetTilt = 0.0f;
    if (isQuickBoosting_ || isBoosting_) {
        // 機体のローカルX軸（横移動）成分で傾ける
        Matrix4x4 invRotateMatrix = Inverse(MakeRotateYMatrix(objectTransform_->transform.rotate.y));
        Vector3 localVelocity = TransformNormal(velocity_, invRotateMatrix);
        targetTilt = localVelocity.x * kMovementTiltFactor_;
        targetTilt = std::clamp(targetTilt, -kMovementMaxTilt_, kMovementMaxTilt_);
    }
    objectTransform_->transform.rotate.z = fLerp(objectTransform_->transform.rotate.z, targetTilt, kMovementTiltLerpFactor_);

    objectTransform_->transform.translate += velocity_;

    // 移動制限
    objectTransform_->transform.translate.x = std::clamp(objectTransform_->transform.translate.x, -495.0f, 495.0f);
    objectTransform_->transform.translate.z = std::clamp(objectTransform_->transform.translate.z, -495.0f, 495.0f);
}
///--------------------------------------------------------------
///                        ジャンプ
void Player::UpdateJump() {
    if (!isJumping_) {
        // Aボタンまたはスペースキーでジャンプ開始
        if (Input::GetInstance()->TriggerGamePadButton(Input::GamePadButton::A) || // ゲームパッド Aボタン
            Input::GetInstance()->Triggerkey(DIK_SPACE)) {                         // キーボード スペースキー
            isJumping_ = true;
            jumpVelocity_ = kJumpInitialVelocity_;
            fallSpeed_ = 0.0f;
        }
    }

    if (isJumping_) {
        objectTransform_->transform.translate.y += jumpVelocity_;
        jumpVelocity_ -= kJumpVelocityDecay_;

        if (jumpVelocity_ <= 0.0f) {
            jumpVelocity_ = 0.0f;
            fallSpeed_ += gravity_;
            if (fallSpeed_ > maxFallSpeed_) {
                fallSpeed_ = maxFallSpeed_;
            }
            objectTransform_->transform.translate.y -= fallSpeed_;
        }

        if (objectTransform_->transform.translate.y <= initialY_) {
            objectTransform_->transform.translate.y = initialY_;
            isJumping_ = false;
            jumpVelocity_ = 0.0f;
            fallSpeed_ = 0.0f;
        }
    }
}
///--------------------------------------------------------------
///						 弾の処理と更新
void Player::UpdateBullets() {
	UpdateMissiles();
	UpdateMachineGunAndHeat();
}

void Player::UpdateMissiles() {
	if (missileCooldown_ > 0) {
		missileCooldown_--;
	}

	// 発射入力かつクールタイム終了してたら撃つ
	if (missileCooldown_ <= 0 &&
		(Input::GetInstance()->Triggerkey(DIK_RETURN) ||
			Input::GetInstance()->TriggerGamePadButton(Input::GamePadButton::RIGHT_SHOULDER))) {
		se1_->SoundPlay("Resources/se/missile.mp3", 0);
		se1_->SetVolume(0.5f); 
		Shoot();
		missileCooldown_ = missileCooldownMax_; // クールタイム開始

		// ドア開くフラグをオンにする
		isDoorOpening_ = true;
		isDoorClosing_ = false; // 強制開き直しにも対応
	
	}

	// 弾の更新
	for (auto& bullet : bullets_) {
		bullet->Update();
	}
	// 画面外に出た弾を削除
	bullets_.erase(std::remove_if(bullets_.begin(), bullets_.end(),
		[](const std::unique_ptr<PlayerMissile>& bullet) { return !bullet->IsActive(); }),
		bullets_.end());
}

void Player::UpdateMachineGunAndHeat() {

	// マシンガンの回転速度（通常時とオーバーヒート時）
	const float kMachineGunRotateSpeed_Normal = 0.25f;
	const float kMachineGunRotateSpeed_Overheat = 0.05f;

	//マシンガンの弾の更新
	 // マシンガンの発射
	if (Input::GetInstance()->PushKey(DIK_J) ||
		Input::GetInstance()->PushGamePadButton(Input::GamePadButton::LEFT_SHOULDER)) {
		

		// 状態に応じて回転速度を変更
		float rotateSpeed = isOverheated_ ? kMachineGunRotateSpeed_Overheat : kMachineGunRotateSpeed_Normal;
		machineGunHeadTransform_->transform.rotate.z += rotateSpeed;

		isShootingMachineGun_ = true;
	} else {
		isShootingMachineGun_ = false;
	}

	if (machineGunCooldown_ > 0) {
		machineGunCooldown_--;
	}

	// マシンガン弾の更新
	for (auto& bullet : machineGunBullets_) {
		bullet->Update();
	}

	// 不要な弾の削除
	machineGunBullets_.erase(std::remove_if(machineGunBullets_.begin(), machineGunBullets_.end(),
		[](const std::unique_ptr<PlayerMachineGun>& bullet) { return !bullet->IsActive(); }),
		machineGunBullets_.end());

	// 発射入力
	if (isShootingMachineGun_ && !isOverheated_ && machineGunCooldown_ <= 0) {
		
		ShootMachineGun();
		if (!machineGunSoundPlayed_) {
			se3_->SoundPlay("Resources/se/999DCD4E5D2A205C05.mp3", 9999);
			se3_->SetVolume(0.5f);
			machineGunSoundPlayed_ = true;
		}
		heatLevel_ += heatPerShot_;
		if (heatLevel_ >= maxHeat_) {
			isOverheated_ = true;
			overheatTimer_ = overheatRecoveryTime_;
		}
		machineGunCooldown_ = kMachineGunFireInterval_;
	}

	if (!isShootingMachineGun_) {
		machineGunSoundPlayed_ = false;
		se3_->SoundStop("Resources/se/999DCD4E5D2A205C05.mp3");
	}
	
	// 自然冷却 or クールダウン処理
	if (isOverheated_) {
		se3_->SoundStop("Resources/se/999DCD4E5D2A205C05.mp3");
		overheatTimer_--;
		if (overheatTimer_ <= 0) {
			isOverheated_ = false;
			heatLevel_ = 0.0f;
		}
		// 煙を出す
		if (smokeEmitter_) {

			Vector3 smokePosition = machineGunHeadTransform_->GetWorldPosition();
			smokeEmitter_->SetPosition(smokePosition);
			smokeEmitter_->Update();  // 一定間隔でEmitしてくれる
		}
	} else {
		heatLevel_ = std::max(0.0f, heatLevel_ - heatCooldownRate_);
	}
}
///                        射撃
void Player::Shoot() {
	Vector3 bulletPos = objectTransform_->transform.translate;
	Vector3 bulletScale = { 0.5f, 0.5f, 0.5f };
	Vector3 bulletRotate = { 0.0f, 0.0f, 0.0f };

	if (lockOnSystem_ && lockOnSystem_->GetLockedEnemyCount() > 0) {
		for (BaseEnemy* enemy : lockOnSystem_->GetLockedEnemies()) {
			if (!enemy) continue;

			// 敵ごとのロックオンレベルを取得
			LockOn::LockLevel lockLevel = lockOnSystem_->GetLockLevel(enemy);
			int lockLevelValue = static_cast<int>(lockLevel);  // 数値に変換

			Vector3 enemyPos = enemy->GetPosition();
			Vector3 direction = Normalize(enemyPos - bulletPos);

			// ロックオンレベルに応じて初速と挙動を変更
			Vector3 initialVelocity;
			if (lockLevel == LockOn::LockLevel::PreciseLock) {
				// 精密ロックオン：より直線的な初速
				initialVelocity = Normalize((direction * 0.7f) + Vector3{ 0.0f, 0.3f, 0.0f });
				initialVelocity = initialVelocity * 0.35f;  // 初速は少し速く
			} else {
				// 簡易ロックオン：上向きに弧を描く初速
				initialVelocity = Normalize((direction * 0.3f) + Vector3{ 0.0f, 0.7f, 0.0f });
				initialVelocity = initialVelocity * 0.25f;  // 初速は少し遅め
			}

			// 新しい弾を作成してターゲットを設定
			auto newBullet = std::make_unique<PlayerMissile>(
				bulletPos, initialVelocity, bulletScale, bulletRotate, lockLevelValue);
			newBullet->SetTarget(enemy); // ターゲットを設定
			bullets_.push_back(std::move(newBullet));

			// 発射エフェクト（ロックオンレベルに応じて調整）
			if (lockLevel == LockOn::LockLevel::PreciseLock) {
				// TODO:精密ロックオンのエフェクト（より派手に）
				//ParticleManager::GetInstance()->Emit("missileTrail", bulletPos, 10);
			} else {
				// TODO:簡易ロックオンのエフェクト（控えめに）
				//ParticleManager::GetInstance()->Emit("missileTrail", bulletPos, 5);
			}
		}
	} else {
		// ロックオンがない場合は上方向に発射（通常ミサイル）
		Vector3 bulletVelocity = { 0.0f, 0.5f, 0.0f };
		bullets_.push_back(std::make_unique<PlayerMissile>(bulletPos, bulletVelocity, bulletScale, bulletRotate, 0));
	}

	// 発射時の反動や効果音はロックオンレベルに関わらず共通
	// ...既存コード...
}

///--------------------------------------------------------------
///                        ブースト処理
bool Player::HandleBoost() {
    UpdateQuickBoostCooldowns();

    // クイックブーストの入力判定と実行試行
    bool quickBoostTriggered = Input::GetInstance()->Triggerkey(DIK_LSHIFT) ||
                               Input::GetInstance()->TriggerGamePadButton(Input::GamePadButton::X);
    bool quickBoostActivatedThisFrame = false;

    if (quickBoostTriggered) {
        quickBoostActivatedThisFrame = HandleQuickBoostActivation();
		if (quickBoostActivatedThisFrame) {
			se2_->SoundPlay("Resources/se/高速移動.mp3", 0);  
			se2_->SetVolume(0.5f);
		}
    }

    if (ProcessActiveQuickBoost() || quickBoostActivatedThisFrame) {
        // クイックブーストが実行中、またはこのフレームで起動した場合
        isBoosting_ = false; // 通常ブーストはオフ
        RecoverBoostEnergy(); // エネルギー回復は試行
        return true; // クイックブーストが処理された
    }

    // 通常ブーストの処理 (クイックブーストが実行されなかった場合)
    bool normalBoostHoldInput = Input::GetInstance()->PushKey(DIK_LSHIFT) ||
                                Input::GetInstance()->PushGamePadButton(Input::GamePadButton::X);

    if (normalBoostHoldInput && currentBoostTime_ > 0.0f && !isQuickBoosting_) {
        isBoosting_ = true;
        currentBoostTime_ -= boostEnergyConsumptionRate_;
        if (currentBoostTime_ < 0.0f) {
            currentBoostTime_ = 0.0f;
            isBoosting_ = false;
        }
    } else {
        isBoosting_ = false;
    }
    
    RecoverBoostEnergy();
    return false; // クイックブーストはこの関数呼び出しでは起動しなかった
}

void Player::UpdateQuickBoostCooldowns() {
	if (quickBoostCooldown_ > 0.0f) {
		quickBoostCooldown_ -= 1.0f; // フレームベースのクールダウン
	}

	if (quickBoostChargeCooldown_ > 0) {
		quickBoostChargeCooldown_--;
		if (quickBoostChargeCooldown_ == 0) {
			quickBoostUsedCount_ = 0;
		}
		quickBoostRegenTimer_ = 0;
	} else if (quickBoostUsedCount_ > 0) {
		quickBoostRegenTimer_++;
		if (quickBoostRegenTimer_ >= quickBoostRegenInterval_) {
			quickBoostUsedCount_--;
			if (quickBoostUsedCount_ < 0) quickBoostUsedCount_ = 0;
			quickBoostRegenTimer_ = 0;
		}
	} else {
		quickBoostRegenTimer_ = 0;
	}
}

bool Player::ProcessActiveQuickBoost() {
	if (!isQuickBoosting_) {
		return false;
	}

	quickBoostFrames_--;

	if (Length(velocity_) > kVelocityStopThreshold_) { // わずかでも速度があれば方向を取る
		Vector3 boostDirection = Normalize(velocity_);
		boostDirection.y = 0.0f;

		float boostSpeed = maxSpeed_ * kQuickBoostSpeedMultiplier_; // 定数使用

		// ブースト後半で速度減衰
		if (quickBoostFrames_ < maxQuickBoostFrames_ / 2) {
			float ratio = static_cast<float>(quickBoostFrames_) / (maxQuickBoostFrames_ / 2.0f); // 2.0fで浮動小数点除算
			boostSpeed *= (0.7f + 0.3f * ratio); // この係数も定数化可能
		}
		velocity_ = boostDirection * boostSpeed;
	}

	if (quickBoostFrames_ <= 0) {
		isQuickBoosting_ = false;
		velocity_ *= kQuickBoostSpeedRetainFactor_; // 定数使用
	}
	return true; // クイックブースト処理を行った
}

bool Player::HandleQuickBoostActivation() {
    // この関数は HandleBoost から quickBoostTriggered が true の場合に呼び出される想定
    // なので、ここでの入力再チェックは不要だが、安全のために残しても良い
    // bool boostInput = Input::GetInstance()->Triggerkey(DIK_LSHIFT) ||
    //                   Input::GetInstance()->TriggerGamePadButton(Input::GamePadButton::X);

    if (quickBoostCooldown_ <= 0.0f &&
        currentBoostTime_ >= quickBoostConsumption_ &&
        quickBoostUsedCount_ < maxQuickBoostUses_ &&
        !isQuickBoosting_) { // isQuickBoosting_ のチェックは ProcessActiveQuickBoost で行われるため、ここでは不要かもしれないが念のため

        Vector3 rawBoostDirection = {0.0f, 0.0f, 0.0f};
        // ... (rawBoostDirection と worldBoostDirection の決定ロジックは変更なし) ...
        if (Input::GetInstance()->PushKey(DIK_W)) { rawBoostDirection.z += 1.0f; }
        if (Input::GetInstance()->PushKey(DIK_S)) { rawBoostDirection.z -= 1.0f; }
        if (Input::GetInstance()->PushKey(DIK_A)) { rawBoostDirection.x -= 1.0f; }
        if (Input::GetInstance()->PushKey(DIK_D)) { rawBoostDirection.x += 1.0f; }
        Vector2 stickInput = Input::GetInstance()->GetLeftStick();
        rawBoostDirection.x += stickInput.x;
        rawBoostDirection.z += stickInput.y;

        Vector3 worldBoostDirection = {0.0f, 0.0f, 0.0f};
        if (Length(rawBoostDirection) > kVelocityStopThreshold_) {
            worldBoostDirection = Normalize(rawBoostDirection);
            if (followCamera_) {
                Matrix4x4 rotateMatrix = MakeRotateMatrix(followCamera_->GetViewProjection().transform.rotate);
                worldBoostDirection = TransformNormal(worldBoostDirection, rotateMatrix);
            }
        } else {
            if (followCamera_){
                 Matrix4x4 rotateMatrix = MakeRotateYMatrix(objectTransform_->transform.rotate.y);
                 worldBoostDirection = TransformNormal({0.0f,0.0f,1.0f},rotateMatrix);
            } else {
                 return false;
            }
        }
        worldBoostDirection.y = 0.0f;
        if (Length(worldBoostDirection) < kVelocityStopThreshold_){
            return false;
        }
        worldBoostDirection = Normalize(worldBoostDirection);

        isQuickBoosting_ = true;
        quickBoostFrames_ = maxQuickBoostFrames_;
        currentBoostTime_ -= quickBoostConsumption_;
        quickBoostCooldown_ = maxQuickBoostCooldown_;
        quickBoostUsedCount_++;

        if (quickBoostUsedCount_ >= maxQuickBoostUses_) {
            quickBoostChargeCooldown_ = quickBoostChargeTime_;
        }
        
        velocity_ = worldBoostDirection * (maxSpeed_ * kQuickBoostSpeedMultiplier_ * 0.8f);
        return true; // クイックブースト起動成功
    }
    return false; // クイックブースト起動せず (条件不足)
}

void Player::RecoverBoostEnergy() {
    // 通常ブーストもクイックブーストも使用していない時にエネルギー回復
    if (!isBoosting_ && !isQuickBoosting_ && currentBoostTime_ < maxBoostTime_) {
        float recoveryMultiplier = (Length(velocity_) < kVelocityStopThreshold_ * 5.0f) ? kStationaryBoostRecoveryMultiplier_ : 1.0f;
        currentBoostTime_ += boostRecoveryRate_ * recoveryMultiplier;
        if (currentBoostTime_ > maxBoostTime_) {
            currentBoostTime_ = maxBoostTime_;
        }
    }
}

void Player::ShootMachineGun() {
	// 反動と揺れ用の出力変数
	Vector3 recoilVelocity;
	float shakeIntensity;

	// 【変更】machineGunHeadTransform_のワールド位置を取得
	Vector3 bulletPosition = machineGunHeadTransform_->GetWorldPosition();

	// Shoot関数に位置を渡す
	auto bullet = PlayerMachineGun::Shoot(
		bulletPosition,
		followCamera_,
		recoilVelocity,
		shakeIntensity
	);

	// 反動と揺れを適用
	recoilVelocity_ += recoilVelocity;
	shakeIntensity_ = shakeIntensity;

	// 弾を追加
	machineGunBullets_.push_back(std::move(bullet));

	// 発射エフェクトもマシンガンヘッドに出すように変更（任意）
	//explosionEmitter_->SetPosition(bulletPosition);
	//explosionEmitter_->Emit();
}


void Player::ApplyRecoil() {
	if (Length(recoilVelocity_) > kRecoilThreshold_) {
		// 反動を適用
		objectTransform_->transform.translate.x += recoilVelocity_.x;
		objectTransform_->transform.translate.z += recoilVelocity_.z;

		recoilVelocity_ *= PlayerMachineGun::GetRecoilDecay(); // 減衰率を取得
	} else {
		recoilVelocity_ = { 0.0f, 0.0f, 0.0f };
	}
}
void Player::ApplyShake() {
	if (shakeIntensity_ > 0.01f) {  // ある程度の揺れが残っているとき
		float shakeAmount = shakeIntensity_ * kShakeBaseIntensityFactor_;

		// rand() の代わりにC++の<random>を使うことを推奨
		float offsetX = (static_cast<float>(rand() % kShakeRandRangeX_ - (kShakeRandRangeX_/2)) ) * shakeAmount * kShakeOffsetXFactor_;
		float offsetRot = (static_cast<float>(rand() % kShakeRandRangeRot_ - (kShakeRandRangeRot_/2)) ) * shakeAmount * kShakeOffsetRotFactor_;

		objectTransform_->transform.translate.x += offsetX;
		objectTransform_->transform.rotate.y += offsetRot;

		shakeIntensity_ *= kShakeDecayRate_;
	} else {
		shakeIntensity_ = 0.0f;
	}
}
void Player::ApplyCameraShake()
{
	// カメラの揺れを適用
	if (!isCameraShaking_)
	{
		isCameraShaking_ = true;
		shakeCurrentTime_ = 0.0f;
	}
}

void Player::CameraShake()
{
	if (!isCameraShaking_)
	{
		return;
	}

	if (shakeCurrentTime_ >= kShakeTime_)
	{
		isCameraShaking_ = false;
		return;
	}

	// positionの設定
	std::uniform_real_distribution<float> distribution(-kShakeMagnitude_, kShakeMagnitude_);
	Vector3 randomTranslate = { distribution(randomEngine),distribution(randomEngine) ,distribution(randomEngine) };

	followCamera_->GetViewProjection().transform.translate += randomTranslate;

	followCamera_->GetViewProjection().Update();

	shakeCurrentTime_ += 1.0f / 60.0f;
}
void Player::OpenDoor()
{
	if (isDoorOpening_) {
		doorAngle_ += kDoorOpenCloseSpeed_;
		if (doorAngle_ >= kDoorOpenAngle_) {
			doorAngle_ = kDoorOpenAngle_;
			isDoorOpening_ = false;
			doorOpenTimer_ = kDoorStayOpenFrames_;
		}
	} else if (doorOpenTimer_ > 0) {
		doorOpenTimer_--;
		if (doorOpenTimer_ <= 0) {
			isDoorClosing_ = true;
		}
	} else if (isDoorClosing_) {
		doorAngle_ -= kDoorOpenCloseSpeed_;
		if (doorAngle_ <= kDoorCloseAngle_) {
			doorAngle_ = kDoorCloseAngle_;
			isDoorClosing_ = false;
		}
	}

	// ドアの回転適用
	doorObjectTransform_->transform.rotate.y = doorAngle_;
}
void Player::ApplyFloating()
{
	if (!isJumping_) {
		floatTimer_ += 1.0f / 60.0f; // フレーム時間を進める（60FPS前提）
		// フロートのオフセットを計算
		float floatOffset = std::sin(floatTimer_ * floatFrequency_ * 1.5f * 3.14159265f) * floatAmplitude_;
		objectTransform_->transform.translate.y = initialY_ + floatOffset;
	}
}
///=============================================================================
///						当たり判定
///--------------------------------------------------------------
///						接触開始処理
void Player::OnCollisionEnter(BaseObject* other) {
	if (dynamic_cast<BaseEnemy*>(other) || dynamic_cast<EnemyBullet*>(other)) {
		HandleDamageAndInvincibility();
	}

	ApplyCameraShake();
}
///--------------------------------------------------------------
///						接触継続処理
void Player::OnCollisionStay(BaseObject* other) {
	if (dynamic_cast<BaseEnemy*>(other)) {
		// isJumping_ = true; // 継続接触でジャンプし続けるかは検討
	}
}
///--------------------------------------------------------------
///						接触終了処理
void Player::OnCollisionExit(BaseObject* other) {

}

void Player::HandleDamageAndInvincibility() {
	if (!isInvincible_) {
		hp_--;
		isInvincible_ = true;
		invincibleTimer_ = kInvincibleDuration_; // 定数使用
		isVisible_ = true; // 無敵開始時は必ず表示
	}
}

void Player::StopMachineGunSound() {
	isShootingMachineGun_ = false;
	machineGunSoundPlayed_ = false;
	se3_->SoundStop("Resources/se/999DCD4E5D2A205C05.mp3");
}
