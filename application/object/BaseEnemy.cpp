#include "BaseEnemy.h"
#include "PlayerMissile.h"
#include <algorithm>
#include <cmath>

///=============================================================================
///						初期化
void BaseEnemy::Initialize(Model *model) {
	//========================================
	// モデルを初期化
	model_ = std::make_unique<Object3d>();
	model_->Initialize(Object3dCommon::GetInstance());
	//========================================
	// モデルを読み込む
	model_->SetModel(model);
	//========================================
	// 初期位置を設定
	worldTransform_ = std::make_unique<WorldTransform>();
	worldTransform_->Initialize();
	/*spawnWorldTransform_ = std::make_unique<WorldTransform>();
	spawnWorldTransform_->Initialize();*/
	//========================================
	// 当たり判定との同期
	BaseObject::Initialize(worldTransform_->transform.translate, 6.0f);

	// 乱数生成器の初期化
	std::random_device rd;
	rng_ = std::mt19937(rd());

	// スポーン位置の初期化
	spawnPosition_ = worldTransform_->transform.translate;

	particleEmitter_ = std::make_unique<ParticleEmitter>();
	particleEmitter_->Initialize("missileSmoke");
	particleEmitter_->SetParticleCount(80);
	particleEmitter_->SetLifeTimeRange(ParticleManager::Range({1.0f, 1.5f}));
	particleEmitter_->SetVelocityRange(ParticleManager::Vec3Range({-10.0f, -10.0f, -10.0f}, {10.0f, 10.0f, 10.0f}));

	particleEmitter_->SetFinishColorRange({ { 0.56f,0.0f,0.0f,0.0f }, { 1.0f,0.37f,0.19f,0.0f } });

	ParticleManager::GetInstance()->GetParticleGroup("missileSmoke")->enableBillboard = true;

	avoidanceVelocity_ = {0.0f, 0.0f, 0.0f};
	otherEnemies_ = nullptr;
}

///=============================================================================
///						描画
void BaseEnemy::Update() {
	if (IsAlive()) {
		// 回避ベクトルをリセット
		avoidanceVelocity_ = {0.0f, 0.0f, 0.0f};
		// 他の敵との衝突回避
		CalculateAvoidance();

		// プレイヤーとの距離維持の計算 (MoveToTarget内でも考慮されるが、ここで明示的に行うことも可能)
		if (target_) {
			Vector3 toTarget = target_->transform.translate - worldTransform_->transform.translate;
			float distanceToTarget = Length(toTarget);
			if (distanceToTarget < safeDistance_ && distanceToTarget > 0.0f) { // 距離が0の場合の除算エラーを避ける
				Vector3 awayFromTarget = Normalize(worldTransform_->transform.translate - target_->transform.translate);
				// safeDistance_ との差に基づいて離れる力を調整
				float pushStrength = (safeDistance_ - distanceToTarget) / safeDistance_;
				avoidanceVelocity_ += awayFromTarget * speed_ * pushStrength * 0.5f; // 離れる力は少し弱めに
			}
		}

		// 弾の更新
		BulletUpdate();

		// ワールド変換の更新
		worldTransform_->UpdateMatrix();

		// モデルのローカル行列を更新
		model_->SetLocalMatrix(MakeIdentity4x4());
		// モデルの更新
		model_->Update();

		// パーティクルの位置を更新
		particleEmitter_->SetPosition(worldTransform_->transform.translate);

		//========================================
		// 当たり判定との同期
		BaseObject::Update(worldTransform_->transform.translate);
		if (worldTransform_->transform.translate.y < minY_) {
			worldTransform_->transform.translate.y = minY_;
		}
		if (worldTransform_->transform.translate.y > maxY_) {
			worldTransform_->transform.translate.y = maxY_;
		}
	}
}

///=============================================================================
///						描画
void BaseEnemy::Draw(ViewProjection viewProjection, DirectionalLight directionalLight, PointLight pointLight, SpotLight spotLight) {
	BulletDraw(viewProjection, directionalLight, pointLight, spotLight);

	//========================================
	// モデルの描画
	if (IsAlive()) {
		model_->Draw(*worldTransform_.get(), viewProjection, directionalLight, pointLight, spotLight);
	}
}

///=============================================================================
///						当たり判定
///--------------------------------------------------------------
///						接触開始処理
void BaseEnemy::OnCollisionEnter(BaseObject *other) {
}

///--------------------------------------------------------------
///						接触継続処理
void BaseEnemy::OnCollisionStay(BaseObject *other) {
}

///--------------------------------------------------------------
///						接触終了処理
void BaseEnemy::OnCollisionExit(BaseObject *other) {
}

void BaseEnemy::BulletUpdate() {
	for (auto it = bullets_.begin(); it != bullets_.end();) {
		if (!(*it)->GetIsActive()) {
			it = bullets_.erase(it); // 非アクティブなら削除
		} else {
			it->get()->Update();
			++it; // アクティブなら次へ
		}
	}
}

void BaseEnemy::BulletDraw(ViewProjection viewProjection, DirectionalLight directionalLight, PointLight pointLight, SpotLight spotLight) {
	for (auto it = bullets_.begin(); it != bullets_.end();) {
		it->get()->Draw(viewProjection, directionalLight, pointLight, spotLight);
		++it;
	}
}

void BaseEnemy::Fire() {
	if (target_) {
		std::unique_ptr<EnemyBullet> newBullet = std::make_unique<EnemyBullet>();
		newBullet->Initialize(*worldTransform_.get(), target_->transform.translate);
		bullets_.push_back(std::move(newBullet));
	}
}

void BaseEnemy::MoveToTarget() {
	if (target_) {
		// 目標方向の計算
		Vector3 toTarget = target_->transform.translate - worldTransform_->transform.translate;
		float distanceToTarget = Length(toTarget);

		Vector3 desiredDir = {0.0f, 0.0f, 0.0f};
		if (distanceToTarget > 0.001f) { // ゼロ除算を避ける
			desiredDir = Normalize(toTarget);
		}

		// プレイヤーに近づきすぎている場合は、少し離れるか停止する
		if (distanceToTarget < safeDistance_) {
			// 離れる方向を維持しつつ、速度を落とすか、逆方向に少し動く
			// desiredDir はプレイヤーへの方向なので、これとは逆向きの動きや停止を考慮
			// 例えば、速度を0にするか、safeDistance_ に向かって後退する
			// ここでは単純に目標速度を0に近づける
			Vector3 targetVelocity = {0.0f, 0.0f, 0.0f};
			// もしsafeDistance_よりかなり近いなら、少し後退する力を加える
			if (distanceToTarget < safeDistance_ * 0.5f && distanceToTarget > 0.0f) {
				Vector3 awayFromTarget = Normalize(worldTransform_->transform.translate - target_->transform.translate);
				targetVelocity = awayFromTarget * speed_ * 0.5f; // 後退速度は少し遅め
			}
			velocity_ = Lerp(velocity_, targetVelocity, 0.2f); // 急停止/急後退しないように補間
		} else {
			// 目標回転角度の計算
			float targetY = std::atan2(desiredDir.x, desiredDir.z);
			float currentY = worldTransform_->transform.rotate.y;

			// ステアリング制限回転
			float delta = targetY - currentY;
			delta = std::fmod(delta + static_cast<float>(std::numbers::pi), 2.0f * static_cast<float>(std::numbers::pi)) - static_cast<float>(std::numbers::pi);

			float maxSteeringAngle = 0.05f; // フレームあたりの最大回転角
			delta = std::clamp(delta, -maxSteeringAngle, maxSteeringAngle);
			worldTransform_->transform.rotate.y += delta;

			// 現在の回転方向に基づいて前進
			Vector3 forward = {
				sinf(worldTransform_->transform.rotate.y),
				0.0f,
				cosf(worldTransform_->transform.rotate.y)};

			// 速度補間(スムーズ)
			Vector3 targetVelocity = forward * speed_;
			velocity_ = Lerp(velocity_, targetVelocity, 0.1f);
		}

		// 位置の移動
		worldTransform_->transform.translate += velocity_;
		worldTransform_->transform.translate += avoidanceVelocity_; // 回避ベクトルを最終的な移動に加算
	}
}

void BaseEnemy::RandomMove(float scale) {
	directionChangeTimer_ += 1.0f / 60.0f;
	SetModelColor(Vector4{1.0f, 1.0f, 1.0f, 1.0f});

	float maxSteeringAngle = 0.05f; // 最大旋回角度

	// 移動目標方向の決定
	if (directionChangeTimer_ >= directionChangeInterval_) {
		Vector3 toSpawn = spawnPosition_ - worldTransform_->transform.translate;
		float distanceToSpawn = Length(toSpawn);

		Vector3 newDir;
		if (distanceToSpawn > wanderRadius_) {
			newDir = Normalize(toSpawn);
		} else {
			float angle = angleDist_(rng_);
			newDir = Vector3{cosf(angle), 0.0f, sinf(angle)};
		}

		// 眺める目標角度の更新
		targetRotationY_ = std::atan2(newDir.x, newDir.z);

		directionChangeTimer_ = 0.0f;
	}

	// 回転加速度適用(自動車のようにスムーズに回転)
	float currentY = worldTransform_->transform.rotate.y;
	float delta = targetRotationY_ - currentY;
	delta = std::fmod(delta + static_cast<float>(std::numbers::pi), 2.0f * static_cast<float>(std::numbers::pi)) - static_cast<float>(std::numbers::pi);
	delta = std::clamp(delta, -maxSteeringAngle, maxSteeringAngle);
	worldTransform_->transform.rotate.y += delta;

	float turnSpeed = 0.1f; // 回転感度
	rotationVelocityY_ += delta * turnSpeed;
	rotationVelocityY_ *= 0.85f; // 減速(摩擦)
	worldTransform_->transform.rotate.y += rotationVelocityY_;

	// 眺める方向を基準に移動(前進)
	Vector3 forward = {
		sinf(worldTransform_->transform.rotate.y),
		0.0f,
		cosf(worldTransform_->transform.rotate.y)};
	worldTransform_->transform.scale = {scale, scale, scale};
	velocity_ = forward * speed_;
	// worldTransform_->transform.translate += velocity_; // BaseEnemy::Update()の最後で加算するためコメントアウト
}
// コンテキストベースの方向選択
Vector3 BaseEnemy::SelectDirection() {
	// 実装は省略（シンプルさを優先）
	return velocity_;
}

void BaseEnemy::CalculateAvoidance() {
	if (!otherEnemies_ || otherEnemies_->empty()) {
		return;
	}

	Vector3 totalAvoidanceForce = {0.0f, 0.0f, 0.0f};
	int neighborsCount = 0;

	for (BaseEnemy *other : *otherEnemies_) {
		if (other == this || !other->IsAlive()) {
			continue;
		}

		Vector3 toOther = other->GetWorldTransform()->transform.translate - worldTransform_->transform.translate;
		float distance = Length(toOther);

		if (distance < avoidanceRadius_ && distance > 0.0f) { // 距離が0の場合は無視
			// 反発力は距離に反比例
			Vector3 avoidanceForce = Normalize(worldTransform_->transform.translate - other->GetWorldTransform()->transform.translate);
			avoidanceForce = avoidanceForce * (1.0f - (distance / avoidanceRadius_)); // 近いほど強い力
			totalAvoidanceForce += avoidanceForce;
			neighborsCount++;
		}
	}

	if (neighborsCount > 0) {
		totalAvoidanceForce.x = totalAvoidanceForce.x / static_cast<float>(neighborsCount); // 平均化
		totalAvoidanceForce.y = totalAvoidanceForce.y / static_cast<float>(neighborsCount); // 平均化
		totalAvoidanceForce.z = totalAvoidanceForce.z / static_cast<float>(neighborsCount); // 平均化
		// 回避速度を現在の速度や状態に応じて調整することも可能
		avoidanceVelocity_ += totalAvoidanceForce * speed_ * 0.75f; // 回避力は少し弱めに設定
	}
}