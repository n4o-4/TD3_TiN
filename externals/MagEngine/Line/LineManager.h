#pragma once
#include <vector>
#include <memory>
#include "Line.h"
#include "LineSetup.h"
#include "DirectXCommon.h"
#include "SrvManager.h"
#include "ViewProjection.h"

/// ラインマネージャクラス
class LineManager {
public:

	/// @brief インスタンスの取得
	/// @return LineManager* インスタンスのポインタ
    static LineManager* GetInstance();

	/// @brief 初期化
	/// @param dxCommon ダイレクトX共通クラス
	/// @param srvManager SRVマネージャ
    void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager);

	/// @brief 終了処理
    void Finalize();

	/// @brief 更新処理
    void Update();

	/// @brief 描画
    void Draw();

	/// @brief ImGuiの描画
    void DrawImGui();

	/// @brief ラインのクリア
    void ClearLines();

	/// @brief ラインの追加
    /// @param start 始点
	/// @param end 終点
	/// @param color 色
    void DrawLine(const Vector3& start, const Vector3& end, const Vector4& color);

	/// @brief グリッドの描画
    /// @param gridSize サイズ
	/// @param divisions 分割数
	/// @param color 色
    void DrawGrid(float gridSize, int divisions, const Vector4& color);

	/// @brief 球体の描画
    /// @param center 中心
	/// @param radius 半径
	/// @param color 色
	/// @param divisions 分割数
    void DrawSphere(const Vector3& center, float radius, const Vector4& color, int divisions = 32);
    
	/// @brief 円の描画
	/// @param center 中心
    /// @param radius 半径
    /// @param color 色
    /// @param divisions 分割数
    void DrawCircle(const Vector3& center, float radius, const Vector4& color, int divisions = 32);

    /// @brief 箱型
    /// @param center 中心
    /// @param size サイズ
    /// @param color 色
    void DrawBox(const Vector3& center, const Vector3& size, const Vector4& color);

    /// @brief 矩形の描画
    /// @param center 中心
    /// @param size サイズ
    /// @param color 色
    void DrawRectangle(const Vector3& center, const Vector2& size, const Vector4& color);

    /// @brief 矢印の描画
    /// @param start 始点
    /// @param end 終点
    /// @param headSize 矢じりのサイズ
    /// @param color 色
    void DrawArrow(const Vector3& start, const Vector3& end, float headSize, const Vector4& color);

    /// @brief 円柱の描画
    /// @param start 始点
    /// @param end 終点
    /// @param radius 半径
    /// @param color 色
    /// @param divisions 分割数
    void DrawCylinder(const Vector3& start, const Vector3& end, float radius, const Vector4& color, int divisions = 32);

    /// @brief コーンの描画
    /// @param start 始点
    /// @param end 終点
    /// @param radius 半径
    /// @param color 色
    /// @param divisions 分割数
    void DrawCone(const Vector3& start, const Vector3& end, float radius, const Vector4& color, int divisions = 32);

	/// @brief ラインの描画設定
	/// @return bool ラインを描画するか
    ViewProjection* GetDefaultCamera() { return lineSetup_->GetDefaultCamera(); }

	/// @brief ラインの描画設定
	/// @param viewProjection ビュープロジェクション
    void SetDefaultCamera(ViewProjection* viewProjection) { lineSetup_->SetDefaultCamera(viewProjection); }

private:
    static LineManager* instance_;

    //コンストラクタ
    LineManager() = default;
    //デストラクタ
    ~LineManager() = default;
    //コピーコンストラクタ
    LineManager(const LineManager&) = delete;    
    //代入演算子
    LineManager& operator=(const LineManager&) = delete;

    // DirectXCommonへの参照
    DirectXCommon* dxCommon_ = nullptr;
    // SrvManagerへの参照
    SrvManager* srvManager_ = nullptr;

    // Line
    std::unique_ptr<Line> line_;

    // LineSetupインスタンス
    std::unique_ptr<LineSetup> lineSetup_;

    // 描画設定
    bool isDrawLine_ = true;   // ラインを描画するか
    bool isDrawGrid_ = true;   // グリッドを描画するか
    float gridSize_ = 64.0f;   // グリッドサイズ
    int gridDivisions_ = 8;    // グリッド分割数
    Vector4 gridColor_ = { 0.5f, 0.5f, 0.5f, 1.0f };  // グリッド色

    // 球体を描画するか
    bool isDrawSphere_ = true;
};