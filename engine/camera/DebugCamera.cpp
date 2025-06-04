#include "DebugCamera.h"  
#include <algorithm>

void DebugCamera::Initialize()  
{  
   BaseCamera::Initialize();  

   targetTransform_ = std::make_unique<WorldTransform>();  
   targetTransform_->Initialize();  

   // ターゲットからのオフセットを初期化  
   offset = { 0.0f,0.0f,-10.0f };  
}  

void DebugCamera::Update()  
{  
   /// デバッグカメラ特有の処理  

   // 計算用のオフセット  
   Vector3 offSet = {};  

   if (Input::GetInstance()->mouseState.lZ != 0)  
   {  
       // マウスホイールの回転量でオフセットを変更  
       offset.z += static_cast<float>(Input::GetInstance()->mouseState.lZ) * 0.01f;  

       offset.z = std::clamp(offset.z, -100.0f, -1.0f); // 修正: float 型のリテラルに変更

       // カメラの角度から回転行列を計算  
       Matrix4x4 rotateMatrix = MakeRotateMatrix(viewProjection_->transform.rotate);  

       // オフセットをカメラの回転に合わせて回転  
       offSet = TransformNormal(offset, rotateMatrix);  

       // カメラの位置をターゲットの位置からオフセット分ずらす  
       viewProjection_->transform.translate = targetTransform_->transform.translate + offSet;  
   }  

#ifdef _DEBUG
   ImGui::Text("offset.z%f", static_cast<float>(Input::GetInstance()->mouseState.lZ));  
#endif
   if (Input::GetInstance()->PushMouseButton(Input::MouseButton::LEFT))  
   {  
       Vector3 rotate = { static_cast<float>(Input::GetInstance()->mouseState.lY) * 0.0008f, static_cast<float>(Input::GetInstance()->mouseState.lX) * 0.0008f,0.0f };  

       viewProjection_->transform.rotate = viewProjection_->transform.rotate + rotate;  

       // カメラの角度から回転行列を計算  
       Matrix4x4 rotateMatrix = MakeRotateMatrix(viewProjection_->transform.rotate);  

       // オフセットをカメラの回転に合わせて回転  
       offSet = TransformNormal(offset, rotateMatrix);  

       // カメラの位置をターゲットの位置からオフセット分ずらす  
       viewProjection_->transform.translate = targetTransform_->transform.translate + offSet;  
   }  

   // 汎用変数更新  
   BaseCamera::Update();  
}
