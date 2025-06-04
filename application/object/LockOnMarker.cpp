#include "LockOnMarker.h"
#include <cmath> // 数学関数用

void LockOnMarker::Initialize() {
    // Multi lock on
    //multiLockOn_ = std::make_unique<Object3d>();
    //multiLockOn_->Initialize(Object3dCommon::GetInstance());
    //ModelManager::GetInstance()->LoadModel("lockOn/Lock_on1.obj");
    //multiLockOn_->SetModel("lockOn/Lock_on1.obj");

    //multiLockOn_->SetEnableLighting(false);

	//multilockOnWorldTransform_ = std::make_unique<WorldTransform>();
	//multilockOnWorldTransform_->Initialize();
    
    // 最初は最大スケールから始める
    //multilockOnWorldTransform_->transform.scale = { maxScale_, maxScale_, maxScale_ };

    isVisible_ = false;
    isAnimating_ = false;
    animationTimer_ = 0.0f;
}

void LockOnMarker::Show() {
    isVisible_ = true;
    isAnimating_ = true;
    // アニメーションを開始時は最大スケールから始める
    //multilockOnWorldTransform_->transform.scale = { maxScale_, maxScale_, maxScale_ };
    animationTimer_ = 0.0f;
}

void LockOnMarker::Hide() {
    isVisible_ = false;
    isAnimating_ = false;
}

// プレイヤー位置とマーカー位置から向きを計算するUpdateメソッド
void LockOnMarker::Update(const Vector3& playerPosition, const Vector3& markerPosition) {
    if (!isVisible_) {
        return;
    }
    
    // マーカーの位置を設定
    //multilockOnWorldTransform_->transform.translate = markerPosition;
    
    // プレイヤーからマーカーへの方向ベクトルを計算
    Vector3 toPlayer = {
        playerPosition.x - markerPosition.x,
        playerPosition.y - markerPosition.y,
        playerPosition.z - markerPosition.z
    };
    
    // ベクトルの長さを計算
    float length = std::sqrt(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y + toPlayer.z * toPlayer.z);
    
    // 長さが0に近い場合は計算しない（ゼロ除算防止）
    if (length > 0.001f) {
        // 正規化
        Vector3 normalized = {
            toPlayer.x / length,
            toPlayer.y / length,
            toPlayer.z / length
        };
        
        // Y軸周りの回転角（水平方向）を計算
        float yaw = std::atan2(normalized.x, normalized.z);
        
        // X軸周りの回転角（垂直方向）を計算
        float pitch = std::atan2(-normalized.y, std::sqrt(normalized.x * normalized.x + normalized.z * normalized.z));
        
        // マーカーの回転を設定
        //multilockOnWorldTransform_->transform.rotate = { pitch, yaw, 0.0f };
    }
    
    // アニメーション処理
    if (isAnimating_) {
        // タイマーを進める
        animationTimer_ += 1.0f / 60.0f; // 60FPSを想定
        
        // 線形に縮小するアニメーション
        float t = std::min(animationTimer_ * animationSpeed_, 1.0f);
        float currentScale = maxScale_ - (maxScale_ - minScale_) * t;
        
        // スケール値を設定
        //multilockOnWorldTransform_->transform.scale = { 
        //    currentScale, 
        //    currentScale, 
        //    currentScale 
        //};
    }
    
    // アニメーション更新（レベルに応じた挙動）
    if (isAnimating_) {
        animationTimer_ += 1.0f / 60.0f;
        
        // レベルに応じてアニメーションパターンを変える
        float t;
        if (lockLevel_ == LOCK_LEVEL_PRECISE) {
            // 精密ロックオンのアニメーション（より細かい脈動）
            t = (sinf(animationTimer_ * animationSpeed_ * 3.14159f) + 1.0f) * 0.5f;
        } else {
            // 通常のアニメーション
            t = (sinf(animationTimer_ * animationSpeed_ * 3.14159f) + 1.0f) * 0.5f;
        }
        
        float currentScale = maxScale_ - (maxScale_ - minScale_) * t;
        
        // スケール値を設定
    //    multilockOnWorldTransform_->transform.scale = { 
    //        currentScale, 
    //        currentScale, 
    //        currentScale 
    //    };
    }

    // 行列を更新
    //multilockOnWorldTransform_->UpdateMatrix();
    //multiLockOn_->
    // (MakeIdentity4x4());
    //multiLockOn_->Update();
}

void LockOnMarker::Draw(ViewProjection viewProjection, DirectionalLight directionalLight, PointLight pointLight, SpotLight spotLight) {
	if (isVisible_) {
	//multiLockOn_->Draw(*multilockOnWorldTransform_.get(), viewProjection, directionalLight, pointLight, spotLight);
	}
}

// ロックオンレベルの設定
void LockOnMarker::SetLockLevel(int level) {
    lockLevel_ = level;
    
    // レベルに応じてマーカーの見た目を変更
    if (lockLevel_ == LOCK_LEVEL_PRECISE) {
        // 精密ロックオンのマーカー設定
        minScale_ = preciseScale_;  // より小さく（精密さを表現）
        maxScale_ = 1.5f;          // 最大スケールも少し小さく
        animationSpeed_ = 3.0f;     // アニメーションを速く
        
        // 色を設定する場合はここでモデルの色を変更
        // multiLockOn_->SetColor({1.0f, 0.2f, 0.2f, 1.0f});  // 赤色で強調
    } else if (lockLevel_ == LOCK_LEVEL_SIMPLE) {
        // 簡易ロックオンのマーカー設定
        minScale_ = 1.5f;          // 通常サイズ
        maxScale_ = 3.0f;          // 通常の最大サイズ
        animationSpeed_ = 2.0f;     // 通常速度
        
        // 色を設定する場合
        // multiLockOn_->SetColor({1.0f, 1.0f, 1.0f, 1.0f});  // 通常色
    } else {
        // ロックオンなしのマーカー設定
        minScale_ = 1.8f;          // やや大きめ
        maxScale_ = 3.2f;          // やや大きめ
        animationSpeed_ = 1.5f;     // やや遅く
        
        // 色を設定する場合
        // multiLockOn_->SetColor({0.7f, 0.7f, 0.7f, 0.7f});  // 薄く表示
    }
}
