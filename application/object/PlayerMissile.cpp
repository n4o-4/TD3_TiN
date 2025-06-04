#define _USE_MATH_DEFINES
#define NOMINMAX
#include <algorithm>
#include <cmath>
#include <random>
#include "PlayerMissile.h"

// コンストラクタ実装
PlayerMissile::PlayerMissile(const Vector3 &position, const Vector3 &initialVelocity,
    const Vector3 &scale, const Vector3 &rotate, int lockLevel) {


    //===================================================
    isActive_ = true;  // 明示的にアクティブに設定

    //===================================================
    // 3Dモデルの初期化
    model_ = std::make_unique<Object3d>();
    model_->Initialize(Object3dCommon::GetInstance());
    // モデルの読み込み
    ModelManager::GetInstance()->LoadModel("missile.obj");
    model_->SetModel("missile.obj");


    //===================================================
    // パーティクルの初期化
    ParticleManager::GetInstance()->CreateParticleGroup("missileSmoke", "Resources/circle.png", ParticleManager::ParticleType::Normal);
	particleEmitterMissileSmoke_ = std::make_unique<ParticleEmitter>();
	particleEmitterMissileSmoke_->Initialize("missileSmoke");
    // パーティクル設定の調整
    particleEmitterMissileSmoke_->SetParticleCount(2);   // デフォルト発生数
    particleEmitterMissileSmoke_->SetFrequency(0.01f);    // より高頻度で発生させる
    particleEmitterMissileSmoke_->SetStartColorRange(ParticleManager::ColorRange({ 1.0f,1.0f,1.0f,1.0f }, { 1.0f,1.0f,1.0f,1.0f }));
	particleEmitterMissileSmoke_->SetFinishColorRange(ParticleManager::ColorRange({ 1.0f,1.0f,1.0f,0.0f }, { 1.0f,1.0f,1.0f,0.0f }));
    particleEmitterMissileSmoke_->SetStartScaleRange(ParticleManager::Vec3Range({ 3.0f,3.0f,3.0f, }, { 5.0f,5.0f,5.0f }));

    particleEmitterMissileSmoke_->SetFinishScaleRange(ParticleManager::Vec3Range({ 3.0f,3.0f,3.0f, }, { 5.0f,5.0f,5.0f }));

    particleEmitterMissileSmoke_->SetVelocityRange(ParticleManager::Vec3Range({ -10.0f,-10.0f,-10.0f }, { 10.0f,10.0f,10.0f }));
    
	particleEmitterMissileSmoke_->SetLifeTimeRange({ 0.1f, 0.5f }); // 寿命を短くする

    ParticleManager::GetInstance()->GetParticleGroup("missileSmoke")->enableBillboard = true;
    
    ParticleManager::GetInstance()->GetParticleGroup("missileSmoke")->enableDeceleration = true;


    ParticleManager::GetInstance()->CreateParticleGroup("explosion", "Resources/circle.png", ParticleManager::ParticleType::Normal);
	explosionEmitter_ = std::make_unique<ParticleEmitter>();
	explosionEmitter_->Initialize("explosion");
    // パーティクル設定の調整
	
	explosionEmitter_->SetParticleCount(100);   
	explosionEmitter_->SetFrequency(0.04f);
    explosionEmitter_->SetLifeTimeRange({ 0.5f, 1.0f }); // 寿命を短くする
    explosionEmitter_->SetStartScaleRange(ParticleManager::Vec3Range({ 3.0f,3.0f,3.0f, }, { 5.0f,5.0f,5.0f }));
    explosionEmitter_->SetFinishScaleRange(ParticleManager::Vec3Range({ 3.0f,3.0f,3.0f, }, { 5.0f,5.0f,5.0f }));
	explosionEmitter_->SetVelocityRange(ParticleManager::Vec3Range({ -100.0f,-100.0f,-100.0f }, { 100.0f,100.0f,100.0f }));


    ParticleManager::GetInstance()->GetParticleGroup("explosion")->enableBillboard = true;
    ParticleManager::GetInstance()->GetParticleGroup("explosion")->enablePulse = true;

    //===================================================
    // トランスフォームの初期化
    worldTransform_ = std::make_unique<WorldTransform>();
    worldTransform_->Initialize();
    worldTransform_->transform.translate = position;
    // 少し小さくしておく
    worldTransform_->transform.scale = { kMissileScaleXY, kMissileScaleXY, kMissileScaleZ };
    worldTransform_->transform.rotate = rotate;
    
    //===================================================
    // 初期速度の設定
    velocity_ = initialVelocity;

    //===================================================
    // ロックオンレベルの保存
    lockLevel_ = lockLevel;

    //===================================================
    // ロックオンレベルに応じた性能設定
    if (lockLevel_ == 2) {  // 精密ロックオン
        // 高性能ミサイルの設定
        precisionTurnFactor_ = kPrecisionTurnFactor;
        precisionTrackingFactor_ = kPrecisionTrackingFactor;
        maxTurnRate_ *= precisionTurnFactor_;
        navigationGain_ *= precisionTrackingFactor_;
        
        // TODO:モデルの色を変更（高性能ミサイルの見た目を区別）
        //model_->SetColor({1.0f, 0.5f, 0.2f, 1.0f});  // オレンジ色で強調
    } else {
        // 通常ミサイル（簡易ロックオン）の設定
        // 基本設定のままでよい
    }

    //===================================================
    // ミサイル性能にランダムなばらつきを追加
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(kPerformanceVariationMin, kPerformanceVariationMax);
    performanceVariation_ = dis(gen);

    //===================================================
    // 前回視線ベクトルの初期化（初回はゼロでよい）
    prevLineOfSight_ = { 0, 0, 0 };

    //===================================================
    // 簡易ロックオンの場合、発射時のターゲット位置を記録
    if (lockLevel_ == 1 && target_) {
        targetPosition_ = target_->GetPosition();
        // impactPosition_にも同じ値を設定
        impactPosition_ = targetPosition_;
    } else if (lockLevel_ == 2 && target_) {
        // 精密ロックオンの場合はターゲットの位置に向かって飛ぶ
        impactPosition_ = target_->GetPosition();
    }

    //========================================
    // 当たり判定との同期
    BaseObject::Initialize(worldTransform_->transform.translate, 1.0f);
}


void PlayerMissile::Update() {
    // ライフタイムを更新
    lifeTime_++;

    // ライフタイム経過によるチェック（寿命）
    if(lifeTime_ > kLifeTimeLimit) {
        isActive_ = false;
    }

    // 非アクティブなら処理しない
    if (!isActive_) {
        return;
    }

    // 精密ロックオン時のターゲット位置更新
    if (lockLevel_ == 2 && target_) {
        // 継続的にターゲット位置を更新
        impactPosition_ = target_->GetPosition();
    }

    // 状態に応じた更新処理
    switch(state_) {
    case BulletState::kLaunch:
        UpdateLaunchState();
        break;
    case BulletState::kTracking:
        UpdateTrackingState();
        break;
    case BulletState::kFinal:
        UpdateFinalState();
        break;
    } 

    // 位置の更新
    worldTransform_->transform.translate = worldTransform_->transform.translate + velocity_;
    model_->SetLocalMatrix(MakeIdentity4x4());// ローカル行列を単位行列に
    worldTransform_->UpdateMatrix();

    if (particleEmitterMissileSmoke_) {
        // ミサイルの進行方向を取得
        Vector3 direction = Normalize(velocity_);
        
        // 進行方向の逆向きにオフセットを適用して煙の発生位置を設定
        Vector3 smokeOffset = direction * -2.6f;  // ミサイルの少し後ろに配置
        
        // エミッタの位置を更新
        particleEmitterMissileSmoke_->SetPosition(worldTransform_->transform.translate + smokeOffset);

        // パーティクルエミッタの更新
        particleEmitterMissileSmoke_->Update();
    }

    // 当たり判定の更新
    BaseObject::Update(worldTransform_->transform.translate);

    // 近接信管の処理
    ApplyProximityFuse();
}

// 発射初期状態の更新
void PlayerMissile::UpdateLaunchState() {
	// タイマー更新
	stateTimer_++;

	// 慣性飛行（最初の数フレームは初期方向に飛ぶ）
	if(stateTimer_ <= kInertialFlightTime) {
		// 初期速度で直進
		float launchSpeedFactor = std::min(1.0f, stateTimer_ / 10.0f);
		velocity_ = Normalize(velocity_) * ( kInitialSpeed + launchSpeedFactor * 0.1f );
	} else {
		// 徐々に目標方向へ旋回を開始
		if(target_) {
			// ターゲットへの方向ベクトル
			Vector3 targetPos = impactPosition_;
			Vector3 direction = targetPos - worldTransform_->transform.translate;
			Vector3 normalizedDirection = Normalize(direction);

			// 現在の速度方向
			Vector3 currentDir = Normalize(velocity_);

			// 徐々にターゲット方向に旋回
			float turnRate = ( stateTimer_ - kInertialFlightTime ) / 30.0f * 0.0022f;
			turnRate = std::min(turnRate, 0.03f);

			// 現在の方向と目標方向の間を補間
			Vector3 newDir = currentDir + ( normalizedDirection - currentDir ) * turnRate;
			newDir = Normalize(newDir);

			// 速度を徐々に増加
			float speedFactor = std::min(1.0f, stateTimer_ / kLaunchSpeedDivider);
			float currentSpeed = kInitialSpeed + speedFactor * kSpeedIncreaseFactor;
			velocity_ = newDir * currentSpeed;
		} else {
			// ターゲットがない場合は直進し続ける
			float speedFactor = std::min(1.0f, stateTimer_ / kLaunchSpeedDivider);
			float currentSpeed = kInitialSpeed + speedFactor * kSpeedIncreaseFactor;
			velocity_ = Normalize(velocity_) * currentSpeed;
		}
	}

	// わずかにランダムな動きを加える（発射演出）
	if(stateTimer_ % kRandomMoveInterval == 0 && stateTimer_ < kLaunchStateTransitionTime * 0.7f) {
		float randomX = ( rand() % 100 - 50 ) / kRandomMoveFactor;
		float randomY = ( rand() % 100 - 50 ) / kRandomMoveFactor;
		velocity_ = velocity_ + Vector3{ randomX, randomY, 0.0f };
		velocity_ = Normalize(velocity_) * Length(velocity_); // 速度の大きさを維持
	}

	// 一定時間経過後、トラッキング状態に移行
	if(stateTimer_ >= kLaunchStateTransitionTime) {
		state_ = BulletState::kTracking;
		stateTimer_ = 0;

		// 前回の視線方向を初期化（追尾状態開始時）
		if(target_) {
			prevLineOfSight_ = impactPosition_ - worldTransform_->transform.translate;
		}
	}
}

// 追尾状態の更新
void PlayerMissile::UpdateTrackingState() {
    stateTimer_++;

    if(target_) {
        // ターゲットの位置を取得
        Vector3 targetPos = impactPosition_;
        Vector3 myPos = worldTransform_->transform.translate;
        Vector3 direction = targetPos - myPos;
        float distance = Length(direction);

        // 比例航法による誘導
        Vector3 navVector = CalculateNavigationVector();

        // 精密ロックオンの場合、より正確な誘導
        if (lockLevel_ == 2) {
            // 旋回性能の限界を高く設定（より機敏に）
            float navMagnitude = Length(navVector);
            if(navMagnitude > maxTurnRate_) {
                navVector = Normalize(navVector) * maxTurnRate_;
            }
            
            // 精密ロックオンでは弧を描く動きをほぼ削除（直線的な軌道に）
            // 0.1fという小さな値にして、わずかな自然な動きのみ残す
            float arcFactor = sinf(stateTimer_ * kArcOscillationSpeed * 0.1f) * 
                             (kArcStrength * 0.1f);
            Vector3 perpVector = Cross(Normalize(velocity_), Vector3{ 0, 1, 0 });
            perpVector = perpVector * arcFactor;
            
            // 追尾遅延を大幅に低減（ほぼ即時反応）
            navVector = navVector * (kTrackingDelayFactor * 1.5f) * performanceVariation_;
            
            // スムージング係数を調整（より素早く方向転換）
            float smoothingFactor = kSmoothingFactor * 1.5f;
            Vector3 desiredChangeVector = navVector + perpVector;
            Vector3 smoothedChangeVector = desiredChangeVector * smoothingFactor;
            
            velocity_ = velocity_ + smoothedChangeVector;
        } else {
            // 簡易ロックオンの場合は既存のコード（変更なし）
            float navMagnitude = Length(navVector);
            if(navMagnitude > maxTurnRate_ * kBasicTurnRateFactor) {
                navVector = Normalize(navVector) * (maxTurnRate_ * kBasicTurnRateFactor);
            }
            
            // 弧を描くような動きを追加
            float arcFactor = sinf(stateTimer_ * kArcOscillationSpeed) * kArcStrength;
            Vector3 perpVector = Cross(Normalize(velocity_), Vector3{ 0, 1, 0 });
            perpVector = perpVector * arcFactor;
            
            // 追尾遅延のシミュレーション
            navVector = navVector * kTrackingDelayFactor * performanceVariation_;
            
            // スムージング係数を導入
            float smoothingFactor = kSmoothingFactor;
            Vector3 desiredChangeVector = navVector + perpVector;
            Vector3 smoothedChangeVector = desiredChangeVector * smoothingFactor;
            
            velocity_ = velocity_ + smoothedChangeVector;
        }

        // 速度の大きさを調整
        float currentMaxSpeed = kBaseTrackingSpeed +
            std::min(kMaxSpeedIncrease, stateTimer_ / kSpeedIncreaseDivider);

        // 精密ロックオンの場合、より高速に
        if (lockLevel_ == 2) {
            currentMaxSpeed *= kPrecisionSpeedFactor;
        }
        
        // 性能のバラつきを適用
        currentMaxSpeed *= (1.0f + (performanceVariation_ - 1.0f) * 0.5f);

        velocity_ = Normalize(velocity_) * currentMaxSpeed;

    } else {
        // ターゲットがいない場合は直進
        velocity_ = Normalize(velocity_) * kBaseTrackingSpeed;
    }

    // 長時間追尾してもターゲットに到達できない場合
    if(stateTimer_ > kAutoFinalStateTransitionTime) {
        state_ = BulletState::kFinal;
        stateTimer_ = 0;
    }
}

// 最終接近状態の更新
void PlayerMissile::UpdateFinalState() {
    stateTimer_++;

	// FIXME : ここから下の処理は、実装が不完全です｡例外スローあり｡
    if(target_) {
        // ターゲットへの方向
        Vector3 targetPos = impactPosition_;
        Vector3 myPos = worldTransform_->transform.translate;
        Vector3 direction = targetPos - myPos;
        float distance = Length(direction);

        // 精密ロックオンの場合、最終フェーズでは軌道計算を大幅に簡略化
        if (lockLevel_ == 2) {
            // 目標方向を直接計算（正規化）
            Vector3 targetDir = Normalize(direction);
            
            // 現在の速度は無視して、方向を敵に向ける（直接突進）
            // 急加速して直進
            float finalSpeed = kBaseFinalSpeed * 1.5f + 
                std::min(kMaxFinalSpeedIncrease * 1.2f, stateTimer_ / (kFinalSpeedIncreaseDivider * 0.5f));
            
            // 直接目標に向かう速度ベクトルを設定
            velocity_ = targetDir * finalSpeed;
            
            // ミサイル性能のランダムばらつきをほぼ無効化（精密誘導）
            if(stateTimer_ > kPrecisionFinalPhaseThreshold) {
                // ほぼ直線的な軌道を維持するため、偏差を最小限に
                float randomDeviation = (rand() % 100 - 50) / (kPrecisionRandomDeviationFactor * 10.0f);
                velocity_.x += randomDeviation;
                velocity_.y += randomDeviation;
            }
        } else {
            // 簡易ロックオンの場合は既存のコード
            Vector3 currentDir = Normalize(velocity_);
            Vector3 targetDir = Normalize(direction);
            
            // 既存の最終段階での旋回率
            float finalTurnRate = maxTurnRate_ * kFinalStageTurnFactor;
            
            // 現在方向と目標方向の間を補間
            Vector3 newDir = currentDir + (targetDir - currentDir) * finalTurnRate;
            newDir = Normalize(newDir);
            
            // 急加速して直進
            float finalSpeed = kBaseFinalSpeed +
                std::min(kMaxFinalSpeedIncrease, stateTimer_ / kFinalSpeedIncreaseDivider);
            
            velocity_ = newDir * finalSpeed;
            
            // 簡易ロックオン時のランダム偏差（既存コード）
            if(stateTimer_ > kBasicFinalPhaseThreshold) {
                float randomDeviation = (rand() % 100 - 50) / kBasicRandomDeviationFactor;
                velocity_.x += randomDeviation;
                velocity_.y += randomDeviation;
            }
        }
    } else {
        // ターゲットがない場合は現在の速度で直進（既存コード）
        float finalSpeed = kBaseFinalSpeed +
            std::min(kMaxFinalSpeedIncrease, stateTimer_ / kFinalSpeedIncreaseDivider);
        velocity_ = Normalize(velocity_) * finalSpeed;
    }
}

// 視線ベクトル（LOS: Line of Sight）の計算
Vector3 PlayerMissile::CalculateLOS() {
	if(!target_) {
		return Normalize(velocity_); // ターゲットがない場合は現在の進行方向
	}

	// ミサイルからターゲットへのベクトル
	Vector3 los = impactPosition_ - worldTransform_->transform.translate;
	return los;
}

// 視線角速度（LOS Rate）の計算
Vector3 PlayerMissile::CalculateLOSRate() {
	if(!target_) {
		return Vector3{ 0, 0, 0 }; // ターゲットがない場合はゼロ
	}

	// 現在の視線ベクトル
	Vector3 currentLOS = CalculateLOS();

	// 前回の視線ベクトルとの差分から角速度を近似
	Vector3 losRate = currentLOS - prevLineOfSight_;

	// 前回の視線ベクトルを更新
	prevLineOfSight_ = currentLOS;

	return losRate;
}

// 比例航法ベクトルの計算（PN: Proportional Navigation）
Vector3 PlayerMissile::CalculateNavigationVector() {
    if(!target_) {
        return Vector3{ 0, 0, 0 };
    }

    // 視線角速度を取得
    Vector3 losRate = CalculateLOSRate();

    // 精密ロックオン時は異なるパラメータを使用
    float reducedNavGain;
    float steeringForceFactor;
    float velocityPerpWeightFactor;
    float curveStrengthFactor;

    if (lockLevel_ == 2) {
        // 精密ロックオン：より正確な誘導
        reducedNavGain = kNavigationGain * 0.9f;  // 90%のゲイン（高めに）
        steeringForceFactor = 0.9f;              // 強めの操舵力
        velocityPerpWeightFactor = 0.3f;         // 垂直成分の影響を抑制
        curveStrengthFactor = 0.2f;              // 曲線運動をほぼ無効化
    } else {
        // 簡易ロックオン：既存の値
        reducedNavGain = kNavigationGain * kNavGainReductionFactor;
        steeringForceFactor = kSteeringForceFactor;
        velocityPerpWeightFactor = kVelocityPerpWeightFactor;
        curveStrengthFactor = kSteeringForceFactor;
    }

    // 比例航法の加速度ベクトル計算
    Vector3 pnAccel = Cross(velocity_, losRate) * reducedNavGain;

    // 現在の速度方向
    Vector3 currentDir = Normalize(velocity_);

    // 視線方向の単位ベクトル
    Vector3 losUnit = Normalize(CalculateLOS());

    // 相対速度から視線方向成分を除去（横方向加速度成分の計算）
    Vector3 velocityPerp;
    velocityPerp.x = velocity_.x - Dot(velocity_, losUnit) * losUnit.x;
    velocityPerp.y = velocity_.y - Dot(velocity_, losUnit) * losUnit.y;
    velocityPerp.z = velocity_.z - Dot(velocity_, losUnit) * losUnit.z;

    // 曲線的な軌道補正（精密ロックオン時は最小化）
    float curveFactor = sinf(stateTimer_ * (kCurveOscillationSpeed * curveStrengthFactor)) * 
                       (kCurveStrength * curveStrengthFactor);
    Vector3 curveVector = Cross(losUnit, Vector3{ 0, 1, 0 }) * curveFactor;

    // 操舵方向を計算
    Vector3 steeringDir = Normalize(losUnit + velocityPerp * velocityPerpWeightFactor + curveVector);

    // 操舵力を計算
    Vector3 steeringForce = (steeringDir - currentDir) * (navigationGain_ * steeringForceFactor);

    // 比例航法と操舵力を組み合わせた最終的な誘導力
    return pnAccel + steeringForce;
}

// 近接信管の処理
void PlayerMissile::ApplyProximityFuse() {
	if(!target_) {
		return;
	}

	// ターゲットとの距離を計算
	Vector3 targetPos = impactPosition_;
	Vector3 myPos = worldTransform_->transform.translate;
	float distance = Length(targetPos - myPos);

	// 近接信管の発動距離内に入った場合
	if(distance < kProximityFuseDistance) {
		proximityFused_ = true;
		// ここで爆発エフェクト等を発生させる処理を追加できます
		// 例: CreateExplosionEffect();

		isActive_ = false; // ミサイルを非アクティブ化
	}
}

void PlayerMissile::Draw(ViewProjection viewProjection, DirectionalLight directionalLight,
	PointLight pointLight, SpotLight spotLight) {
// アクティブな場合のみ描画
	if(isActive_) {
		// 弾を進行方向に向ける
		if(Length(velocity_) > 0.01f) {
			Vector3 direction = Normalize(velocity_);
			// Y軸回転角を計算
			float rotY = atan2f(direction.x, direction.z);
			// X軸回転角を計算（上下の角度）
			float xzLength = sqrtf(direction.x * direction.x + direction.z * direction.z);
			float rotX = -atan2f(direction.y, xzLength);

			worldTransform_->transform.rotate = { rotX, rotY, 0.0f };
		}
	}
	// モデルの描画
	model_->Draw(*worldTransform_.get(), viewProjection, directionalLight,
		pointLight, spotLight);
}

///=============================================================================
///						当たり判定
///--------------------------------------------------------------
///						接触開始処理
void PlayerMissile::OnCollisionEnter(BaseObject *other) {
	//========================================
	// 敵接触
	if(BaseEnemy *enemy = dynamic_cast<BaseEnemy *>( other )) {
		//---------------------------------------
		//弾を消す
		isActive_ = false;

		// 接触時に爆発エフェクトを発生させるなど
		// 追加の処理をここに実装できます

		explosionEmitter_->SetPosition(worldTransform_->transform.translate);
		explosionEmitter_->Emit();
	}
}

///--------------------------------------------------------------
///						接触継続処理
void PlayerMissile::OnCollisionStay(BaseObject *other) {
	// 継続的な衝突処理が必要な場合はここに実装
}

///--------------------------------------------------------------
///						接触終了処理
void PlayerMissile::OnCollisionExit(BaseObject *other) {
	// 衝突終了時の処理が必要な場合はここに実装
}