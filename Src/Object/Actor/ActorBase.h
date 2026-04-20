#pragma once
#include <vector>
#include <map>
#include "../Common/Transform.h"

class ResourceManager;
class SceneManager;
class ColliderBase;

class ActorBase
{

public:

	// コンストラクタ
	ActorBase(void);

	// デストラクタ
	virtual ~ActorBase(void);

	// 初期化
	void Init(void);

	// 更新
	virtual void Update(void) = 0;

	// 描画
	virtual void Draw(void);

	// 解放
	virtual void Release(void);

	// 大きさ、回転、座標等の取得
	const Transform& GetTransform(void) const;

	// 自身の衝突情報取得
	const std::map<int, ColliderBase*>& GetOwnColliders(void) const
	{
		return ownColliders_;
	}

	// 特定の自身の衝突情報取得
	const ColliderBase* GetOwnCollider(int key) const;

	// 衝突対象となるコライダを登録
	void AddHitCollider(const ColliderBase* hitCollider);

	// 衝突対象となるコライダをクリア
	void ClearHitCollider(void);

protected:

	// シングルトン参照
	ResourceManager& resMng_;
	SceneManager& scnMng_;

	// モデル制御の基本情報
	Transform transform_;

	// 自身の衝突情報
	std::map<int, ColliderBase*> ownColliders_;

	// 衝突相手の情報
	std::vector<const ColliderBase*> hitColliders_;

	// リソースロード
	virtual void InitLoad(void) = 0;

	// 大きさ、回転、座標の初期化
	virtual void InitTransform(void) = 0;

	// 衝突判定の初期化
	virtual void InitCollider(void) = 0;

	// アニメーションの初期化
	virtual void InitAnimation(void) = 0;

	// 初期化後の個別処理
	virtual void InitPost(void) = 0;

};
