#pragma once
#include "FollowCamera.h"
#include "LineManager.h"
#include "LockOn.h"
#include "Matrixs.h"
#include "MyMath.h"
#include "Player.h"

///=============================================================================
///						Hudクラス
class Hud {
public:
	///=============================================================================
	///                        メンバ関数
	// 初期化
	void Initialize(FollowCamera *followCamera, Player *player, LockOn *lockOnSystem);

	// 更新
	void Update();

	// 描画
	void Draw(ViewProjection viewProjection);

	void DrawImGui();

	// 戦闘モード切り替え
	void CombatModeActive();
	void CombatModeDeactive();

	// 敵コンテナとスポーンブロックの設定
	void SetEnemiesAndSpawns(const std::vector<std::unique_ptr<BaseEnemy>> *enemies,
							 const std::vector<std::unique_ptr<BaseEnemy>> *spawns);

private:
	// 照準の描画
	void DrawCrosshair(ViewProjection viewProjection);

	// ロックオンの描画
	void DrawLockOn(ViewProjection viewProjection);

	// 3D球体ミニマップの描画
	void DrawSphereMap(ViewProjection viewProjection);

	// プレイヤーステータスバーの描画
	void DrawPlayerStatusBars(ViewProjection viewProjection);

	// プレイヤーヘルスバーの描画
	void DrawPlayerHealthBar(ViewProjection viewProjection);

	// カメラに正対する円を描画する補助関数
	void DrawFacingCircle(const Vector3 &center, float radius, const Vector4 &color, int segments, const Vector3 &cameraForward);

	// カメラに正対する四角形を描画する補助関数
	void DrawFacingSquare(const Vector3 &center, float size, const Vector4 &color, const Vector3 &cameraRight, const Vector3 &cameraUp);

	// カメラに正対する多角形を描画する補助関数
	void DrawFacingPolygon(const Vector3 &center, float size, int segments, const Vector4 &color, const Vector3 &cameraForward);

	// カメラに正対する三角形を描画する補助関数
	void DrawFacingTriangle(const Vector3 &center, float size, const Vector4 &color, const Vector3 &cameraForward);

	// 2Dステータスバーを描画する補助関数
	void DrawStatusBar(const Vector3 &center, float width, float height, const Vector4 &color,
					   const Vector3 &cameraRight, const Vector3 &cameraUp);

	// ステータスバーの背景枠を描画する補助関数
	void DrawStatusBarFrame(const Vector3 &center, float width, float height, const Vector4 &color,
							const Vector3 &cameraRight, const Vector3 &cameraUp);

	// 3D球体ミニマップ関連の関数
	Vector3 WorldToSpherePosition(const Vector3 &worldPos, const Vector3 &sphereCenter, float radius, float maxRange);
	void Draw3DTriangle(const Vector3 &center, float size, const Vector4 &color, const Vector3 &direction);
	void Draw3DHexagon(const Vector3 &center, float size, const Vector4 &color, const Vector3 &direction);
	void DrawSphereGrid(const Vector3 &center, float radius, const Vector4 &color);

	///=============================================================================
	///                        メンバ変数
private:
	// 各種ポインタ
	FollowCamera *followCamera_ = nullptr;
	Player *player_ = nullptr;
	LockOn *lockOnSystem_ = nullptr;
	LineManager *lineManager_ = nullptr;
	const std::vector<std::unique_ptr<BaseEnemy>> *enemies_ = nullptr;
	const std::vector<std::unique_ptr<BaseEnemy>> *spawns_ = nullptr;

	// 数学定数
	static constexpr float kPi = 3.14159f;
	static constexpr float kTwoPi = 2.0f * kPi;

	// 戦闘モード制御
	bool isCombatMode_ = false; // HUD全体の表示/非表示を制御

	// HUDの基本色設定（UI全体の統一感を保つ）
	Vector4 hudBaseColor_ = {0.0f, 1.0f, 0.4f, 0.9f};	// メイン色（緑系）
	Vector4 hudAccentColor_ = {0.2f, 1.0f, 0.8f, 0.9f}; // アクセント色（シアン系）
	Vector4 hudEnemyColor_ = {1.0f, 0.3f, 0.0f, 0.9f};	// 敵マーカー色（赤橙）
	Vector4 hudAlertColor_ = {1.0f, 0.8f, 0.0f, 0.9f};	// 警告色（黄色）
	Vector4 hudLockColor_ = {1.0f, 0.0f, 0.0f, 0.9f};	// ロック完了色（赤）

	// 照準（クロスヘア）関連設定
	float crosshairSize_ = 2.5f;			 // 照準の基本サイズ
	Vector4 crosshairColor_ = hudBaseColor_; // 照準の色
	float crosshairDistance_ = 64.0f;		 // プレイヤーから照準までの表示距離
	float crosshairGap_ = 8.0f;				 // 中心からマーカーまでのギャップ
	float crosshairCenterRadius_ = 0.8f;	 // 中央円の半径
	int crosshairCircleSegments_ = 16;		 // 円の分割数（滑らかさ）

	// ロックオン表示関連設定
	float lockOnSize_ = 5.0f;						   // ロックオンマーカーの基本サイズ
	Vector4 lockOnColor_ = hudLockColor_;			   // ロックオンマーカーの基本色
	Vector4 preciseLockonColor_ = hudLockColor_;	   // 精密ロック時の色（赤）
	Vector4 simpleLockonColor_ = hudAlertColor_;	   // 簡易ロック時の色（黄）
	Vector4 noLockonColor_ = {0.5f, 0.8f, 0.5f, 0.7f}; // ロックなし時の色（薄緑）
	Vector4 progressColor_ = hudAccentColor_;		   // ロック進行度の色（シアン）
	float preciseLockonSizeRatio_ = 0.6f;			   // 精密ロック時のサイズ倍率
	float simpleLockonSizeRatio_ = 0.4f;			   // 簡易ロック時のサイズ倍率
	float noLockonSizeRatio_ = 0.2f;				   // ロックなし時のサイズ倍率
	float lockOnRotation_ = 0.0f;					   // ロックオン表示の回転角度
	float lockOnRotationSpeed_ = 0.02f;				   // ロックオン表示の回転速度

	// 3D球体ミニマップ関連設定
	bool use3DSphereMap_ = true;						   // 3D球体ミニマップの使用フラグ
	float sphereMapRadius_ = 6.0f;						   // 球体の半径
	float sphereMapPositionX_ = 13.0f;					   // 球体の表示位置X（右側）
	float sphereMapPositionY_ = -5.0f;					   // 球体の表示位置Y（下側）
	float sphereMapPositionZ_ = 30.0f;					   // 球体の表示位置Z（前方）
	float sphereMapRange_ = 1000.0f;					   // ミニマップの検出範囲
	float sphereObjectScale_ = 0.4f;					   // 球体上のオブジェクトサイズ
	Vector4 sphereMapFovColor_ = {0.5f, 0.8f, 1.0f, 0.3f}; // 視界扇形の色
	float sphereMapFov_ = 60.0f * (kPi / 180.0f);		   // 視界角度（ラジアン）
	Vector4 enemyDotColor_ = hudEnemyColor_;			   // 敵ドットの色
	Vector4 spawnBlockColor_ = hudAlertColor_;			   // スポーンブロックの色
	Vector4 sphereGridColor_ = {0.0f, 0.8f, 0.4f, 0.6f};   // 球体グリッドの色
	int sphereLongitudeCount_ = 8;						   // 経線の数（縦線）
	int sphereLatitudeCount_ = 4;						   // 緯線の数（横線）

	// プレイヤーステータスバー基本設定
	float screenDistance_ = 65.0f; // HUD表示距離（プレイヤーからの距離）
	float statusBarWidth_ = 14.0f; // ステータスバーの幅
	float statusBarHeight_ = 0.8f; // ステータスバーの高さ

	// ブーストゲージ位置設定（画面左下）
	float boostBarOffsetX_ = -35.0f; // ブーストバーのX位置（左側）
	float boostBarOffsetY_ = -18.0f; // ブーストバーのY位置（下側）

	// ミサイルゲージ位置設定（画面右下）
	float mgBarOffsetX_ = 0.0f;			 // マシンガンバーのX位置（右側）
	float mgBarOffsetY_ = -18.0f;		 // マシンガンバーのY位置（下側）
	float weaponCooldownSpacing_ = 1.2f; // 武器クールダウンとの間隔
	float weaponCooldownHeight_ = 0.5f;	 // 武器クールダウンの高さ
	float missileBarSpacing_ = -3.5f;	 // ミサイルバーとマシンガンバーの間隔
	float missileReadySpacing_ = 1.0f;	 // ミサイル準備完了表示との間隔
	float missileReadyHeight_ = 0.3f;	 // ミサイル準備完了表示の高さ

	// ヘルスバー位置設定（画面上部）
	float healthBarOffsetX_ = 0.0f;	 // ヘルスバーのX位置（中央）
	float healthBarOffsetY_ = 22.0f; // ヘルスバーのY位置（上部）
	float healthBarWidth_ = 20.0f;	 // ヘルスバーの幅
	float healthBarHeight_ = 1.2f;	 // ヘルスバーの高さ

	// ゲージ色設定（各種ステータス表示）
	Vector4 boostGaugeColor_ = {0.2f, 0.8f, 1.0f, 0.8f};   // ブーストゲージの色（青系）
	Vector4 weaponReadyColor_ = {0.2f, 1.0f, 0.5f, 0.8f};  // 武器準備完了時の色（緑）
	Vector4 weaponReloadColor_ = {1.0f, 0.6f, 0.2f, 0.7f}; // 武器リロード中の色（橙）
	Vector4 weaponHeatColor_ = {1.0f, 0.8f, 0.0f, 0.8f};   // 武器発熱時の色（黄）
	Vector4 overheatColor_ = {1.0f, 0.2f, 0.1f, 0.9f};	   // オーバーヒート時の色（赤）
	Vector4 healthHighColor_ = {0.2f, 1.0f, 0.4f, 0.9f};   // ヘルス高い時の色（緑）
	Vector4 healthMidColor_ = {1.0f, 0.8f, 0.2f, 0.9f};	   // ヘルス中程度の色（黄）
	Vector4 healthLowColor_ = {1.0f, 0.2f, 0.1f, 0.9f};	   // ヘルス低い時の色（赤）
	Vector4 gaugeBorderColor_ = {0.3f, 0.6f, 0.3f, 0.6f};  // ゲージ背景枠の色
	float gaugeBorderWidth_ = 0.1f;						   // ゲージ背景枠の幅
};