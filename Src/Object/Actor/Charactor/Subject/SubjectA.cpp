#include "SubjectA.h"
#include "../../../Common/Transform.h"
#include "../../Charactor/Player/Player.h"
#include "../../../../Utility/AsoUtility.h"
#include <cfloat>

SubjectA::SubjectA(void)
{
	// Player を生成しない（外部から座標が渡される前提）
	player_ = nullptr;
}

SubjectA::~SubjectA(void)
{
}

void SubjectA::InitPost(void)
{
	Subject::InitPost();

	// SubjectA の移動範囲設定
	SetMoveArea(VGet(-40000.0f, 0.0f, -40000.0f),
		VGet(40000.0f, 0.0f, 40000.0f));
}

VECTOR SubjectA::GetInitPos(void)
{
	return VGet(0.0f, 100.0f, 0.0f);
}

ResourceManager::SRC SubjectA::GetModelType() const
{
	return ResourceManager::SRC::SUBJECT;
}

void SubjectA::UpdateMove(void)
{
	// Subject の通常移動をデフォルト動作とする
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

	// 最短プレイヤー方向へ moveDir_ を向けて進む
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
	transform_.pos = VAdd(transform_.pos, VScale(moveDir_, MOVE_SPEED));
	FaceMoveDirection();
}
