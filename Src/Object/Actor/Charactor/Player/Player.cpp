#include "Player.h"
#include "../../../../Manager/ResourceManager.h"
#include "../../../../Object/Collider/ColliderModel.h"
#include "../../../../Utility/AsoUtility.h"

Player::Player(void)
	:
	ActorBase()
{
}

Player::~Player(void)
{
}

void Player::Update(void)
{
}

void Player::InitLoad(void)
{
	transform_.SetModel(
		resMng_.LoadModelDuplicate(ResourceManager::SRC::PLAYER));

	// 描画されているかチェック
	if (transform_.modelId == -1)
	{
		// ロード失敗
		return;
	}
}

void Player::InitTransform(void)
{
	transform_.scl = { 0.01f,0.01f,0.01f };
	transform_.quaRot = Quaternion::Identity();
	transform_.quaRotLocal = Quaternion::Identity();
	transform_.pos = INIT_POS;
	transform_.Update();
}

void Player::InitCollider(void)
{
	// DxLib側の衝突判定をセットアップ
	MV1SetupCollInfo(transform_.modelId);
	// モデルのコライダー
	ColliderModel* colModel =
		new ColliderModel(ColliderBase::TAG::STAGE, &transform_);

	ownColliders_.emplace(static_cast<int>(COLLIDER_TYPE::MODEL), colModel);

}

void Player::InitAnimation(void)
{
}

void Player::InitPost(void)
{
}

