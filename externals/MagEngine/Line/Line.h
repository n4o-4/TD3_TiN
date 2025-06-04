#pragma once
#include <vector>
#include <wrl.h>
#include <d3d12.h>
#include "Vectors.h"
#include "Matrixs.h"
#include "DirectXCommon.h"
#include "ViewProjection.h"
#include "Camera.h"

// 前方宣言
class LineSetup;

// ラインの頂点データ
struct LineVertex {
    Vector3 position; // 座標
    Vector4 color;    // 色
};

// トランスフォーメーションマトリックスデータ
struct TransformationMatrix {
    Matrix4x4 WVP;               // ワールドビュープロジェクション行列
    Matrix4x4 World;             // ワールド行列
    Matrix4x4 WorldInvTranspose; // ワールド行列の逆行列の転置行列
};

class Line {
public:
    /// 初期化
    void Initialize(LineSetup* lineSetup);

    /// 更新
    void Update();

    /// 描画
    void Draw();

    /// ラインのクリア
    void ClearLines();

    /// ライン描画
    void DrawLine(const Vector3& start, const Vector3& end, const Vector4& color);

private:
    /// 頂点バッファの作成
    void CreateVertexBuffer();

    /// 変換行列バッファの作成
    void CreateTransformationMatrixBuffer();

public:
    // トランスフォーム系
    struct TransformStructure {
        Vector3 scale = {1.0f, 1.0f, 1.0f};
        Vector3 rotate = {0.0f, 0.0f, 0.0f};
        Vector3 translate = {0.0f, 0.0f, 0.0f};
    };

    /// スケールの設定
    void SetScale(const Vector3& scale) { transform_.scale = scale; }
    
    /// スケールの取得
    const Vector3& GetScale() const { return transform_.scale; }

    /// 回転の設定
    void SetRotate(const Vector3& rotate) { transform_.rotate = rotate; }
    
    /// 回転の取得
    const Vector3& GetRotate() const { return transform_.rotate; }

    /// 平行移動の設定
    void SetTranslate(const Vector3& translate) { transform_.translate = translate; }
    
    /// 平行移動の取得
    const Vector3& GetTranslate() const { return transform_.translate; }

private:

    //---------------------------------------
    // オブジェクト3Dセットアップポインタ
    LineSetup* lineSetup_ = nullptr;

    //---------------------------------------
    // 頂点データ
    std::vector<LineVertex> vertices_;

    //---------------------------------------
    // 頂点バッファ
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_ = nullptr;
    // バッファリソースの使い道を指すポインタ
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_ = {};

    //---------------------------------------
    //トランスフォーメーションマトリックス
    Microsoft::WRL::ComPtr <ID3D12Resource> transfomationMatrixBuffer_;

    //---------------------------------------
    // バッファリソース内のデータを指すポインタ
    //トランスフォーメーションマトリックス
    TransformationMatrix* transformationMatrixData_ = nullptr;

    //--------------------------------------
    // Transform
    Transform transform_ = {};

    //--------------------------------------
    // カメラ
    ViewProjection* camera_ = nullptr;
};