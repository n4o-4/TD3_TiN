/*********************************************************************
 * \file   Hud.cpp
 * \brief
 *
 * \author Harukichimaru
 * \date   March 2025
 * \note
 *********************************************************************/
#include "Hud.h"

///=============================================================================
///						初期化
void Hud::Initialize(FollowCamera *followCamera, Player *player, LockOn *lockOnSystem) {
	// カメラのポインタをセット
	followCamera_ = followCamera;
	// プレイヤーのポインタをセット
	player_ = player;
	// ロックオンシステムのポインタをセット
	lockOnSystem_ = lockOnSystem;
	// ラインマネージャをセット
	lineManager_ = LineManager::GetInstance();

	// 初期状態では戦闘モードをアクティブにする
	isCombatMode_ = true;
}

///=============================================================================
///						敵コンテナとスポーンブロックの設定
void Hud::SetEnemiesAndSpawns(const std::vector<std::unique_ptr<BaseEnemy>> *enemies,
							  const std::vector<std::unique_ptr<BaseEnemy>> *spawns) {
	enemies_ = enemies;
	spawns_ = spawns;
}

///=============================================================================
///						更新
void Hud::Update() {
	// 戦闘モードでない場合は更新しない
	if (!isCombatMode_) {
		return;
	}

	// ロックオン表示の回転を更新
	lockOnRotation_ += lockOnRotationSpeed_;
	if (lockOnRotation_ > kTwoPi) {
		lockOnRotation_ -= kTwoPi;
	}
}

///=============================================================================
///						描画
void Hud::Draw(ViewProjection viewProjection) {
	// 戦闘モードでない場合は描画しない
	if (!isCombatMode_) {
		return;
	}

	// 照準の描画
	DrawCrosshair(viewProjection);

	// ロックオンの描画
	DrawLockOn(viewProjection);

	// 3D球体ミニマップの描画（2Dレーダーの代わりに使用）
	if (use3DSphereMap_) {
		DrawSphereMap(viewProjection);
	}

	// プレイヤーステータスバーの描画
	DrawPlayerStatusBars(viewProjection);

	// プレイヤーヘルスバーの描画
	DrawPlayerHealthBar(viewProjection);
}

///=============================================================================
///						戦闘モード切り替え
void Hud::CombatModeActive() {
	isCombatMode_ = true;
}
void Hud::CombatModeDeactive() {
	isCombatMode_ = false;
}

///=============================================================================
///						照準の描画
void Hud::DrawCrosshair(ViewProjection viewProjection) {
	// プレイヤーの位置を取得
	Vector3 playerPos = followCamera_->GetViewProjection().worldPosition_;

	// カメラの前方向ベクトルを取得
	Vector3 cameraForward = followCamera_->GetForwardDirection();

	// 照準の位置を計算（プレイヤーの少し前方）
	Vector3 crosshairPos = {
		playerPos.x + cameraForward.x * crosshairDistance_,
		playerPos.y + cameraForward.y * crosshairDistance_,
		playerPos.z + cameraForward.z * crosshairDistance_};

	// 上下左右の線の位置計算用に、カメラの右方向と上方向を取得
	Vector3 rightDir = followCamera_->GetRightDirection();
	Vector3 upDir = followCamera_->GetUpDirection();

	// 中心点
	Vector3 center = crosshairPos;

	// 全天周囲モニター風の照準に更新

	// 内側の円を描画（サイト中心）
	DrawFacingCircle(center, crosshairCenterRadius_ * 0.5f,
					 Vector4{1.0f, 1.0f, 1.0f, 0.9f}, crosshairCircleSegments_, cameraForward);

	// 外側のリング
	DrawFacingCircle(center, crosshairSize_ * 1.0f, crosshairColor_, crosshairCircleSegments_ * 2, cameraForward);

	// 方向指示マーク（十字線の代わり）
	float markerSize = crosshairSize_ * 0.8f;
	float gapSize = crosshairGap_ * 0.7f;

	// 上マーカー（三角形）
	Vector3 topMarkerCenter = center + upDir * gapSize;
	DrawFacingTriangle(topMarkerCenter, markerSize * 0.3f, crosshairColor_, cameraForward);

	// 下マーカー（四角形）
	Vector3 bottomMarkerCenter = center - upDir * gapSize;
	DrawFacingSquare(bottomMarkerCenter, markerSize * 0.25f, crosshairColor_, rightDir, upDir);

	// 左右マーカー（線）
	float sideMarkerLength = markerSize * 0.4f;
	Vector3 rightStart = center + rightDir * gapSize;
	Vector3 rightEnd = rightStart + rightDir * sideMarkerLength;
	lineManager_->DrawLine(rightStart, rightEnd, crosshairColor_);

	Vector3 leftStart = center - rightDir * gapSize;
	Vector3 leftEnd = leftStart - rightDir * sideMarkerLength;
	lineManager_->DrawLine(leftStart, leftEnd, crosshairColor_);

	// ロックオンターゲットがある場合の照準拡張
	if (lockOnSystem_) {
		const auto &lockedEnemies = lockOnSystem_->GetLockedEnemies();
		if (!lockedEnemies.empty()) {
			// 最初のロックオンターゲットの情報を照準に反映
			BaseEnemy *target = lockedEnemies[0];
			if (target) {
				LockOn::LockLevel lockLevel = lockOnSystem_->GetLockLevel(target);

				// ロックレベルに応じた表示
				if (lockLevel == LockOn::LockLevel::PreciseLock) {
					// 精密ロック時は照準を拡張
					float pulseSize = 1.0f + 0.2f * sinf(lockOnRotation_ * 5.0f);
					DrawFacingCircle(center, crosshairSize_ * 1.5f * pulseSize,
									 hudLockColor_, crosshairCircleSegments_, cameraForward);

					// 「ロック完了」を示す照準外枠
					DrawFacingPolygon(center, crosshairSize_ * 2.0f, 8, hudLockColor_, cameraForward);
				} else if (lockLevel == LockOn::LockLevel::SimpleLock) {
					// 簡易ロック時は照準に補助円を追加
					DrawFacingCircle(center, crosshairSize_ * 1.2f,
									 hudAlertColor_, crosshairCircleSegments_, cameraForward);
				}
			}
		}
	}
}

void Hud::DrawLockOn(ViewProjection viewProjection) {
	// ロックオンシステムがない場合は何もしない
	if (!lockOnSystem_) {
		return;
	}

	// ロックオンしている敵の数を取得
	const auto &lockedEnemies = lockOnSystem_->GetLockedEnemies();

	// カメラの方向ベクトルを取得
	Vector3 cameraForward = followCamera_->GetForwardDirection();
	Vector3 cameraRight = followCamera_->GetRightDirection();
	Vector3 cameraUp = followCamera_->GetUpDirection();

	// 各ロックオンターゲットに対して描画
	for (auto *enemy : lockedEnemies) {
		if (!enemy)
			continue;

		// 敵の位置を取得
		Vector3 enemyPos = enemy->GetPosition();

		// ロックオンレベルに応じた表示
		LockOn::LockLevel lockLevel = lockOnSystem_->GetLockLevel(enemy);

		Vector4 markerColor = lockOnColor_;
		float markerSize = lockOnSize_;

		// ロックオンレベルによって色や表示を変更
		switch (lockLevel) {
		case LockOn::LockLevel::PreciseLock: {
			// 精密ロックオン（エースコンバットスタイル）
			markerColor = preciseLockonColor_;
			markerSize = lockOnSize_ * preciseLockonSizeRatio_;

			// 八角形のターゲットボックスを描画
			int segmentCount = 8;
			Vector3 prevPoint;
			for (int i = 0; i <= segmentCount; i++) {
				float segmentAngle = i * kTwoPi / segmentCount;
				Vector3 point = enemyPos + cameraRight * (cosf(segmentAngle) * markerSize) + cameraUp * (sinf(segmentAngle) * markerSize);

				if (i > 0) {
					lineManager_->DrawLine(prevPoint, point, markerColor);
				}
				prevPoint = point;
			}

			// 中央の十字マーク
			float crossSize = markerSize * 0.3f;
			lineManager_->DrawLine(enemyPos - cameraRight * crossSize, enemyPos + cameraRight * crossSize, markerColor);
			lineManager_->DrawLine(enemyPos - cameraUp * crossSize, enemyPos + cameraUp * crossSize, markerColor);
			break;
		}
		case LockOn::LockLevel::SimpleLock: {
			// 簡易ロックオン（エースコンバットスタイル - 六角形）
			markerColor = simpleLockonColor_;
			markerSize = lockOnSize_ * simpleLockonSizeRatio_;

			// 六角形を描画
			int segmentCount = 6;
			Vector3 prevPoint;
			for (int i = 0; i <= segmentCount; i++) {
				float segmentAngle = i * kTwoPi / segmentCount;
				Vector3 point = enemyPos + cameraRight * (cosf(segmentAngle) * markerSize) + cameraUp * (sinf(segmentAngle) * markerSize);

				if (i > 0) {
					lineManager_->DrawLine(prevPoint, point, markerColor);
				}
				prevPoint = point;
			}
			break;
		}
		case LockOn::LockLevel::None:
		default: {
			// ロックオンなし/デフォルト（菱形マーカー）
			markerColor = noLockonColor_;
			markerSize = lockOnSize_ * noLockonSizeRatio_;

			// 菱形マーカーを描画
			Vector3 top = enemyPos + cameraUp * markerSize;
			Vector3 right = enemyPos + cameraRight * markerSize;
			Vector3 bottom = enemyPos - cameraUp * markerSize;
			Vector3 left = enemyPos - cameraRight * markerSize;

			lineManager_->DrawLine(top, right, markerColor);
			lineManager_->DrawLine(right, bottom, markerColor);
			lineManager_->DrawLine(bottom, left, markerColor);
			lineManager_->DrawLine(left, top, markerColor);
			break;
		}
		}

		// ロックオン進行状況インジケーター（エースコンバットスタイル - 四分割進行度）
		LockOn::LockedEnemyInfo *info = lockOnSystem_->GetLockedEnemyInfo(enemy);
		if (info && lockLevel == LockOn::LockLevel::SimpleLock) {
			// ロックオン進行度を計算（0.0～1.0）
			float lockProgress = info->lockTimer / lockOnSystem_->GetTimeToPreciseLock();
			lockProgress = std::min(lockProgress, 1.0f);

			// 進行度に応じて4つのコーナーを順に表示
			float cornerSize = markerSize * 1.2f;
			float cornerLength = markerSize * 0.8f;

			// 進行度に応じて表示するコーナーの数を決定
			int cornersToShow = static_cast<int>(lockProgress * 4.0f) + 1;
			cornersToShow = std::min(cornersToShow, 4);

			// 左上コーナー
			if (cornersToShow >= 1) {
				Vector3 corner1Start = enemyPos - cameraRight * cornerSize + cameraUp * cornerSize;
				lineManager_->DrawLine(corner1Start, corner1Start + cameraRight * cornerLength, progressColor_);
				lineManager_->DrawLine(corner1Start, corner1Start - cameraUp * cornerLength, progressColor_);
			}

			// 右上コーナー
			if (cornersToShow >= 2) {
				Vector3 corner2Start = enemyPos + cameraRight * cornerSize + cameraUp * cornerSize;
				lineManager_->DrawLine(corner2Start, corner2Start - cameraRight * cornerLength, progressColor_);
				lineManager_->DrawLine(corner2Start, corner2Start - cameraUp * cornerLength, progressColor_);
			}

			// 右下コーナー
			if (cornersToShow >= 3) {
				Vector3 corner3Start = enemyPos + cameraRight * cornerSize - cameraUp * cornerSize;
				lineManager_->DrawLine(corner3Start, corner3Start - cameraRight * cornerLength, progressColor_);
				lineManager_->DrawLine(corner3Start, corner3Start + cameraUp * cornerLength, progressColor_);
			}

			// 左下コーナー
			if (cornersToShow >= 4) {
				Vector3 corner4Start = enemyPos - cameraRight * cornerSize - cameraUp * cornerSize;
				lineManager_->DrawLine(corner4Start, corner4Start + cameraRight * cornerLength, progressColor_);
				lineManager_->DrawLine(corner4Start, corner4Start + cameraUp * cornerLength, progressColor_);
			}
		}
	}
}

///=============================================================================
///						プレイヤーステータスバーの描画
void Hud::DrawPlayerStatusBars(ViewProjection viewProjection) {
	if (!player_ || !followCamera_) {
		return;
	}

	Vector3 playerPos = followCamera_->GetViewProjection().worldPosition_;
	Vector3 cameraForward = followCamera_->GetForwardDirection();
	Vector3 cameraRight = followCamera_->GetRightDirection();
	Vector3 cameraUp = followCamera_->GetUpDirection();

	// 画面の基準位置を計算（プレイヤーの前方、固定距離）
	Vector3 screenBasePos = playerPos + cameraForward * screenDistance_;

	// --- ブーストゲージ（画面左下） ---
	Vector3 boostBarCenter = screenBasePos + cameraRight * boostBarOffsetX_ + cameraUp * boostBarOffsetY_;
	float boostRatio = player_->GetCurrentBoostTime() / player_->GetMaxBoostTime();

	// 背景枠を描画（最大サイズ）
	DrawStatusBarFrame(boostBarCenter, statusBarWidth_, statusBarHeight_, gaugeBorderColor_, cameraRight, cameraUp);

	// ブーストゲージ本体（現在の値に応じてサイズが変化）
	DrawStatusBar(boostBarCenter, statusBarWidth_ * boostRatio, statusBarHeight_,
				  boostGaugeColor_, cameraRight, cameraUp);

	// --- マシンガンステータス（画面右下） ---
	Vector3 mgBarCenter = screenBasePos + cameraRight * mgBarOffsetX_ + cameraUp * mgBarOffsetY_;

	// 熱量ゲージ
	float heatRatio = player_->GetHeatLevel() / player_->GetMaxHeat();
	Vector4 heatColor = player_->IsOverheated() ? overheatColor_ : (heatRatio > 0.7f ? hudAlertColor_ : weaponHeatColor_);

	// 背景枠
	DrawStatusBarFrame(mgBarCenter, statusBarWidth_, statusBarHeight_, gaugeBorderColor_, cameraRight, cameraUp);

	// 熱量ゲージ本体（現在の値に応じてサイズが変化）
	DrawStatusBar(mgBarCenter, statusBarWidth_ * heatRatio, statusBarHeight_,
				  heatColor, cameraRight, cameraUp);

	// マシンガンクールダウン表示（熱量バーの上）
	if (player_->GetMachineGunCooldown() > 0 && !player_->IsOverheated()) {
		float mgCooldownRatio = 1.0f - (static_cast<float>(player_->GetMachineGunCooldown()) /
										static_cast<float>(Player::GetMachineGunFireInterval()));
		Vector3 mgCooldownPos = mgBarCenter + cameraUp * (statusBarHeight_ + weaponCooldownSpacing_);

		// クールダウンの背景枠
		DrawStatusBarFrame(mgCooldownPos, statusBarWidth_, weaponCooldownHeight_, gaugeBorderColor_, cameraRight, cameraUp);

		// クールダウンゲージ（進行度に応じてサイズが変化）
		DrawStatusBar(mgCooldownPos, statusBarWidth_ * mgCooldownRatio, weaponCooldownHeight_,
					  weaponReloadColor_, cameraRight, cameraUp);
	}

	// オーバーヒート警告表示
	if (player_->IsOverheated()) {
		Vector3 warningPos = mgBarCenter + cameraUp * (statusBarHeight_ + weaponCooldownSpacing_);
		// 点滅効果
		float blinkAlpha = 0.5f + 0.5f * sinf(lockOnRotation_ * 8.0f);
		Vector4 warningColor = Vector4{overheatColor_.x, overheatColor_.y, overheatColor_.z, blinkAlpha};

		// 背景枠
		DrawStatusBarFrame(warningPos, statusBarWidth_, weaponCooldownHeight_, gaugeBorderColor_, cameraRight, cameraUp);

		// 警告ゲージ（フルサイズで点滅）
		DrawStatusBar(warningPos, statusBarWidth_, weaponCooldownHeight_, warningColor, cameraRight, cameraUp);
	}

	// --- ミサイルステータス（マシンガンバーの下） ---
	Vector3 missileBarCenter = mgBarCenter + cameraUp * missileBarSpacing_;
	float missileReloadRatio = 1.0f - (static_cast<float>(player_->GetMissileCooldown()) /
									   static_cast<float>(player_->GetMissileCooldownMax()));
	Vector4 missileColor = (player_->GetMissileCooldown() == 0) ? weaponReadyColor_ : weaponReloadColor_;

	// 背景枠
	DrawStatusBarFrame(missileBarCenter, statusBarWidth_, statusBarHeight_, gaugeBorderColor_, cameraRight, cameraUp);

	// ミサイルリロードゲージ本体（現在の値に応じてサイズが変化）
	DrawStatusBar(missileBarCenter, statusBarWidth_ * missileReloadRatio, statusBarHeight_,
				  missileColor, cameraRight, cameraUp);

	// ミサイル準備完了表示
	if (player_->GetMissileCooldown() == 0) {
		Vector3 readyPos = missileBarCenter + cameraUp * (statusBarHeight_ + missileReadySpacing_);
		// 緩やかな点滅
		float readyAlpha = 0.7f + 0.3f * sinf(lockOnRotation_ * 3.0f);
		Vector4 readyColor = Vector4{weaponReadyColor_.x, weaponReadyColor_.y, weaponReadyColor_.z, readyAlpha};

		// 背景枠
		DrawStatusBarFrame(readyPos, statusBarWidth_, missileReadyHeight_, gaugeBorderColor_, cameraRight, cameraUp);

		// 準備完了ゲージ（フルサイズで点滅）
		DrawStatusBar(readyPos, statusBarWidth_, missileReadyHeight_, readyColor, cameraRight, cameraUp);
	}
}

///=============================================================================
///						プレイヤーヘルスバーの描画
void Hud::DrawPlayerHealthBar(ViewProjection viewProjection) {
	if (!player_ || !followCamera_) {
		return;
	}

	Vector3 playerPos = followCamera_->GetViewProjection().worldPosition_;
	Vector3 cameraForward = followCamera_->GetForwardDirection();
	Vector3 cameraRight = followCamera_->GetRightDirection();
	Vector3 cameraUp = followCamera_->GetUpDirection();

	// 画面上部の基準位置を計算
	Vector3 healthBarCenter = playerPos + cameraForward * screenDistance_ +
							  cameraRight * healthBarOffsetX_ +
							  cameraUp * healthBarOffsetY_;

	// プレイヤーの現在HPを取得
	int currentHp = player_->GetHp();
	int maxHp = 10; // プレイヤーの最大HPを定数として設定（Player.hから取得できる場合は変更）

	// HP比率を計算
	float healthRatio = static_cast<float>(currentHp) / static_cast<float>(maxHp);
	if (healthRatio < 0.0f)
		healthRatio = 0.0f;
	if (healthRatio > 1.0f)
		healthRatio = 1.0f;

	// HP比率に応じて色を決定
	Vector4 healthColor;
	if (healthRatio > 0.7f) {
		// HP高い（70%以上）：緑色
		healthColor = healthHighColor_;
	} else if (healthRatio > 0.3f) {
		// HP中程度（30%～70%）：黄色
		healthColor = healthMidColor_;
	} else {
		// HP低い（30%未満）：赤色で点滅
		float blinkFactor = 0.5f + 0.5f * sinf(lockOnRotation_ * 6.0f);
		healthColor = Vector4{
			healthLowColor_.x,
			healthLowColor_.y,
			healthLowColor_.z,
			healthLowColor_.w * blinkFactor};
	}

	// 背景枠を描画（最大サイズ）
	DrawStatusBarFrame(healthBarCenter, healthBarWidth_, healthBarHeight_,
					   gaugeBorderColor_, cameraRight, cameraUp);

	// ヘルスバー本体（現在のHPに応じてサイズが変化）
	DrawStatusBar(healthBarCenter, healthBarWidth_ * healthRatio, healthBarHeight_,
				  healthColor, cameraRight, cameraUp);

	// セグメント分割線の描画（10分割でHPを視覚的に分かりやすく）
	for (int i = 1; i < maxHp; ++i) {
		float segmentX = (static_cast<float>(i) / static_cast<float>(maxHp) - 0.5f) * healthBarWidth_;
		Vector3 segmentStart = healthBarCenter + cameraRight * segmentX -
							   cameraUp * (healthBarHeight_ * 0.6f);
		Vector3 segmentEnd = healthBarCenter + cameraRight * segmentX +
							 cameraUp * (healthBarHeight_ * 0.6f);

		Vector4 segmentColor = Vector4{gaugeBorderColor_.x, gaugeBorderColor_.y,
									   gaugeBorderColor_.z, gaugeBorderColor_.w * 0.8f};
		lineManager_->DrawLine(segmentStart, segmentEnd, segmentColor);
	}
}

void Hud::DrawImGui() {
	ImGui::Begin("HUD Settings");

	ImGui::Checkbox("Combat Mode", &isCombatMode_);
	ImGui::Checkbox("Use 3D Sphere Map", &use3DSphereMap_);

	if (ImGui::CollapsingHeader("Sphere Map Settings")) {
		ImGui::SliderFloat("Radius", &sphereMapRadius_, 1.0f, 30.0f);
		ImGui::SliderFloat("Position X", &sphereMapPositionX_, -30.0f, 30.0f);
		ImGui::SliderFloat("Position Y", &sphereMapPositionY_, -30.0f, 30.0f);
		ImGui::SliderFloat("Position Z", &sphereMapPositionZ_, -30.0f, 30.0f);
		ImGui::SliderFloat("Range", &sphereMapRange_, 50.0f, 1000.0f);
	}

	if (ImGui::CollapsingHeader("Status Bar Settings")) {
		ImGui::SliderFloat("Screen Distance", &screenDistance_, 30.0f, 100.0f);
		ImGui::SliderFloat("Status Bar Width", &statusBarWidth_, 4.0f, 15.0f);
		ImGui::SliderFloat("Boost Bar Offset X", &boostBarOffsetX_, -30.0f, -10.0f);
		ImGui::SliderFloat("MG Bar Offset X", &mgBarOffsetX_, 5.0f, 15.0f);
	}

	if (ImGui::CollapsingHeader("Health Bar Settings")) {
		ImGui::SliderFloat("Health Bar Offset X", &healthBarOffsetX_, -15.0f, 15.0f);
		ImGui::SliderFloat("Health Bar Offset Y", &healthBarOffsetY_, 15.0f, 30.0f);
		ImGui::SliderFloat("Health Bar Width", &healthBarWidth_, 10.0f, 30.0f);
		ImGui::SliderFloat("Health Bar Height", &healthBarHeight_, 0.5f, 2.0f);
	}

	ImGui::End();
};

// カメラに正対する円を描画する補助関数
void Hud::DrawFacingCircle(const Vector3 &center, float radius, const Vector4 &color, int segments, const Vector3 &cameraForward) {
	// カメラの前方向に垂直な平面上に円を描画
	Vector3 up = {0.0f, 1.0f, 0.0f};
	// カメラの前方向とほぼ平行な場合は別の基準軸を使用
	if (fabsf(cameraForward.y) > 0.99f) {
		up = {0.0f, 0.0f, 1.0f};
	}

	Vector3 right = Cross(up, cameraForward);
	Normalize(right);

	up = Cross(cameraForward, right);
	Normalize(up);

	// 円を描く
	for (int i = 0; i < segments; i++) {
		float angle1 = i * kTwoPi / segments;
		float angle2 = (i + 1) * kTwoPi / segments;

		Vector3 point1 = {
			center.x + (right.x * cosf(angle1) + up.x * sinf(angle1)) * radius,
			center.y + (right.y * cosf(angle1) + up.y * sinf(angle1)) * radius,
			center.z + (right.z * cosf(angle1) + up.z * sinf(angle1)) * radius};

		Vector3 point2 = {
			center.x + (right.x * cosf(angle2) + up.x * sinf(angle2)) * radius,
			center.y + (right.y * cosf(angle2) + up.y * sinf(angle2)) * radius,
			center.z + (right.z * cosf(angle2) + up.z * sinf(angle2)) * radius};

		lineManager_->DrawLine(point1, point2, color);
	}
}

// カメラに正対する四角形を描画する補助関数
void Hud::DrawFacingSquare(const Vector3 &center, float size, const Vector4 &color, const Vector3 &cameraRight, const Vector3 &cameraUp) {
	// 四角形の各頂点を計算
	Vector3 topLeft = center - cameraRight * size + cameraUp * size;
	Vector3 topRight = center + cameraRight * size + cameraUp * size;
	Vector3 bottomLeft = center - cameraRight * size - cameraUp * size;
	Vector3 bottomRight = center + cameraRight * size - cameraUp * size;

	// 四角形の4辺を描画
	lineManager_->DrawLine(topLeft, topRight, color);
	lineManager_->DrawLine(topRight, bottomRight, color);
	lineManager_->DrawLine(bottomRight, bottomLeft, color);
	lineManager_->DrawLine(bottomLeft, topLeft, color);
}

// カメラに正対する多角形を描画する補助関数
void Hud::DrawFacingPolygon(const Vector3 &center, float size, int segments, const Vector4 &color, const Vector3 &cameraForward) {
	// カメラの前方向に垂直な平面上に多角形を描画
	Vector3 up = {0.0f, 1.0f, 0.0f};
	if (fabsf(cameraForward.y) > 0.99f) {
		up = {0.0f, 0.0f, 1.0f};
	}

	Vector3 right = Cross(up, cameraForward);
	Normalize(right);
	up = Cross(cameraForward, right);
	Normalize(up);

	// 多角形を描く
	std::vector<Vector3> points;
	for (int i = 0; i < segments; i++) {
		float angle = i * kTwoPi / segments;
		Vector3 point = center + right * (cosf(angle) * size) + up * (sinf(angle) * size);
		points.push_back(point);
	}

	// 各頂点を線で結ぶ
	for (int i = 0; i < segments; i++) {
		lineManager_->DrawLine(points[i], points[(i + 1) % segments], color);
	}
}

// カメラに正対する三角形を描画する補助関数
void Hud::DrawFacingTriangle(const Vector3 &center, float size, const Vector4 &color, const Vector3 &cameraForward) {
	// カメラの前方向に垂直な平面上に三角形を描画
	Vector3 up = {0.0f, 1.0f, 0.0f};
	if (fabsf(cameraForward.y) > 0.99f) {
		up = {0.0f, 0.0f, 1.0f};
	}

	Vector3 right = Cross(up, cameraForward);
	Normalize(right);
	up = Cross(cameraForward, right);
	Normalize(up);

	// 三角形の頂点を計算（上向き三角形）
	Vector3 top = center + up * size;
	Vector3 bottomLeft = center - right * (size * 0.866f) - up * (size * 0.5f);
	Vector3 bottomRight = center + right * (size * 0.866f) - up * (size * 0.5f);

	// 三角形を描画
	lineManager_->DrawLine(top, bottomLeft, color);
	lineManager_->DrawLine(bottomLeft, bottomRight, color);
	lineManager_->DrawLine(bottomRight, top, color);
}

// 2Dステータスバーを描画する補助関数
void Hud::DrawStatusBar(const Vector3 &center, float width, float height, const Vector4 &color,
						const Vector3 &cameraRight, const Vector3 &cameraUp) {
	// バーの四隅を計算
	float halfWidth = width * 0.5f;
	float halfHeight = height * 0.5f;

	Vector3 topLeft = center - cameraRight * halfWidth + cameraUp * halfHeight;
	Vector3 topRight = center + cameraRight * halfWidth + cameraUp * halfHeight;
	Vector3 bottomLeft = center - cameraRight * halfWidth - cameraUp * halfHeight;
	Vector3 bottomRight = center + cameraRight * halfWidth - cameraUp * halfHeight;

	// バーを塗りつぶすように線で描画
	int fillLines = static_cast<int>(height * 15.0f);
	for (int i = 0; i < fillLines; ++i) {
		float t = static_cast<float>(i) / static_cast<float>(fillLines - 1);
		Vector3 left;
		left.x = fLerp(bottomLeft.x, topLeft.x, t);
		left.y = fLerp(bottomLeft.y, topLeft.y, t);
		left.z = fLerp(bottomLeft.z, topLeft.z, t);
		Vector3 right;
		right.x = fLerp(bottomRight.x, topRight.x, t);
		right.y = fLerp(bottomRight.y, topRight.y, t);
		right.z = fLerp(bottomRight.z, topRight.z, t);
		lineManager_->DrawLine(left, right, color);
	}
}

// ステータスバーの背景枠を描画する補助関数
void Hud::DrawStatusBarFrame(const Vector3 &center, float width, float height, const Vector4 &color,
							 const Vector3 &cameraRight, const Vector3 &cameraUp) {
	// 外枠の四隅を計算（少し大きめに）
	float halfWidth = width * 0.5f + gaugeBorderWidth_;
	float halfHeight = height * 0.5f + gaugeBorderWidth_;

	Vector3 topLeft = center - cameraRight * halfWidth + cameraUp * halfHeight;
	Vector3 topRight = center + cameraRight * halfWidth + cameraUp * halfHeight;
	Vector3 bottomLeft = center - cameraRight * halfWidth - cameraUp * halfHeight;
	Vector3 bottomRight = center + cameraRight * halfWidth - cameraUp * halfHeight;

	// 外枠を描画
	lineManager_->DrawLine(topLeft, topRight, color);
	lineManager_->DrawLine(topRight, bottomRight, color);
	lineManager_->DrawLine(bottomRight, bottomLeft, color);
	lineManager_->DrawLine(bottomLeft, topLeft, color);
}

///=============================================================================
///						3D球体ミニマップの描画
void Hud::DrawSphereMap(ViewProjection viewProjection) {
	// プレイヤーの位置を取得
	Vector3 playerPos = followCamera_->GetViewProjection().worldPosition_;

	// カメラの位置と向きを取得
	Vector3 cameraForward = followCamera_->GetForwardDirection();
	Vector3 cameraRight = followCamera_->GetRightDirection();
	Vector3 cameraUp = followCamera_->GetUpDirection();

	// 球体マップの位置を計算
	Vector3 sphereCenter = {
		playerPos.x + cameraRight.x * sphereMapPositionX_ + cameraUp.x * sphereMapPositionY_ + cameraForward.x * sphereMapPositionZ_,
		playerPos.y + cameraRight.y * sphereMapPositionX_ + cameraUp.y * sphereMapPositionY_ + cameraForward.y * sphereMapPositionZ_,
		playerPos.z + cameraRight.z * sphereMapPositionX_ + cameraUp.z * sphereMapPositionY_ + cameraForward.z * sphereMapPositionZ_};

	// 球体グリッドを描画
	DrawSphereGrid(sphereCenter, sphereMapRadius_, sphereGridColor_);

	// プレイヤーの位置を中心にマーカーを表示
	DrawFacingCircle(sphereCenter, 0.8f, hudBaseColor_, 8, cameraForward);

	// 視界扇形の描画
	float halfFov = sphereMapFov_ / 2.0f;
	int fovSegments = 10;

	// カメラの前方ベクトルをXZ平面に投影して正規化
	Vector3 forwardXZ = {cameraForward.x, 0.0f, cameraForward.z};
	Normalize(forwardXZ);
	if (Length(forwardXZ) < 0.001f) {
		forwardXZ = {1.0f, 0.0f, 0.0f};
	}

	Vector3 firstFovPointOnSphere;
	Vector3 prevFovPointOnSphere;

	for (int i = 0; i <= fovSegments; ++i) {
		float currentAngle = -halfFov + (sphereMapFov_ * static_cast<float>(i) / static_cast<float>(fovSegments));

		// 回転行列を作成
		Matrix4x4 rotationMatrix = MakeRotateYMatrix(atan2f(forwardXZ.x, forwardXZ.z) + currentAngle);

		// Z軸基本ベクトルを回転させて方向ベクトルを得る
		Vector3 fovDirection = {0.0f, 0.0f, 1.0f};
		fovDirection = TransformNormal(fovDirection, rotationMatrix);
		Normalize(fovDirection);

		Vector3 fovPointOnSphere = {
			sphereCenter.x + fovDirection.x * sphereMapRadius_,
			sphereCenter.y,
			sphereCenter.z + fovDirection.z * sphereMapRadius_};

		if (i == 0) {
			firstFovPointOnSphere = fovPointOnSphere;
		} else {
			lineManager_->DrawLine(prevFovPointOnSphere, fovPointOnSphere, sphereMapFovColor_);
		}
		prevFovPointOnSphere = fovPointOnSphere;
	}

	// 扇の始点と終点を中心と結ぶ
	lineManager_->DrawLine(sphereCenter, firstFovPointOnSphere, sphereMapFovColor_);
	lineManager_->DrawLine(sphereCenter, prevFovPointOnSphere, sphereMapFovColor_);

	// 敵を球体上に表示
	if (enemies_ && !enemies_->empty()) {
		for (const auto &enemy : *enemies_) {
			if (enemy) {
				Vector3 enemyPos = enemy->GetPosition();
				Vector3 sphereEnemyPos = WorldToSpherePosition(enemyPos, sphereCenter, sphereMapRadius_, sphereMapRange_);

				if (sphereEnemyPos.x != 0.0f || sphereEnemyPos.y != 0.0f || sphereEnemyPos.z != 0.0f) {
					Vector3 enemyDirection = enemyPos - playerPos;

					if (lockOnSystem_ && lockOnSystem_->IsLocked(enemy.get())) {
						float pulseScale = 1.0f + 0.3f * sinf(lockOnRotation_ * 5.0f);
						Draw3DTriangle(sphereEnemyPos, sphereObjectScale_ * 2.0f * pulseScale, hudLockColor_, enemyDirection);
					} else {
						Draw3DTriangle(sphereEnemyPos, sphereObjectScale_ * 1.5f, enemyDotColor_, enemyDirection);
					}
				}
			}
		}
	}

	// スポーンブロックを球体上に表示
	if (spawns_ && !spawns_->empty()) {
		for (const auto &spawn : *spawns_) {
			if (spawn) {
				Vector3 spawnPos = spawn->GetPosition();
				Vector3 sphereSpawnPos = WorldToSpherePosition(spawnPos, sphereCenter, sphereMapRadius_, sphereMapRange_);

				if (sphereSpawnPos.x != 0.0f || sphereSpawnPos.y != 0.0f || sphereSpawnPos.z != 0.0f) {
					float blinkFactor = 0.5f + 0.5f * sinf(lockOnRotation_ * 4.0f);
					Vector4 blinkColor = {
						spawnBlockColor_.x,
						spawnBlockColor_.y * blinkFactor,
						spawnBlockColor_.z,
						spawnBlockColor_.w};

					Draw3DHexagon(sphereSpawnPos, sphereObjectScale_ * 1.8f, blinkColor, spawnPos - playerPos);
				}
			}
		}
	}
}

// 世界座標を球体マップ上の座標に変換する関数
Vector3 Hud::WorldToSpherePosition(const Vector3 &worldPos, const Vector3 &sphereCenter, float radius, float maxRange) {
	Vector3 playerPos = followCamera_->GetViewProjection().worldPosition_;

	Vector3 relativePos = {
		worldPos.x - playerPos.x,
		worldPos.y - playerPos.y,
		worldPos.z - playerPos.z};

	float distance = sqrtf(relativePos.x * relativePos.x + relativePos.y * relativePos.y + relativePos.z * relativePos.z);

	if (distance > maxRange) {
		return {0.0f, 0.0f, 0.0f};
	}

	Vector3 direction = {
		relativePos.x / distance,
		relativePos.y / distance,
		relativePos.z / distance};

	Vector3 spherePos = {
		sphereCenter.x + direction.x * radius,
		sphereCenter.y + direction.y * radius,
		sphereCenter.z + direction.z * radius};

	return spherePos;
}

// 3D空間に三角形を描画する関数
void Hud::Draw3DTriangle(const Vector3 &center, float size, const Vector4 &color, const Vector3 &direction) {
	Vector3 dir = direction;
	float length = sqrtf(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
	if (length > 0.0f) {
		dir.x /= length;
		dir.y /= length;
		dir.z /= length;
	} else {
		dir = {0.0f, 1.0f, 0.0f};
	}

	Vector3 up = {0.0f, 1.0f, 0.0f};
	if (fabsf(dir.y) > 0.99f) {
		up = {0.0f, 0.0f, 1.0f};
	}

	Vector3 right = Cross(up, dir);
	Normalize(right);
	up = Cross(dir, right);
	Normalize(up);

	Vector3 top = center + dir * size;
	Vector3 bottomLeft = center - right * (size * 0.5f) - dir * (size * 0.5f);
	Vector3 bottomRight = center + right * (size * 0.5f) - dir * (size * 0.5f);

	lineManager_->DrawLine(top, bottomLeft, color);
	lineManager_->DrawLine(bottomLeft, bottomRight, color);
	lineManager_->DrawLine(bottomRight, top, color);
}

// 3D空間に六角形を描画する関数
void Hud::Draw3DHexagon(const Vector3 &center, float size, const Vector4 &color, const Vector3 &direction) {
	Vector3 dir = direction;
	float length = sqrtf(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
	if (length > 0.0f) {
		dir.x /= length;
		dir.y /= length;
		dir.z /= length;
	} else {
		dir = {0.0f, 1.0f, 0.0f};
	}

	Vector3 up = {0.0f, 1.0f, 0.0f};
	if (fabsf(dir.y) > 0.99f) {
		up = {0.0f, 0.0f, 1.0f};
	}

	Vector3 right = Cross(up, dir);
	Normalize(right);
	up = Cross(dir, right);
	Normalize(up);

	const int segments = 6;
	std::vector<Vector3> vertices;

	for (int i = 0; i < segments; i++) {
		float angle = i * kTwoPi / segments;
		Vector3 vertex = center + right * (cosf(angle) * size) + up * (sinf(angle) * size);
		vertices.push_back(vertex);
	}

	for (int i = 0; i < segments; i++) {
		lineManager_->DrawLine(vertices[i], vertices[(i + 1) % segments], color);
	}
}

// 球体のグリッドを描画する関数
void Hud::DrawSphereGrid(const Vector3 &center, float radius, const Vector4 &color) {
	// 赤道のみを描画（中心の緯線1本だけ）
	for (int j = 0; j < 48; j++) {
		float lon1 = j * kTwoPi / 48.0f;
		float lon2 = (j + 1) * kTwoPi / 48.0f;

		Vector3 p1 = {
			center.x + radius * cosf(lon1),
			center.y,
			center.z + radius * sinf(lon1)};

		Vector3 p2 = {
			center.x + radius * cosf(lon2),
			center.y,
			center.z + radius * sinf(lon2)};

		Vector4 equatorColor = {color.x * 1.5f, color.y * 1.5f, color.z * 1.5f, color.w};
		lineManager_->DrawLine(p1, p2, equatorColor);
	}
}
