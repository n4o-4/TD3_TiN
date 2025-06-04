#include "CollisionManager.h"
#include "BaseObject.h"
#include "LineManager.h"
#include <algorithm>
#include <cmath>

///=============================================================================
///						初期化
void CollisionManager::Initialize(float cellSize) {
	// グリッドのセルサイズを設定
	cellSize_ = cellSize;
}
///=============================================================================
///						更新処理
void CollisionManager::Update() {
	//========================================
	// 前回のグリッド情報をクリア
	grid_.clear();
	//========================================
	// 全オブジェクトをグリッドに登録
	for (auto *baseObj : Objects_) {
		int index = GetGridIndex(baseObj->GetCollider()->GetPosition());
		grid_[index].objects.push_back(baseObj);
	}
	//========================================
	// 衝突判定を実行
	CheckAllCollisions();
	for (const auto &pair : grid_) {
		for (auto *obj : pair.second.objects) {
			Vector3 position = obj->GetCollider()->GetPosition();
			float radius = obj->GetCollider()->GetRadius();
		}
	}

	DrawImGui();
}
///=============================================================================
///						描画
void CollisionManager::Draw() {
	if (isHitDraw_) {
		for (const auto &pair : grid_) {
			for (auto *obj : pair.second.objects) {
				if (obj && obj->GetCollider()) {
					Vector3 position = obj->GetCollider()->GetPosition();
					float radius = obj->GetCollider()->GetRadius();
					LineManager::GetInstance()->DrawSphere(position, radius, {1.0f, 0.0f, 0.0f, 1.0f}, 16); // 赤色で球体を描画、分割数は16
				}
			}
		}
	}
}
///=============================================================================
///						Imguiの描画
void CollisionManager::DrawImGui() {

#ifdef _DEBUG

	// あたってるオブジェクトの数
	ImGui::Begin("CollisionManager");
	ImGui::Text("Colliding Objects: %d", Objects_.size());
	// HitBoxの表示
	ImGui::Checkbox("HitBox", &isHitDraw_);
	ImGui::End();

#endif
}

///=============================================================================
///						リセット
void CollisionManager::Reset() {
	// リストを空っぽにする
	Objects_.clear();
	// グリッドをクリアする
	grid_.clear();
}
///=============================================================================
///						コライダーの追加
void CollisionManager::AddCollider(BaseObject *baseObj) {
	// オブジェクトをリストに追加
	Objects_.push_back(baseObj);
	// オブジェクトをグリッドに追加
	int index = GetGridIndex(baseObj->GetCollider()->GetPosition());
	// グリッドにオブジェクトを追加
	grid_[index].objects.push_back(baseObj);
}
///=============================================================================
///						グリッドのインデックスを取得
int CollisionManager::GetGridIndex(const Vector3 &position) const {
	// セルのインデックスを計算
	int x = static_cast<int>(position.x / cellSize_);
	int y = static_cast<int>(position.y / cellSize_);
	int z = static_cast<int>(position.z / cellSize_);
	// インデックスを返す
	return (x * 73856093) ^ (y * 19349663) ^ (z * 83492791); // ハッシュ関数
}
///=============================================================================
///						セル内の当たり判定をチェック
void CollisionManager::CheckCollisionsInCell(const GridCell &cell) {
	//========================================
	// セル内の全てのオブジェクトペアの衝突判定を行う
	for (size_t i = 0; i < cell.objects.size(); ++i) {
		//========================================
		// 同じオブジェクト同士の衝突はチェックしない
		for (size_t j = i + 1; j < cell.objects.size(); ++j) {
			BaseObject *objA = cell.objects[i];
			BaseObject *objB = cell.objects[j];
			//========================================
			// 現在の衝突状態を取得
			bool currentlyColliding = objA->GetCollider()->Intersects(*objB->GetCollider());
			//========================================
			// 衝突しているかどうかを確認
			if (currentlyColliding) {
				// オブジェクトペアを作成
				auto pair = std::make_pair(objA, objB);
				//---------------------------------------
				// 新たな衝突かどうかを確認
				if (collidedPairs_.find(pair) == collidedPairs_.end()) {
					//---------------------------------------
					// 新たな衝突
					objA->OnCollisionEnter(objB);
					objB->OnCollisionEnter(objA);
					// 衝突ペアをセットに追加
					collidedPairs_.insert(pair);
				} else {
					//---------------------------------------
					// 継続中の衝突
					objA->OnCollisionStay(objB);
					objB->OnCollisionStay(objA);
				}
			} else {
				//========================================
				// オブジェクトペアを作成
				auto pair = std::make_pair(objA, objB);
				//========================================
				// 衝突が終了したかどうかを確認
				if (collidedPairs_.find(pair) != collidedPairs_.end()) {
					//---------------------------------------
					// 衝突終了
					objA->OnCollisionExit(objB);
					objB->OnCollisionExit(objA);
					// 衝突ペアをセットから削除
					collidedPairs_.erase(pair);
				}
			}
		}
	}
}
///=============================================================================
///						コリジョン同士をチェック
void CollisionManager::CheckCollisionsBetweenCells(const GridCell &cellA, const GridCell &cellB) {
	//========================================
	// 異なるセル間の全てのオブジェクトペアの衝突判定を行う
	for (auto *objA : cellA.objects) {
		for (auto *objB : cellB.objects) {
			bool currentlyColliding = objA->GetCollider()->Intersects(*objB->GetCollider());
			//========================================
			// オブジェクトのポインタを比較して順序を決定
			BaseObject *first = (objA < objB) ? objA : objB;
			BaseObject *second = (objA < objB) ? objB : objA;
			auto pair = std::make_pair(first, second);

			//========================================
			// 衝突しているかどうかを確認
			if (currentlyColliding) {
				//========================================
				// 新たな衝突かどうかを確認
				if (collidedPairs_.find(pair) == collidedPairs_.end()) {
					// 新たな衝突
					objA->OnCollisionEnter(objB);
					objB->OnCollisionEnter(objA);
					collidedPairs_.insert(pair);
				} else {
					// 継続中の衝突
					objA->OnCollisionStay(objB);
					objB->OnCollisionStay(objA);
				}
			} else {
				//========================================
				// 衝突が終了したかどうかを確認
				if (collidedPairs_.find(pair) != collidedPairs_.end()) {
					// 衝突終了
					objA->OnCollisionExit(objB);
					objB->OnCollisionExit(objA);
					collidedPairs_.erase(pair);
				}
			}
		}
	}
}
///=============================================================================
///						すべての当たり判定をチェック
void CollisionManager::CheckAllCollisions() {
	//========================================
	// グリッド内の全てのセルの当たり判定をチェック
	for (auto &pair : grid_) {
		CheckCollisionsInCell(pair.second);
	}
	//========================================
	// グリッド間の全てのセルの当たり判定をチェック
	for (auto it1 = grid_.begin(); it1 != grid_.end(); ++it1) {
		//========================================
		// 他のセルとの当たり判定をチェック
		for (auto it2 = std::next(it1); it2 != grid_.end(); ++it2) {
			//---------------------------------------
			// セル間の当たり判定をチェック
			CheckCollisionsBetweenCells(it1->second, it2->second);
		}
	}
}
