#include "SubjectB.h"
#include "../../../Common/Transform.h"
#include "../../../../Utility/AsoUtility.h"
#include <cfloat>

SubjectB::SubjectB(void)
{
	// 外部からプレイヤー座標が渡される前提のため何もしない
}

SubjectB::~SubjectB(void)
{
}

void SubjectB::InitPost(void)
{
	Subject::InitPost();

	// SubjectB の移動範囲（必要なら調整）
	SetMoveArea(VGet(2000.0f, 0.0f, 2000.0f),
		VGet(2000.0f, 0.0f, 2000.0f));
}

VECTOR SubjectB::GetInitPos(void)
{
	// SubjectB 固有の初期位置
	return VGet(200.0f, 100.0f, 0.0f);
}

ResourceManager::SRC SubjectB::GetModelType() const
{
	return ResourceManager::SRC::SUBJECT;
}

void SubjectB::UpdateMove(void)
{
	// プレイヤー座標が無ければデフォルト挙動
	const auto& players = GetPlayerPos();
	if (players.empty())
	{
		Subject::UpdateMove();
		return;
	}

	// 最も近いプレイヤーを探索
	int nearestIdx = -1;
	float nearestDist = FLT_MAX;
	for (size_t i = 0; i < players.size(); ++i)
	{
		VECTOR diff = VSub(players[i], transform_.pos);
		diff.y = 0.0f;
		const float d = VSize(diff);
		if (d < nearestDist)
		{
			nearestDist = d;
			nearestIdx = static_cast<int>(i);
		}
	}

	// 検知範囲外なら通常挙動へフォールバック
	if (nearestIdx < 0 || nearestDist > playerDetectRange_)
	{
		Subject::UpdateMove();
		return;
	}

	// 最短プレイヤー方向へ向かう（SubjectA と同様の単純追従）
	VECTOR target = players[nearestIdx];
	VECTOR dir = VSub(target, transform_.pos);
	dir.y = 0.0f;
	const float len = VSize(dir);
	if (len <= 0.0001f)
	{
		Subject::UpdateMove();
		return;
	}

	dir = VScale(dir, 1.0f / len); // 正規化
	moveDir_ = dir;

	// 移動速度はベースの MOVE_SPEED を使用（必要なら調整）
	transform_.pos = VAdd(transform_.pos, VScale(moveDir_, MOVE_SPEED));
	FaceMoveDirection();
}

