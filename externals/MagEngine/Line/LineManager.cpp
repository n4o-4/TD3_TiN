/*********************************************************************
* \file   LineManager.cpp
* \brief
*
* \author Harukichimaru
* \date   January 2025
* \note
*********************************************************************/
#include "LineManager.h"
#include "ImGuiManager.h"
//========================================
// 数学関数のインクルード
#define _USE_MATH_DEFINES
#include <math.h>

///=============================================================================
///						インスタンス
LineManager *LineManager::instance_ = nullptr;

///=============================================================================
///						インスタンス設定
LineManager *LineManager::GetInstance() {
	if(instance_ == nullptr) {
		instance_ = new LineManager();
	}
	return instance_;
}

///=============================================================================
///						初期化
void LineManager::Initialize(DirectXCommon *dxCommon, SrvManager *srvManager) {
	//========================================
	// DirectXCommonの取得
	dxCommon_ = dxCommon;
	// SrvSetupの取得
	srvManager_ = srvManager;
	//========================================
	// ラインセットアップの生成
	lineSetup_ = std::make_unique<LineSetup>();
	// ラインセットアップの初期化
	lineSetup_->Initialize(dxCommon_, nullptr);
	//========================================
	// ラインの初期化
	line_ = std::make_unique<Line>();
	// ラインの初期化
	line_->Initialize(lineSetup_.get());
}

///=============================================================================
///						終了処理
void LineManager::Finalize() {
	//インスタンスの削除
	delete instance_;
	instance_ = nullptr;
}

///=============================================================================
///						更新処理
void LineManager::Update() {
	//Gridの描画
	//DrawGrid(gridSize_, gridDivisions_, gridColor_);
	// ラインの更新
	line_->Update();
}

///=============================================================================
///						ラインの描画
void LineManager::Draw() {
	//========================================
	// 共通描画設定
	lineSetup_->CommonDrawSetup();
	// ラインの描画
	line_->Draw();

	//========================================
	// ラインのクリア
	line_->ClearLines();
}

///=============================================================================
///						Imguiの描画
void LineManager::DrawImGui() {
#ifdef _DEBUG
	//========================================
	// 描画設定
	ImGui::Begin("LineManager");
	//========================================
	// Lineを描画するか
	ImGui::Checkbox("Line", &isDrawLine_);
	ImGui::Separator();
	//========================================
	// Gridの描画
	ImGui::Checkbox("Grid", &isDrawGrid_);
	//Gridの設定
	ImGui::SliderFloat("GridSize", &gridSize_, 1.0f, 10000.0f);
	ImGui::SliderInt("Divisions", &gridDivisions_, 1, 512);
	//色
	ImGui::ColorEdit4("Color", &gridColor_.x);
	//セパレーター
	ImGui::Separator();
	//========================================
	// Sphereの描画
	ImGui::Checkbox("Sphere", &isDrawSphere_);
	
	ImGui::End();
#endif // _DEBUG
}

///=============================================================================
///						ラインのクリア
void LineManager::ClearLines() {
	// ラインのクリア
	line_->ClearLines();
}

///=============================================================================
///						ラインの追加
void LineManager::DrawLine(const Vector3 &start, const Vector3 &end, const Vector4 &color) {
	if(!isDrawLine_) {
		return;
	}
	// ラインの追加
	line_->DrawLine(start, end, color);
}

///=============================================================================
///						グリッドの描画
void LineManager::DrawGrid(float gridSize, int divisions, const Vector4 &color) {
	if(!isDrawGrid_ || divisions <= 0) {
		return;
	}
	float halfSize = gridSize * 0.5f;
	float step = gridSize / divisions;

	for(int i = 0; i <= divisions; ++i) {
		float offset = -halfSize + ( i * step );

		// X軸に平行な線
		DrawLine(Vector3(-halfSize, 0.0f, offset), Vector3(halfSize, 0.0f, offset), color);

		// Z軸に平行な線
		DrawLine(Vector3(offset, 0.0f, -halfSize), Vector3(offset, 0.0f, halfSize), color);
	}
}

///=============================================================================
///						球体の描画
void LineManager::DrawSphere(const Vector3 &center, float radius, const Vector4 &color, int divisions) {
	if(!isDrawSphere_ || divisions <= 0) {
		return;
	}
	float angleStep = 2.0f * static_cast<float>( M_PI ) / divisions;

	// XY, XZ, YZ 平面の円を描画
	for(int i = 0; i < divisions; ++i) {
		float angle1 = angleStep * i;
		float angle2 = angleStep * ( i + 1 );

		// XY 平面の円
		DrawLine(
			Vector3(center.x + radius * cosf(angle1), center.y + radius * sinf(angle1), center.z),
			Vector3(center.x + radius * cosf(angle2), center.y + radius * sinf(angle2), center.z),
			color
		);

		// XZ 平面の円
		DrawLine(
			Vector3(center.x + radius * cosf(angle1), center.y, center.z + radius * sinf(angle1)),
			Vector3(center.x + radius * cosf(angle2), center.y, center.z + radius * sinf(angle2)),
			color
		);

		// YZ 平面の円
		DrawLine(
			Vector3(center.x, center.y + radius * cosf(angle1), center.z + radius * sinf(angle1)),
			Vector3(center.x, center.y + radius * cosf(angle2), center.z + radius * sinf(angle2)),
			color
		);
	}

	// 緯度方向の分割を追加
	for(int lat = 1; lat < divisions / 2; ++lat) {
		float latAngle1 = static_cast<float>( M_PI ) * lat / ( divisions / 2 );
		float latAngle2 = static_cast<float>( M_PI ) * ( lat + 1 ) / ( divisions / 2 );

		float r1 = radius * sinf(latAngle1);
		float r2 = radius * sinf(latAngle2);
		float y1 = center.y + radius * cosf(latAngle1);
		float y2 = center.y + radius * cosf(latAngle2);

		for(int i = 0; i < divisions; ++i) {
			float angle1 = angleStep * i;
			float angle2 = angleStep * ( i + 1 );

			// 緯度方向の円
			DrawLine(
				Vector3(center.x + r1 * cosf(angle1), y1, center.z + r1 * sinf(angle1)),
				Vector3(center.x + r1 * cosf(angle2), y1, center.z + r1 * sinf(angle2)),
				color
			);

			DrawLine(
				Vector3(center.x + r2 * cosf(angle1), y2, center.z + r2 * sinf(angle1)),
				Vector3(center.x + r2 * cosf(angle2), y2, center.z + r2 * sinf(angle2)),
				color
			);
		}
	}

	// 経度方向の線を追加
	for(int lon = 0; lon < divisions; ++lon) {
		float lonAngle = angleStep * lon;
		float nextLonAngle = angleStep * ( lon + 1 );

		for(int lat = 0; lat <= divisions / 2; ++lat) {
			float latAngle = static_cast<float>( M_PI ) * lat / ( divisions / 2 );
			float nextLatAngle = static_cast<float>( M_PI ) * ( lat + 1 ) / ( divisions / 2 );

			float r1 = radius * sinf(latAngle);
			float r2 = radius * sinf(nextLatAngle);
			float y1 = center.y + radius * cosf(latAngle);
			float y2 = center.y + radius * cosf(nextLatAngle);

			DrawLine(
				Vector3(center.x + r1 * cosf(lonAngle), y1, center.z + r1 * sinf(lonAngle)),
				Vector3(center.x + r2 * cosf(lonAngle), y2, center.z + r2 * sinf(lonAngle)),
				color
			);
		}
	}
}

///=============================================================================
///						円の描画
void LineManager::DrawCircle(const Vector3 &center, float radius, const Vector4 &color, int divisions) {
	if(!isDrawLine_ || divisions <= 0) {
		return;
	}

	float angleStep = 2.0f * static_cast<float>( M_PI ) / divisions;

	// XZ平面上に円を描画（Y軸に垂直な円）
	for(int i = 0; i < divisions; ++i) {
		float angle1 = angleStep * i;
		float angle2 = angleStep * ( i + 1 );

		DrawLine(
			Vector3(center.x + radius * cosf(angle1), center.y, center.z + radius * sinf(angle1)),
			Vector3(center.x + radius * cosf(angle2), center.y, center.z + radius * sinf(angle2)),
			color
		);
	}
}

///=============================================================================
///						箱型
void LineManager::DrawBox(const Vector3 &center, const Vector3 &size, const Vector4 &color) {
	if(!isDrawLine_) {
		return;
	}

	// サイズの半分
	Vector3 halfSize = {
		size.x * 0.5f,
		size.y * 0.5f,
		size.z * 0.5f
	};

	// 8つの頂点を計算
	Vector3 vertices[8] = {
		{ center.x - halfSize.x, center.y - halfSize.y, center.z - halfSize.z }, // 左下前
		{ center.x + halfSize.x, center.y - halfSize.y, center.z - halfSize.z }, // 右下前
		{ center.x + halfSize.x, center.y + halfSize.y, center.z - halfSize.z }, // 右上前
		{ center.x - halfSize.x, center.y + halfSize.y, center.z - halfSize.z }, // 左上前
		{ center.x - halfSize.x, center.y - halfSize.y, center.z + halfSize.z }, // 左下後
		{ center.x + halfSize.x, center.y - halfSize.y, center.z + halfSize.z }, // 右下後
		{ center.x + halfSize.x, center.y + halfSize.y, center.z + halfSize.z }, // 右上後
		{ center.x - halfSize.x, center.y + halfSize.y, center.z + halfSize.z }  // 左上後
	};

	// 前面の四角形
	DrawLine(vertices[0], vertices[1], color);
	DrawLine(vertices[1], vertices[2], color);
	DrawLine(vertices[2], vertices[3], color);
	DrawLine(vertices[3], vertices[0], color);

	// 後面の四角形
	DrawLine(vertices[4], vertices[5], color);
	DrawLine(vertices[5], vertices[6], color);
	DrawLine(vertices[6], vertices[7], color);
	DrawLine(vertices[7], vertices[4], color);

	// 前面と後面を繋ぐ線
	DrawLine(vertices[0], vertices[4], color);
	DrawLine(vertices[1], vertices[5], color);
	DrawLine(vertices[2], vertices[6], color);
	DrawLine(vertices[3], vertices[7], color);
}

///=============================================================================
///						矩形の描画
void LineManager::DrawRectangle(const Vector3 &center, const Vector2 &size, const Vector4 &color) {
	if(!isDrawLine_) {
		return;
	}

	// サイズの半分
	float halfWidth = size.x * 0.5f;
	float halfHeight = size.y * 0.5f;

	// 頂点を計算（XZ平面上に配置）
	Vector3 v1 = { center.x - halfWidth, center.y, center.z - halfHeight }; // 左下
	Vector3 v2 = { center.x + halfWidth, center.y, center.z - halfHeight }; // 右下
	Vector3 v3 = { center.x + halfWidth, center.y, center.z + halfHeight }; // 右上
	Vector3 v4 = { center.x - halfWidth, center.y, center.z + halfHeight }; // 左上

	// 4辺を描画
	DrawLine(v1, v2, color);
	DrawLine(v2, v3, color);
	DrawLine(v3, v4, color);
	DrawLine(v4, v1, color);
}

///=============================================================================
///						矢印の描画
void LineManager::DrawArrow(const Vector3 &start, const Vector3 &end, float headSize, const Vector4 &color) {
	if(!isDrawLine_) {
		return;
	}

	// 矢印のメイン部分
	DrawLine(start, end, color);

	// 方向ベクトルを計算
	Vector3 direction = {
		end.x - start.x,
		end.y - start.y,
		end.z - start.z
	};

	// 方向ベクトルの長さを計算
	float length = sqrtf(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
	if(length < 0.0001f) return;

	// 単位ベクトル化
	direction = { direction.x / length, direction.y / length, direction.z / length };

	// 垂直ベクトルを作成
	Vector3 right;
	if(fabsf(direction.y) > 0.99f) {
		right = { 1.0f, 0.0f, 0.0f };
	} else {
		right = { -direction.z, 0.0f, direction.x };
		float rightLength = sqrtf(right.x * right.x + right.z * right.z);
		right = { right.x / rightLength, 0.0f, right.z / rightLength };
	}

	// 矢じりの計算
	Vector3 back = {
		end.x - direction.x * headSize,
		end.y - direction.y * headSize,
		end.z - direction.z * headSize
	};

	Vector3 rightPoint = {
		back.x + right.x * headSize * 0.5f,
		back.y + right.y * headSize * 0.5f,
		back.z + right.z * headSize * 0.5f
	};

	Vector3 leftPoint = {
		back.x - right.x * headSize * 0.5f,
		back.y - right.y * headSize * 0.5f,
		back.z - right.z * headSize * 0.5f
	};

	// 矢じりを描画
	DrawLine(end, rightPoint, color);
	DrawLine(end, leftPoint, color);
	DrawLine(rightPoint, leftPoint, color);
}

///=============================================================================
///						円柱の描画
// LineManager.cpp に実装
void LineManager::DrawCylinder(const Vector3 &start, const Vector3 &end, float radius, const Vector4 &color, int divisions) {
	if(!isDrawLine_ || divisions <= 0) {
		return;
	}

	// 方向ベクトルを計算
	Vector3 direction = {
		end.x - start.x,
		end.y - start.y,
		end.z - start.z
	};

	float length = sqrtf(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
	if(length < 0.0001f) return;

	// 単位ベクトル化
	direction = { direction.x / length, direction.y / length, direction.z / length };

	// 円の基準となる軸を作成
	Vector3 u, v;
	if(fabsf(direction.x) < 0.707f && fabsf(direction.z) < 0.707f) {
		// Y軸に近い場合
		u = { 1.0f, 0.0f, 0.0f };
	} else {
		// それ以外の場合
		u = { 0.0f, 1.0f, 0.0f };
	}

	// 外積で直交する2つのベクトルを作成
	v = {
		direction.y * u.z - direction.z * u.y,
		direction.z * u.x - direction.x * u.z,
		direction.x * u.y - direction.y * u.x
	};

	float vLength = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
	v = { v.x / vLength, v.y / vLength, v.z / vLength };

	u = {
		direction.y * v.z - direction.z * v.y,
		direction.z * v.x - direction.x * v.z,
		direction.x * v.y - direction.y * v.x
	};

	// 円周上の点を計算
	std::vector<Vector3> startPoints(divisions);
	std::vector<Vector3> endPoints(divisions);

	float angleStep = 2.0f * static_cast<float>( M_PI ) / divisions;

	for(int i = 0; i < divisions; i++) {
		float angle = i * angleStep;
		float c = cosf(angle) * radius;
		float s = sinf(angle) * radius;

		startPoints[i] = {
			start.x + u.x * c + v.x * s,
			start.y + u.y * c + v.y * s,
			start.z + u.z * c + v.z * s
		};

		endPoints[i] = {
			end.x + u.x * c + v.x * s,
			end.y + u.y * c + v.y * s,
			end.z + u.z * c + v.z * s
		};
	}

	// 円を描画
	for(int i = 0; i < divisions; i++) {
		int next = ( i + 1 ) % divisions;
		DrawLine(startPoints[i], startPoints[next], color);
		DrawLine(endPoints[i], endPoints[next], color);
		DrawLine(startPoints[i], endPoints[i], color);
	}
}

///=============================================================================
///						コーンの描画
void LineManager::DrawCone(const Vector3 &start, const Vector3 &end, float radius, const Vector4 &color, int divisions) {
	if(!isDrawLine_ || divisions <= 0) {
		return;
	}

	// 方向ベクトルを計算
	Vector3 direction = {
		end.x - start.x,
		end.y - start.y,
		end.z - start.z
	};

	float length = sqrtf(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
	if(length < 0.0001f) return;

	// 単位ベクトル化
	direction = { direction.x / length, direction.y / length, direction.z / length };

	// 円の基準となる軸を作成
	Vector3 u, v;
	if(fabsf(direction.x) < 0.707f && fabsf(direction.z) < 0.707f) {
		// Y軸に近い場合
		u = { 1.0f, 0.0f, 0.0f };
	} else {
		// それ以外の場合
		u = { 0.0f, 1.0f, 0.0f };
	}

	// 外積で直交する2つのベクトルを作成
	v = {
		direction.y * u.z - direction.z * u.y,
		direction.z * u.x - direction.x * u.z,
		direction.x * u.y - direction.y * u.x
	};

	float vLength = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
	v = { v.x / vLength, v.y / vLength, v.z / vLength };

	u = {
		direction.y * v.z - direction.z * v.y,
		direction.z * v.x - direction.x * v.z,
		direction.x * v.y - direction.y * v.x
	};

	// 円周上の点を計算
	std::vector<Vector3> basePoints(divisions);

	float angleStep = 2.0f * static_cast<float>( M_PI ) / divisions;

	for(int i = 0; i < divisions; i++) {
		float angle = i * angleStep;
		float c = cosf(angle) * radius;
		float s = sinf(angle) * radius;

		basePoints[i] = {
			start.x + u.x * c + v.x * s,
			start.y + u.y * c + v.y * s,
			start.z + u.z * c + v.z * s
		};
	}

	// 底面の円を描画
	for(int i = 0; i < divisions; i++) {
		int next = ( i + 1 ) % divisions;
		DrawLine(basePoints[i], basePoints[next], color);
	}

	// 底面の各点から頂点へ線を引く
	for(int i = 0; i < divisions; i++) {
		DrawLine(basePoints[i], end, color);
	}
}