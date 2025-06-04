#pragma once
#include "imgui.h"
#include "WinApp.h"
#include "DirectXCommon.h"

class ImGuiManager {
public:

    ~ImGuiManager();

	/// <summary>
    /// 初期化
    /// </summary>
    void Initialize(WinApp* winApp,DirectXCommon* dxCommon);

    /// <summary>
    /// ImGui受付開始
    /// </summary>
    void Begin();

    /// <summary>
    /// ImGui受付終了
    /// </summary>
    void End();

    /// <summary>
    /// 画面への描画
    /// </summary>
    void Draw(DirectXCommon* dxCommon);

private:

    // SRV用デスクリプタヒープ
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvHeap_;

};


