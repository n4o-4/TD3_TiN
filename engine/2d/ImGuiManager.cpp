#include "ImGuiManager.h"
#include <imgui_impl_win32.h>
#include <imgui_impl_dx12.h>

ImGuiManager::~ImGuiManager()
{

	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();

}

#include "imgui.h" // 追加

void ImGuiManager::Initialize(WinApp* winApp, DirectXCommon* dxCommon)
{
	// ImGuiのコンテキストを生成
	ImGui::CreateContext();

	// ImGuiのスタイルを設定
	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(winApp->GetHWND());

	// デスクリプタヒープ設定
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.NumDescriptors = 1;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	// デスクリプタヒープ生成
	HRESULT result = dxCommon->GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&srvHeap_));
	assert(SUCCEEDED(result));

	ImGui_ImplDX12_Init(
		dxCommon->GetDevice().Get(),
		static_cast<int>(dxCommon->GetBackBufferCount()),
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, srvHeap_.Get(),
		srvHeap_->GetCPUDescriptorHandleForHeapStart(),
		srvHeap_->GetGPUDescriptorHandleForHeapStart());


	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // ドッキングを
}

void ImGuiManager::Begin()
{
	// ImGuiフレーム開始
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void ImGuiManager::End()
{
	// 描画前準備
	ImGui::Render();
}

void ImGuiManager::Draw(DirectXCommon* dxCommon)
{
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = dxCommon->GetCommandList().Get();

	// デスクリプタヒープの配列をセットするコマンド
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> ppHeaps[] = { srvHeap_.Get() };
	commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps->GetAddressOf());
	// 描画コマンドを実行
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList.Get());
}