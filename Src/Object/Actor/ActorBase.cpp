#include "../../Manager/ResourceManager.h"
#include "../../Manager/SceneManager.h"
#include "../../Object/Collider/ColliderBase.h"
#include "ActorBase.h"

ActorBase::ActorBase(void)
	:
	resMng_(ResourceManager::GetInstance()),
	scnMng_(SceneManager::GetInstance()),
	transform_()
{
}

ActorBase::~ActorBase(void)
{
}

void ActorBase::Init(void)
{

	// リソースロード
	InitLoad();

	// Transform初期化
	InitTransform();

	// 衝突判定の初期化
	InitCollider();

	// アニメーションの初期化
	InitAnimation();

	// 初期化後の個別処理
	InitPost();

}

void ActorBase::Draw(void)
{
	if (transform_.modelId != -1)
	{
		MV1DrawModel(transform_.modelId);
	}

#ifdef _DEBUG

	// 所有しているコライダの描画
	for (const auto& own : ownColliders_)
	{
		own.second->Draw();
	}

#endif // _DEBUG
}

void ActorBase::Release(void)
{
	transform_.Release();

	// 自身のコライダ解放
	for (auto& own : ownColliders_)
	{
		delete own.second;
	}
	ownColliders_.clear();
}

const Transform& ActorBase::GetTransform(void) const
{
	return transform_;
}

const ColliderBase* ActorBase::GetOwnCollider(int key) const
{
	if (ownColliders_.count(key) == 0)
	{
		return nullptr;
	}
	return ownColliders_.at(key);
}

void ActorBase::AddHitCollider(const ColliderBase* hitCollider)
{
	for (const auto& c : hitColliders_)
	{
		if (c == hitCollider)
		{
			return;
		}
	}
	hitColliders_.emplace_back(hitCollider);
}

void ActorBase::ClearHitCollider(void)
{
	hitColliders_.clear();
}

